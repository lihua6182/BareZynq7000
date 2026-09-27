#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
Save a copy of a PDF with the text translated into Chinese and overlaid
at the original positions as a toggleable PDF layer (OCG).

How it works
------------
1. Open the source PDF and work on a copy (the original is never modified).
2. Extract every text block together with its line geometry (bbox + baseline).
3. Translate each block EN -> ZH at paragraph level (better context), with a
   persistent cache so repeated headers/footers are translated only once.
4. Mask the original English with the page's own background colour and draw the
   Chinese onto the original baselines, auto-fitting the font size so the text
   fits the original line widths.
5. Everything added lives in one Optional Content Group ("中文翻译"), so the
   layer can be switched off in a PDF viewer to reveal the pristine original.

Resumable: the cache is reused between runs, so an interrupted job continues
where it left off instead of re-translating.
"""

import argparse
import hashlib
import json
import os
import re
import sys
import time
import urllib.parse
import urllib.request
from concurrent.futures import ThreadPoolExecutor

import pymupdf as fitz

try:
    import numpy as np
except Exception:
    np = None

# --------------------------------------------------------------------------
# Identifiers that must survive translation verbatim.
# --------------------------------------------------------------------------
GLOSSARY = [
    "Zynq-7000", "Zynq", "SoC", "FPGA", "ARM", "Cortex-A9", "AMBA", "AXI", "AHB", "APB",
    "PS", "PL", "MIO", "EMIO", "GPIO", "SPI", "I2C", "UART", "USB", "CAN", "SDIO", "QSPI",
    "DDR", "DDR3", "DDR2", "LPDDR2", "OCM", "SLCR", "APU", "GIC", "TTC", "WDT", "SWDT",
    "IRQ", "FIQ", "FIFO", "PLL", "DMA", "DMAC", "JTAG", "DAP", "PCAP", "DevC", "XADC",
    "SRAM", "OTP", "eFUSE", "CPU", "FPU", "MMU", "NEON", "TrustZone", "ECC", "CRC",
    "SHA", "AES", "RSA", "HMAC", "PCIE", "SATA", "RTC", "SMC", "NAND", "NOR",
    "GigE", "GMAC", "OTG", "LVTTL", "LVCMOS", "HSTL", "SSTL", "IDCODE", "FSBL",
    "Vivado", "Vitis", "Xilinx", "AMD", "IOP", "TPIU", "ETB", "SCU",
]

UA = {"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64)"}

URL_RE = re.compile(r"(https?://|www\.|@)", re.I)
# running header: "Zynq 7000 SoC Technical Reference Manual 41  UG585 (v1.14) June 30, 2023"
RUNHEAD_RE = re.compile(
    r"^(Zynq 7000 SoC Technical Reference Manual)\s+(\d+)\s+(UG585 \(v[\d.]+\)\s+\w+\s+\d+,\s+\d+)$")
# table-of-contents line: "Introduction . . . . . . 12"
LEADER_RE = re.compile(r"^(.*?)(\s*(?:\.\s*){3,})\s*(\d+)\s*$")


# --------------------------------------------------------------------------
# cache
# --------------------------------------------------------------------------
class Cache:
    def __init__(self, path):
        self.path = path
        self.data = {}
        if path and os.path.exists(path):
            try:
                with open(path, "r", encoding="utf-8") as f:
                    self.data = json.load(f)
            except Exception:
                self.data = {}

    def key(self, text):
        return hashlib.sha1(text.strip().encode("utf-8")).hexdigest()

    def get(self, text):
        return self.data.get(self.key(text))

    def put(self, text, zh):
        self.data[self.key(text)] = zh

    def save(self):
        if not self.path:
            return
        tmp = self.path + ".tmp"
        with open(tmp, "w", encoding="utf-8") as f:
            json.dump(self.data, f, ensure_ascii=False)
        os.replace(tmp, self.path)


# --------------------------------------------------------------------------
# glossary protection
# --------------------------------------------------------------------------
def protect(text):
    store = []
    out = text
    for i, term in enumerate(GLOSSARY):
        pat = re.compile(r"(?<![\w-])" + re.escape(term) + r"(?![\w-])")
        if pat.search(out):
            tok = "QQZ%dZZQ" % i
            out = pat.sub(tok, out)
            store.append((tok, term))
    return out, store


def restore(text, store):
    for tok, term in store:
        text = re.sub(tok, term, text, flags=re.I)
    return re.sub(r"QQZ\d+ZZQ", "", text)


# --------------------------------------------------------------------------
# backends
# --------------------------------------------------------------------------
class MyMemoryBackend:
    """Keyless translation service (subject to a daily quota)."""

    name = "mymemory"

    def __init__(self, email="", workers=6):
        self.email = email
        self.workers = workers
        self.quota_exhausted = False
        self.words = 0

    def _one(self, text):
        q = urllib.parse.quote(text)
        url = "https://api.mymemory.translated.net/get?q=%s&langpair=en%%7Czh-CN" % q
        if self.email:
            url += "&de=" + urllib.parse.quote(self.email)
        req = urllib.request.Request(url, headers=UA)
        with urllib.request.urlopen(req, timeout=30) as r:
            js = json.loads(r.read().decode("utf-8", "replace"))
        if js.get("quotaFinished"):
            self.quota_exhausted = True
        out = (js.get("responseData", {}) or {}).get("translatedText", "") or ""
        if not out or "MYMEMORY WARNING" in out.upper() or "USAGE LIMIT" in out.upper():
            self.quota_exhausted = True
            return ""
        return out

    def _chunk(self, text):
        """MyMemory accepts <=500 bytes per request; split on sentence bounds."""
        parts, cur = [], ""
        for sent in re.split(r"(?<=[.;:!?])\s+", text):
            if len((cur + " " + sent).encode("utf-8")) > 430 and cur:
                parts.append(cur)
                cur = sent
            else:
                cur = (cur + " " + sent).strip()
        if cur:
            parts.append(cur)
        final = []
        for p in parts:
            while len(p.encode("utf-8")) > 430:
                cut = 430
                while cut > 100 and len(p[:cut].encode("utf-8")) > 430:
                    cut -= 20
                sp = p.rfind(" ", 0, cut)
                cut = sp if sp > 100 else cut
                final.append(p[:cut])
                p = p[cut:]
            final.append(p)
        return final

    def translate(self, text):
        if not text.strip() or self.quota_exhausted:
            return ""
        out = []
        for p in self._chunk(text):
            try:
                t = self._one(p)
            except Exception as e:
                print("  ! net error: %s" % e, file=sys.stderr)
                return ""
            if not t:
                return ""
            out.append(t)
            self.words += len(p.split())
            time.sleep(0.15)
        return "".join(out)

    def translate_many(self, items):
        """items: list of (idx, text). Returns dict idx -> translation."""
        res = {}
        if not items:
            return res
        with ThreadPoolExecutor(max_workers=self.workers) as ex:
            futs = {ex.submit(self.translate, t): i for i, t in items}
            for fu in futs:
                i = futs[fu]
                try:
                    res[i] = fu.result()
                except Exception:
                    res[i] = ""
        return res


class NoneBackend:
    """Offline: uses only --trans / cache. Never aborts the run."""

    name = "none"
    quota_exhausted = False
    words = 0

    def translate(self, text):
        return ""

    def translate_many(self, items):
        return {i: "" for i, _ in items}


# --------------------------------------------------------------------------
# geometry / layout
# --------------------------------------------------------------------------
def parse_pages(spec, n):
    if spec in ("all", "*", ""):
        return list(range(n))
    out = []
    for part in spec.split(","):
        part = part.strip()
        if "-" in part:
            a, b = part.split("-", 1)
            a = int(a or 0)
            b = int(b or n - 1)
            out.extend(range(max(0, a), min(n - 1, b) + 1))
        elif part:
            out.append(int(part))
    return sorted(set(out))


def should_skip(text):
    t = text.strip()
    if not t or len(t) < 3:
        return True
    if not re.search(r"[A-Za-z]", t):
        return True                 # pure numbers / symbols
    if URL_RE.search(t):
        return True                 # URLs / emails
    return False


def block_lines(block):
    """Line geometry for a text block, or None if unsupported (rotated etc.)."""
    lines = []
    for ln in block.get("lines", []):
        if ln.get("wmode", 0) != 0:
            return None
        d = ln.get("dir", (1, 0))
        if abs(d[0] - 1) > 1e-6 or abs(d[1]) > 1e-6:
            return None
        spans = [s for s in ln.get("spans", []) if s.get("text", "").strip()]
        if not spans:
            continue
        x0, y0, x1, y1 = ln["bbox"]
        origin = spans[0].get("origin", (x0, y1))
        lines.append({
            "bbox": fitz.Rect(x0, y0, x1, y1),
            "x": min(x0, origin[0]),
            "baseline": origin[1],
            "width": max(1.0, x1 - x0),
            "size": max(s.get("size", 9) for s in spans),
            "bold": bool(spans[0].get("flags", 0) & 2),
            "color": spans[0].get("color", 0),
        })
    return lines or None


def split_units(en):
    """Split a block into independently cacheable translation units."""
    m = RUNHEAD_RE.match(en)
    if m:
        return [m.group(1), m.group(3)], "{0} %s {1}" % m.group(2)
    m = LEADER_RE.match(en)
    if m:
        return [m.group(1)], "{0} %s" % (m.group(2).strip() + " " + m.group(3))
    return [en], "{0}"


class BgSampler:
    """Dominant background colour under a rect (handles shaded table cells)."""

    def __init__(self, page):
        self.arr = None
        if np is None:
            return
        try:
            pix = page.get_pixmap(dpi=72, colorspace=fitz.csRGB)
            self.arr = np.frombuffer(pix.samples, dtype=np.uint8).reshape(
                pix.height, pix.width, 3)
        except Exception:
            self.arr = None

    def color(self, rect):
        if self.arr is None:
            return (1, 1, 1)
        h, w, _ = self.arr.shape
        x0 = max(0, int(rect.x0)); x1 = min(w, int(rect.x1) + 1)
        y0 = max(0, int(rect.y0)); y1 = min(h, int(rect.y1) + 1)
        if x1 <= x0 or y1 <= y0:
            return (1, 1, 1)
        sub = self.arr[y0:y1, x0:x1].reshape(-1, 3).astype(np.int32)
        packed = sub[:, 0] * 65536 + sub[:, 1] * 256 + sub[:, 2]
        vals, counts = np.unique(packed, return_counts=True)
        best = int(vals[counts.argmax()])
        return ((best >> 16) & 255) / 255.0, ((best >> 8) & 255) / 255.0, (best & 255) / 255.0


def fit_lines(font, zh, lines, size):
    """Distribute the Chinese string across the original line widths."""
    chunks, i, n = [], 0, len(zh)
    for ln in lines:
        if i >= n:
            chunks.append("")
            continue
        avail = ln["width"]
        lo, hi, best = 1, n - i, 0
        while lo <= hi:
            mid = (lo + hi) // 2
            if font.text_length(zh[i:i + mid], size) <= avail:
                best = mid
                lo = mid + 1
            else:
                hi = mid - 1
        if best == 0:
            best = 1
        if i + best < n:                       # don't cut a Latin word in half
            back = 0
            while back < 12 and best - back > 1:
                c = zh[i + best - back - 1]
                if c in " \u3000" or not c.isalnum():
                    break
                back += 1
            if back < 12 and best - back > 1:
                best -= back
        chunks.append(zh[i:i + best])
        i += best
    if i < n and chunks:
        chunks[-1] += zh[i:]
    return chunks


def dump_blocks(doc, pages, path):
    """Write {"page:idx": {"page":p, "en": text}} so any translator can fill it in."""
    out = {}
    for pno in pages:
        page = doc[pno]
        for bidx, blk in enumerate(page.get_text("dict").get("blocks", [])):
            if blk.get("type", 0) != 0:
                continue
            lines = block_lines(blk)
            if not lines:
                continue
            raw = " ".join(s.get("text", "") for ln in blk.get("lines", [])
                           for s in ln.get("spans", []))
            en = " ".join(raw.split())
            if should_skip(en):
                continue
            units, _ = split_units(en)
            if should_skip(units[0]):
                continue
            out["%d:%d" % (pno, bidx)] = {"page": pno, "en": en, "lines": len(lines)}
    with open(path, "w", encoding="utf-8") as f:
        json.dump(out, f, ensure_ascii=False, indent=1)
    words = sum(len(v["en"].split()) for v in out.values())
    print("dumped %d blocks (~%d words) -> %s" % (len(out), words, path))


# --------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--pages", default="all")
    ap.add_argument("--cache", default="")
    ap.add_argument("--backend", default="mymemory", choices=["mymemory", "none"])
    ap.add_argument("--email", default="")
    ap.add_argument("--workers", type=int, default=6)
    ap.add_argument("--font", default=r"C:\Windows\Fonts\msyh.ttc")
    ap.add_argument("--font-bold", default=r"C:\Windows\Fonts\simhei.ttf")
    ap.add_argument("--layer", default="中文翻译")
    ap.add_argument("--max-size", type=float, default=11.0)
    ap.add_argument("--min-size", type=float, default=5.0)
    ap.add_argument("--no-cover", action="store_true")
    ap.add_argument("--no-glossary", action="store_true")
    ap.add_argument("--dump", default="", help="dump EN blocks to json and exit")
    ap.add_argument("--trans", default="", help="json {page:idx: chinese} of prepared translations")
    ap.add_argument("--trim", action="store_true", help="output only the processed pages")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--save-every", type=int, default=20)
    args = ap.parse_args()

    doc = fitz.open(args.src)
    pages = parse_pages(args.pages, doc.page_count)
    print("source : %s" % args.src)
    print("pages  : %d total, %d selected" % (doc.page_count, len(pages)))

    # NOTE: with fontfile= an explicit fontname= is REQUIRED. Without it,
    # pymupdf builds a simple font that cannot encode CJK -> renders notdef.
    font_reg = fitz.Font(fontfile=args.font)
    try:
        font_bold = fitz.Font(fontfile=args.font_bold)
    except Exception:
        font_bold = font_reg

    cache = Cache(args.cache)
    backend = MyMemoryBackend(args.email, args.workers) if args.backend == "mymemory" else NoneBackend()

    transmap = {}
    if args.trans and os.path.exists(args.trans):
        with open(args.trans, "r", encoding="utf-8") as f:
            transmap = json.load(f)
        print("trans  : %d ready translations loaded" % len(transmap))

    ocg = doc.add_ocg(args.layer, on=True)
    print("layer  : '%s' (xref %s)" % (args.layer, ocg))

    if args.dump:
        dump_blocks(doc, pages, args.dump)
        return

    st = {"pages": 0, "blocks": 0, "new": 0, "cached": 0, "skipped": 0}
    stop = False

    for pno in pages:
        if stop:
            break
        page = doc[pno]
        bg = BgSampler(page)
        blocks = page.get_text("dict").get("blocks", [])

        prepared, pending = [], []
        for bidx, blk in enumerate(blocks):
            if blk.get("type", 0) != 0:
                continue
            lines = block_lines(blk)
            if not lines:
                continue
            raw = " ".join(s.get("text", "") for ln in blk.get("lines", [])
                           for s in ln.get("spans", []))
            en = " ".join(raw.split())
            if should_skip(en):
                st["skipped"] += 1
                continue

            # a ready-made translation wins over any backend
            bid = "%d:%d" % (pno, bidx)
            if bid in transmap:
                prepared.append({"lines": lines, "en": en, "units": [en],
                                 "tmpl": "{0}", "got": [transmap[bid]]})
                st["cached"] += 1
                continue

            units, tmpl = split_units(en)
            if should_skip(units[0]):
                st["skipped"] += 1
                continue

            got = []
            for u in units:
                z = cache.get(u)
                if z is None:
                    got.append(None)
                    key = (len(prepared), len(got) - 1)
                    pending.append((u, key))
                else:
                    got.append(z)
                    st["cached"] += 1
            prepared.append({"lines": lines, "en": en, "units": units,
                             "tmpl": tmpl, "got": got})

        # --- translate everything missing on this page ---
        if pending and not args.dry_run:
            res = backend.translate_many([(k, u) for u, k in pending])
            for u, (bi, ui) in pending:
                zh = res.get((bi, ui), "")
                if zh:
                    prepared[bi]["got"][ui] = zh
                    cache.put(u, zh)
                    st["new"] += 1
            if backend.quota_exhausted and any(
                    g is None for p in prepared for g in p["got"]):
                print("  quota exhausted - stopping after page %d" % pno)
                stop = True

        # --- render ---
        dirty = False
        for p in prepared:
            if any(g is None for g in p["got"]):
                st["skipped"] += 1
                continue
            zh = " ".join(p["tmpl"].format(*p["got"]).split())
            if not zh or zh == p["en"]:
                st["skipped"] += 1
                continue

            if args.dry_run:
                st["blocks"] += 1
                continue

            lines = p["lines"]
            capacity = sum(l["width"] for l in lines)
            unit = max(0.001, font_reg.text_length(zh, 1.0))
            size_fit = capacity / unit * 0.97
            base = min(max(l["size"] for l in lines), args.max_size)
            size = max(args.min_size, min(base, size_fit))
            bold = any(l["bold"] for l in lines)
            font = font_bold if bold else font_reg
            chunks = fit_lines(font, zh, lines, size)

            if not args.no_cover:
                for ln in lines:
                    page.draw_rect(ln["bbox"] + (-0.6, -0.6, 0.6, 0.6),
                                   color=None, fill=bg.color(ln["bbox"]), oc=ocg)
            for ln, chunk in zip(lines, chunks):
                if not chunk:
                    continue
                col = ln["color"]
                if isinstance(col, int):
                    rgb = (((col >> 16) & 255) / 255.0, ((col >> 8) & 255) / 255.0,
                           (col & 255) / 255.0)
                else:
                    rgb = (0, 0, 0)
                page.insert_text((ln["x"], ln["baseline"]), chunk, fontsize=size,
                                 fontname=("zhbold" if bold else "zhreg"),
                                 fontfile=(args.font_bold if bold else args.font),
                                 color=rgb, oc=ocg)
            st["blocks"] += 1
            dirty = True

        st["pages"] += 1
        if dirty and not args.trim and st["pages"] % args.save_every == 0:
            cache.save()
            try:
                doc.subset_fonts()          # CJK fonts are huge -> keep used glyphs only
            except Exception:
                pass
            doc.save(args.out, garbage=3, deflate=True)
            print("  p%-5d blocks=%d new=%d cached=%d words=%d" %
                  (pno, st["blocks"], st["new"], st["cached"], backend.words))
            sys.stdout.flush()

    cache.save()
    if not args.dry_run:
        if args.trim:
            doc.select(pages)
        try:
            doc.subset_fonts()
        except Exception as e:
            print("subset_fonts skipped:", e)
        doc.save(args.out, garbage=3, deflate=True)
    print("\nDONE pages=%d blocks=%d new=%d cached=%d skipped=%d words~%d" % (
        st["pages"], st["blocks"], st["new"], st["cached"], st["skipped"], backend.words))
    print("quota_exhausted:", backend.quota_exhausted)
    print("output:", "(dry run)" if args.dry_run else args.out)


if __name__ == "__main__":
    main()

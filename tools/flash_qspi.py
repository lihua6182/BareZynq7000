#!/usr/bin/env python3
"""Flash a boot image to QSPI through SEGGER J-Link Commander.

The .hex carries the XIP addresses inside the QSPI linear window, so J-Link's
QSPI flash algorithm programs the device directly.  After programming the image
is read back and compared against the .hex, because J-Link Commander keeps
running after a failed command and its exit code does not report failures.
"""

import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile
from typing import NoReturn

DEVICE = "XC7Z010"
QSPI_BASE = 0xFC000000
QSPI_WINDOW = 16 * 1024 * 1024
# IRPre,IRPost of the PS DAP (it sits behind the 6-bit PL TAP).  Without it
# J-Link asks for the chain position on stdin and eats the scripted commands.
JTAG_CONF = "6,1"
JLINK_ERRORS = ("Error occurred", "Could not", "Failed to")


def fail(msg: str) -> NoReturn:
    print(f"flash_qspi.py: error: {msg}", file=sys.stderr)
    sys.exit(1)


def jlink_version(path: str) -> int:
    m = re.search(r"JLink_V(\d+)", path, re.I)
    # an unversioned install dir is the maintained one, rank it newest
    return int(m.group(1)) if m else 10**9


def find_jlink(override: str | None) -> str:
    if override:
        return override
    on_path = shutil.which("JLink.exe") or shutil.which("JLink")
    if on_path:
        return on_path
    candidates = []
    for root in (r"C:\Program Files", r"C:\Program Files (x86)"):
        candidates += glob.glob(os.path.join(root, "SEGGER", "JLink*", "JLink.exe"))
    candidates = sorted(set(candidates), key=jlink_version)
    if not candidates:
        fail("JLink.exe not found on PATH or in Program Files/SEGGER (use --jlink)")
    return candidates[-1]


def load_hex(path: str) -> dict[int, int]:
    """Parse an Intel HEX file into an address -> byte map (holes are omitted)."""
    image: dict[int, int] = {}
    base = 0
    with open(path, encoding="ascii") as f:
        for lineno, line in enumerate(f, 1):
            line = line.strip()
            if not line:
                continue
            if not line.startswith(":") or len(line) < 11:
                fail(f"{path}:{lineno}: not an Intel HEX record")
            count = int(line[1:3], 16)
            offset = int(line[3:7], 16)
            kind = int(line[7:9], 16)
            data = bytes.fromhex(line[9 : 9 + 2 * count])
            if kind == 0:
                for i, byte in enumerate(data):
                    image[base + offset + i] = byte
            elif kind == 2:
                base = int(line[9:13], 16) << 4
            elif kind == 4:
                base = int(line[9:13], 16) << 16
    if not image:
        fail(f"{path}: no data records")
    return image


def image_range(image: dict[int, int]) -> tuple[int, int]:
    lo = min(image)
    hi = max(image) + 1
    if not QSPI_BASE <= lo < QSPI_BASE + QSPI_WINDOW:
        fail(f"image starts at 0x{lo:08X}, outside the QSPI linear window")
    return lo, hi


def build_commands(image_path: str, dump_path: str, lo: int, hi: int, write: bool) -> list[str]:
    cmds = ["h"]
    if write:
        cmds.append(f"loadfile {image_path}")
    cmds.append(f"savebin {dump_path}, 0x{lo:08X}, 0x{hi - lo:X}")
    if not write:
        cmds.append("g")  # paused for the readback only, resume the application
    cmds.append("qc")
    return cmds


def restart(jlink: str, speed: int) -> None:
    """Soft-reset the PS so the freshly programmed image starts.

    slcr.PSS_RST_CTRL = 1 resets the PS as soon as the write lands, so the AP
    transaction (and anything after it) can report "Failed to write memory".
    That is expected here and must not fail the run.
    """
    run_jlink(jlink, speed, ["w4 0xF8000008 0xDF0D", "w4 0xF8000240 0xF",
                             "w4 0xF8000200 0x1", "qc"], strict=False)


def run_jlink(jlink: str, speed: int, cmds: list[str], strict: bool = True) -> str:
    fd, script = tempfile.mkstemp(suffix=".jl", text=True)
    try:
        with os.fdopen(fd, "w", encoding="ascii") as f:
            f.write("\n".join(cmds) + "\n")
        result = subprocess.run(
            [jlink, "-device", DEVICE, "-if", "JTAG", "-speed", str(speed),
             "-jtagconf", JTAG_CONF, "-autoconnect", "1", "-NoGui", "1",
             "-CommandFile", script],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
            errors="replace")
    finally:
        os.unlink(script)
    output = result.stdout
    problems = [marker for marker in JLINK_ERRORS if marker in output]
    if not strict:
        if problems:
            print(f"flash_qspi.py: note: '{problems[0]}' (expected during the reset)")
        return output
    if problems:
        print(output)
        fail(f"J-Link reported '{problems[0]}'")
    if result.returncode != 0:
        print(output)
        fail(f"J-Link exited with {result.returncode}")
    return output


def verify(dump_path: str, image: dict[int, int], lo: int, hi: int) -> None:
    with open(dump_path, "rb") as f:
        dump = f.read()
    if len(dump) != hi - lo:
        fail(f"readback is {len(dump)} bytes, expected {hi - lo}")
    bad = [(addr, image[addr], dump[addr - lo]) for addr in sorted(image)
           if dump[addr - lo] != image[addr]]
    if bad:
        for addr, want, got in bad[:16]:
            print(f"  0x{addr:08X}: flash 0x{got:02X}, image 0x{want:02X}", file=sys.stderr)
        fail(f"{len(bad)} of {len(image)} bytes differ")
    print(f"flash_qspi.py: verified {len(image)} bytes")


def main() -> None:
    ap = argparse.ArgumentParser(
        description="Flash a boot image to QSPI through SEGGER J-Link Commander.")
    ap.add_argument("hexfile", help="image to flash (.hex)")
    ap.add_argument("--jlink", help="path to JLink.exe (default: auto-detect)")
    ap.add_argument("--speed", type=int, default=4000, help="JTAG speed in kHz (default: 4000)")
    ap.add_argument("--verify-only", action="store_true",
                    help="read the flash back and compare, without programming")
    args = ap.parse_args()

    if not os.path.isfile(args.hexfile):
        fail(f"image not found: {args.hexfile}")
    jlink = find_jlink(args.jlink)
    image = load_hex(args.hexfile)
    lo, hi = image_range(image)

    print(f"flash_qspi.py: {jlink}")
    print(f"flash_qspi.py: {os.path.basename(args.hexfile)} -> "
          f"0x{lo:08X}..0x{hi - 1:08X} ({len(image)} bytes)"
          f"{' [verify only]' if args.verify_only else ''}")

    fd, dump_path = tempfile.mkstemp(suffix=".bin")
    os.close(fd)
    try:
        cmds = build_commands(os.path.abspath(args.hexfile), dump_path, lo, hi, not args.verify_only)
        run_jlink(jlink, args.speed, cmds)
        verify(dump_path, image, lo, hi)
        if not args.verify_only:
            restart(jlink, args.speed)
    finally:
        if os.path.exists(dump_path):
            os.unlink(dump_path)


if __name__ == "__main__":
    main()

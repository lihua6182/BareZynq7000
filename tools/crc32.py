"""Print the ground-truth CRC32 for qspi_probe runs.

The flash is programmed from app.hex (Intel HEX): objcopy emits no records
for inter-section gaps, so the gaps stay erased (0xFF) in flash, while
app.bin zero-fills them. The probe's ground truth is therefore the CRC of
the .hex rendered with 0xFF fill over the app.bin span - NOT crc32(app.bin).
"""
import os
import sys
import zlib


def load_ihex(path):
    mem = {}
    base = 0
    for line in open(path):
        line = line.strip()
        if not line.startswith(":"):
            continue
        n = int(line[1:3], 16)
        addr = int(line[3:7], 16)
        typ = int(line[7:9], 16)
        data = bytes.fromhex(line[9 : 9 + 2 * n])
        if typ == 0x00:
            for i, b in enumerate(data):
                mem[base + addr + i] = b
        elif typ == 0x01:
            break
        elif typ == 0x04:
            base = int.from_bytes(data, "big") << 16
    return mem


def main():
    hex_path = sys.argv[1]
    span = os.path.getsize(sys.argv[2])  # app.bin size = probe span
    mem = load_ihex(hex_path)
    lo = min(mem)
    img = bytearray(b"\xff" * span)
    for a, b in mem.items():
        o = a - lo
        if 0 <= o < span:
            img[o] = b
    print(
        "crc32(%s rendered over %d bytes, gaps=0xFF) = %08X"
        % (hex_path, span, zlib.crc32(bytes(img)))
    )


if __name__ == "__main__":
    main()

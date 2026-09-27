#!/usr/bin/env python3
"""Validate the BootROM header of a raw Zynq-7000 QSPI XIP boot image.

Checks the fields of the 0x8C0-byte boot header (UG585 ch. 6, "BootROM
Header Parameters") against the rules the BootROM enforces before it hands
control to the image:

  - width detection word / image signature / QSPI config word
  - source offset: >= 0x8C0 and 64-byte aligned
  - execute-in-place selection: image length and total length both 0
  - XIP start of execution: absolute address inside the 16 MB QSPI linear
    window, at or past the header, 64-byte aligned
  - header checksum: one's complement of the sum of words 0x20..0x44

Run on the .bin produced by objcopy; exits non-zero on any mismatch so a
stale header fails the build instead of silently bricking the image.
"""

import argparse
import struct
import sys

HEADER_SIZE = 0x8C0
WIDTH_DETECTION_WORD = 0xAA995566
HEADER_SIGNATURE = 0x584C4E58  # 'XLNX'
QSPI_CONFIG_WORD = 0x00000001
QSPI_LINEAR_BASE = 0xFC000000
QSPI_LINEAR_SIZE = 16 * 1024 * 1024  # x4 single device


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("image", help="raw boot image (.bin, flash offset 0 at byte 0)")
    args = parser.parse_args()

    with open(args.image, "rb") as f:
        data = f.read()

    errors = []

    def word(offset):
        return struct.unpack_from("<I", data, offset)[0]

    if len(data) < HEADER_SIZE:
        errors.append(f"image is {len(data):#x} bytes, smaller than the 0x8C0 boot header")
    else:
        if word(0x20) != WIDTH_DETECTION_WORD:
            errors.append(f"width detection word is {word(0x20):#010x}, expected {WIDTH_DETECTION_WORD:#010x}")
        if word(0x24) != HEADER_SIGNATURE:
            errors.append(f"image signature is {word(0x24):#010x}, expected {HEADER_SIGNATURE:#010x}")
        if word(0x28) in (0xA5C3C5A3, 0x3A5C3C5A):
            errors.append("encryption status selects an encrypted image; only plain images are supported here")
        if word(0x44) != QSPI_CONFIG_WORD:
            errors.append(f"QSPI config word is {word(0x44):#010x}, expected {QSPI_CONFIG_WORD:#010x}")

        source_offset = word(0x30)
        image_length = word(0x34)
        exec_address = word(0x3C)
        total_length = word(0x40)

        if source_offset < HEADER_SIZE:
            errors.append(f"source offset {source_offset:#010x} points inside the boot header, must be >= 0x8C0")
        if source_offset % 64:
            errors.append(f"source offset {source_offset:#010x} is not 64-byte aligned")

        if image_length == 0:
            # Execute-in-place (UG585 Table 6-5, note 6)
            if total_length != 0:
                errors.append("XIP image: total image length must be 0 when the image length is 0")
            if not (QSPI_LINEAR_BASE + HEADER_SIZE <= exec_address < QSPI_LINEAR_BASE + QSPI_LINEAR_SIZE):
                errors.append(
                    f"XIP start of execution {exec_address:#010x} is outside the QSPI linear window "
                    f"[{QSPI_LINEAR_BASE + HEADER_SIZE:#010x}, {QSPI_LINEAR_BASE + QSPI_LINEAR_SIZE:#010x})"
                )
            if exec_address % 64:
                errors.append(f"start of execution {exec_address:#010x} is not 64-byte aligned")
        elif total_length != image_length:
            errors.append("non-secure boot: total image length must equal the image length")

        checksum = word(0x48)
        expected = ~sum(word(off) for off in range(0x20, 0x48, 4)) & 0xFFFFFFFF
        if checksum != expected:
            errors.append(f"header checksum is {checksum:#010x}, expected {expected:#010x}")

    if errors:
        for error in errors:
            print(f"error: {error}", file=sys.stderr)
        sys.exit(1)

    print(f"BootROM header OK ({args.image}, {len(data)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())

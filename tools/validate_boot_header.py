#!/usr/bin/env python3
"""Validate the BootROM header of a raw Zynq-7000 boot image."""

import argparse
import os
import struct
import sys
from typing import NoReturn

HEADER_SIZE = 0x8C0
HEAD_BYTES = 80  # every checked field sits within 0x20..0x48
QSPI_BASE = 0xFC000000
QSPI_WINDOW = 16 * 1024 * 1024

WIDTH_DETECT_WORD = 0xAA995566
SIGNATURE_WORD = 0x584C4E58  # 'XLNX'
ENCRYPTED_WORDS = (0xA5C3C5A3, 0x3A5C3C5A)
QSPI_CONFIG_WORD = 0x00000001
CHECKSUM_OFFSETS = range(0x20, 0x48, 4)  # 0x20..0x44; the checksum word itself is 0x48


def fail(msg: str) -> NoReturn:
    print(f"validate_boot_header.py: error: {msg}", file=sys.stderr)
    sys.exit(1)


def word_le(data: bytes, off: int) -> int:
    return struct.unpack_from("<I", data, off)[0]


def check_header(head: bytes, size: int) -> list[str]:
    errors: list[str] = []
    if size < HEADER_SIZE:
        errors.append(f"image is {size} bytes, smaller than the 0x8C0 boot header")
        return errors

    width_detect = word_le(head, 0x20)
    signature = word_le(head, 0x24)
    encryption = word_le(head, 0x28)
    source_offset = word_le(head, 0x30)
    image_length = word_le(head, 0x34)
    exec_address = word_le(head, 0x3C)
    total_length = word_le(head, 0x40)
    qspi_config = word_le(head, 0x44)
    checksum = word_le(head, 0x48)

    if width_detect != WIDTH_DETECT_WORD:
        errors.append(f"width detection word is 0x{width_detect:08X}, expected 0xAA995566")
    if signature != SIGNATURE_WORD:
        errors.append(f"image signature is 0x{signature:08X}, expected 0x584C4E58 ('XLNX')")
    if encryption in ENCRYPTED_WORDS:
        errors.append("encryption status selects an encrypted image; "
                      "only plain images are supported here")
    if qspi_config != QSPI_CONFIG_WORD:
        errors.append(f"QSPI config word is 0x{qspi_config:08X}, expected 0x00000001")

    if source_offset < HEADER_SIZE:
        errors.append(f"source offset 0x{source_offset:08X} points inside the boot header, must be >= 0x8C0")
    if source_offset % 64:
        errors.append(f"source offset 0x{source_offset:08X} is not 64-byte aligned")

    if image_length == 0:
        # image length 0 marks XIP: BootROM jumps straight into the QSPI window.
        if total_length != 0:
            errors.append("XIP image: total image length must be 0 when the image length is 0")
        exec_lo = QSPI_BASE + HEADER_SIZE
        exec_hi = QSPI_BASE + QSPI_WINDOW
        if not exec_lo <= exec_address < exec_hi:
            errors.append(f"XIP start of execution 0x{exec_address:08X} is outside the QSPI "
                          f"linear window [0x{exec_lo:08X}, 0x{exec_hi:08X})")
        if exec_address % 64:
            errors.append(f"start of execution 0x{exec_address:08X} is not 64-byte aligned")
    elif total_length != image_length:
        errors.append("non-secure boot: total image length must equal the image length")

    total = sum(word_le(head, off) for off in CHECKSUM_OFFSETS) & 0xFFFFFFFF
    expected = ~total & 0xFFFFFFFF
    if checksum != expected:
        errors.append(f"header checksum is 0x{checksum:08X}, expected 0x{expected:08X}")

    return errors


def main() -> None:
    ap = argparse.ArgumentParser(
        description="Validate the BootROM header of a raw Zynq-7000 boot image.")
    ap.add_argument("image", help="raw boot image (.bin) carrying a BootROM header")
    args = ap.parse_args()

    if not os.path.isfile(args.image):
        fail(f"image not found: {args.image}")
    size = os.path.getsize(args.image)
    with open(args.image, "rb") as f:
        head = f.read(HEAD_BYTES)

    errors = check_header(head, size)
    if errors:
        for e in errors:
            print(f"validate_boot_header.py: error: {e}", file=sys.stderr)
        sys.exit(1)
    print(f"validate_boot_header.py: BootROM header OK ({args.image}, {size} bytes)")


if __name__ == "__main__":
    main()

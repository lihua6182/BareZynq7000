#!/usr/bin/env python3
"""Measure USB CDC throughput over a serial port."""

from __future__ import annotations

import sys
import time

import serial

PORT = "COM12"
BAUD_RATE = 2_000_000
MAX_BYTES = 10 * 1024 * 1024
CHUNK = b"0xAA" * 4096


def measure_out(port: serial.Serial) -> float:
    sent = 0
    start = time.monotonic()
    while sent < MAX_BYTES:
        sent += port.write(CHUNK)
    return sent / (time.monotonic() - start) / (1024 * 1024)


def measure_in(port: serial.Serial) -> float:
    received = 0
    start = time.monotonic()
    while received < MAX_BYTES:
        chunk = port.read(MAX_BYTES)
        if not chunk:
            break
        received += len(chunk)
    return received / (time.monotonic() - start) / (1024 * 1024)


def main() -> int:
    try:
        port = serial.Serial(PORT, BAUD_RATE, timeout=1)
    except serial.SerialException as exc:
        print(f"cannot open {PORT}: {exc}", file=sys.stderr)
        return 1

    with port:
        print("test cdc out speed")
        port.setDTR(0)
        print(f"cdc out speed {measure_out(port):.3f} MB/s")

        print("test cdc in speed")
        port.setDTR(1)
        print(f"cdc in speed {measure_in(port):.3f} MB/s")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3
"""
Pack a 16 KiB raw C64 cartridge ROM into a CCS64/VICE .crt container.

Cartridge metadata: hardware type 0, EXROM and GAME inactive (normal 16 KiB per CRT.TXT).

**One CHIP packet** (16 KiB @ $8000, size `$4000`), per **CRT.TXT** hardware type **0** (“Adventure
Creator” layout). Type **0** with two 8K CHIPs is **not** valid in that spec (8K+8K examples use
other hardware types); VICE may reject such files with “Failed to attach image”.

Usage:
  pack_crt.py <input-16k.bin> <output.crt> [cartridge-name]

The input must be exactly 16384 bytes (Quattro: build/quattro-cart-8000.rom from make c64_cart).
"""

from __future__ import annotations

import struct
import sys

CRT_SIGNATURE = b"C64 CARTRIDGE   "  # 16 bytes, space-padded
HEADER_LENGTH = 0x40
CRT_VERSION = 0x0100
# Type 0 = generic "normal" cartridge (8K/16K per EXROM/GAME and CHIP layout).
HW_TYPE_NORMAL = 0
# Normal 16K: both lines low per CRT.TXT memory map table.
EXROM_INACTIVE = 0
GAME_INACTIVE = 0
CHIP_SIG = b"CHIP"
CHIP_HEADER_LEN = 16
CHIP_TYPE_ROM = 0


def _chip_packet(load_addr: int, rom_image: bytes) -> bytes:
    if len(rom_image) not in (0x2000, 0x4000):
        raise SystemExit("internal: chip ROM must be 8192 or 16384 bytes")
    total = CHIP_HEADER_LEN + len(rom_image)
    pkt = bytearray()
    pkt += CHIP_SIG
    pkt += struct.pack(">I", total)
    pkt += struct.pack(">H", CHIP_TYPE_ROM)
    pkt += struct.pack(">H", 0)  # bank
    pkt += struct.pack(">H", load_addr)
    pkt += struct.pack(">H", len(rom_image))
    pkt += rom_image
    return bytes(pkt)


def _pack_crt(rom: bytes, name: str) -> bytes:
    if len(rom) != 0x4000:
        raise SystemExit(f"expected exactly 16384-byte ROM, got {len(rom)}")

    name_bytes = name.encode("ascii", errors="replace")[:31]
    name_field = name_bytes.ljust(32, b"\x00")

    header = bytearray(HEADER_LENGTH)
    header[0:16] = CRT_SIGNATURE
    struct.pack_into(">I", header, 16, HEADER_LENGTH)
    struct.pack_into(">H", header, 20, CRT_VERSION)
    struct.pack_into(">H", header, 22, HW_TYPE_NORMAL)
    header[24] = EXROM_INACTIVE
    header[25] = GAME_INACTIVE
    # 26–31 reserved (already zero)
    header[32:64] = name_field

    return bytes(header) + _chip_packet(0x8000, rom)


def main() -> None:
    argv = sys.argv[1:]
    if len(argv) < 2:
        print(__doc__.strip(), file=sys.stderr)
        sys.exit(2)
    in_path, out_path = argv[0], argv[1]
    cart_name = argv[2] if len(argv) > 2 else "QUATTRO"

    with open(in_path, "rb") as f:
        rom = f.read()
    out = _pack_crt(rom, cart_name)
    with open(out_path, "wb") as f:
        f.write(out)
    print(
        f"Wrote {out_path} ({len(out)} bytes), type=0 16K: one CHIP $8000+$4000, EXROM=GAME=0"
    )


if __name__ == "__main__":
    main()

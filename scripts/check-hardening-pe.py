#!/usr/bin/env python3
"""Verify that Windows PE binaries were built with the expected exploit mitigations.

    python scripts/check-hardening-pe.py [--require-cet] FILE...

Fails if a file lacks ASLR (DYNAMIC_BASE, plus HIGH_ENTROPY_VA on 64-bit), DEP
(NX_COMPAT) or Control Flow Guard (GUARD_CF). --require-cet also requires the
CET shadow stack compatibility flag (/CETCOMPAT).

Only the standard library is used, so it runs on any CI runner.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

HIGH_ENTROPY_VA = 0x0020
DYNAMIC_BASE = 0x0040
NX_COMPAT = 0x0100
GUARD_CF = 0x4000
DEBUG_TYPE_EX_DLLCHARACTERISTICS = 20
EX_CET_COMPAT = 0x0001
DEBUG_DIRECTORY_INDEX = 6


class PeError(Exception):
    pass


def u16(data: bytes, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def u32(data: bytes, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def inspect(data: bytes) -> tuple[bool, int, int | None]:
    """Return (is_64_bit, dll_characteristics, ex_dll_characteristics or None)."""
    if data[:2] != b"MZ":
        raise PeError("not a PE file (no MZ header)")
    pe = u32(data, 0x3C)
    if data[pe : pe + 4] != b"PE\0\0":
        raise PeError("not a PE file (no PE signature)")

    coff = pe + 4
    section_count = u16(data, coff + 2)
    optional_size = u16(data, coff + 16)
    optional = coff + 20

    magic = u16(data, optional)
    if magic == 0x20B:
        is_64_bit, directories = True, optional + 112
    elif magic == 0x10B:
        is_64_bit, directories = False, optional + 96
    else:
        raise PeError(f"unknown optional header magic 0x{magic:x}")

    dll_characteristics = u16(data, optional + 70)
    directory_count = u32(data, directories - 4)

    sections = []
    table = optional + optional_size
    for index in range(section_count):
        entry = table + 40 * index
        virtual_size = u32(data, entry + 8)
        virtual_address = u32(data, entry + 12)
        raw_size = u32(data, entry + 16)
        raw_pointer = u32(data, entry + 20)
        sections.append((virtual_address, max(virtual_size, raw_size), raw_pointer))

    def rva_to_offset(rva: int) -> int:
        for virtual_address, size, raw_pointer in sections:
            if virtual_address <= rva < virtual_address + size:
                return raw_pointer + rva - virtual_address
        raise PeError(f"RVA 0x{rva:x} is outside every section")

    ex_dll_characteristics = None
    if directory_count > DEBUG_DIRECTORY_INDEX:
        debug_rva = u32(data, directories + 8 * DEBUG_DIRECTORY_INDEX)
        debug_size = u32(data, directories + 8 * DEBUG_DIRECTORY_INDEX + 4)
        if debug_rva and debug_size:
            start = rva_to_offset(debug_rva)
            for entry in range(start, start + debug_size, 28):
                if u32(data, entry + 12) == DEBUG_TYPE_EX_DLLCHARACTERISTICS:
                    ex_dll_characteristics = u32(data, u32(data, entry + 24))
    return is_64_bit, dll_characteristics, ex_dll_characteristics


def check(path: Path, require_cet: bool) -> bool:
    print(f"== {path}")
    try:
        is_64_bit, flags, ex_flags = inspect(path.read_bytes())
    except (OSError, PeError, struct.error) as error:
        print(f"  FAIL: cannot read PE headers: {error}")
        return False

    required = [
        (DYNAMIC_BASE, "ASLR (DYNAMIC_BASE)"),
        (NX_COMPAT, "DEP (NX_COMPAT)"),
        (GUARD_CF, "Control Flow Guard (GUARD_CF)"),
    ]
    if is_64_bit:
        required.append((HIGH_ENTROPY_VA, "high-entropy ASLR (HIGH_ENTROPY_VA)"))

    ok = True
    for flag, name in required:
        if flags & flag:
            print(f"  ok:   {name}")
        else:
            print(f"  FAIL: {name} not enabled")
            ok = False

    if require_cet:
        if ex_flags is not None and ex_flags & EX_CET_COMPAT:
            print("  ok:   CET shadow stack compatible (CETCOMPAT)")
        else:
            print("  FAIL: CET shadow stack compatibility (/CETCOMPAT) not enabled")
            ok = False
    return ok


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--require-cet", action="store_true")
    parser.add_argument("files", nargs="+", type=Path)
    args = parser.parse_args()
    results = [check(path, args.require_cet) for path in args.files]
    return 0 if all(results) else 1


if __name__ == "__main__":
    sys.exit(main())

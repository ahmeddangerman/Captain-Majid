#!/usr/bin/env python3
"""Export the fixed-bank dictionary used by dialogue substitutions."""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path

from render_text_glyphs import load_rom

FIXED_BANK_ADDRESS = 0xE000
POINTER_TABLE_ADDRESS = 0xF329
POINTER_COUNT = 0xF0
RUNTIME_BUFFER_ADDRESS = 0x05EB


def read_dictionary(prg: bytes) -> list[dict[str, object]]:
    fixed_bank = prg[-0x2000:]
    table_offset = POINTER_TABLE_ADDRESS - FIXED_BANK_ADDRESS
    table = fixed_bank[table_offset : table_offset + POINTER_COUNT * 2]
    if len(table) != POINTER_COUNT * 2:
        raise ValueError("Dictionary pointer table is truncated")

    pointers = [table[i] | table[i + 1] << 8 for i in range(0, len(table), 2)]
    aliases = Counter(pointers)
    entries: list[dict[str, object]] = []
    for index, pointer in enumerate(pointers):
        if pointer < FIXED_BANK_ADDRESS:
            if index != 0 or pointer != RUNTIME_BUFFER_ADDRESS:
                raise ValueError(
                    f"Unexpected non-ROM dictionary pointer ${pointer:04X} "
                    f"at index ${index:02X}"
                )
            entries.append(
                {
                    "index": index,
                    "index_hex": f"{index:02X}",
                    "pointer": pointer,
                    "pointer_hex": f"{pointer:04X}",
                    "kind": "runtime_buffer",
                    "length": None,
                    "bytes": None,
                    "hex": None,
                    "terminator": None,
                    "alias_count": aliases[pointer],
                }
            )
            continue

        offset = pointer - FIXED_BANK_ADDRESS
        data = bytearray()
        terminator = None
        for value in fixed_bank[offset:]:
            if value >= 0xE0:
                terminator = value
                break
            data.append(value)
        if terminator is None:
            raise ValueError(
                f"Unterminated dictionary entry ${index:02X} at ${pointer:04X}"
            )
        entries.append(
            {
                "index": index,
                "index_hex": f"{index:02X}",
                "pointer": pointer,
                "pointer_hex": f"{pointer:04X}",
                "kind": "rom_string",
                "length": len(data),
                "bytes": list(data),
                "hex": bytes(data).hex(" "),
                "terminator": terminator,
                "terminator_hex": f"{terminator:02X}",
                "alias_count": aliases[pointer],
            }
        )
    return entries


def export_dictionary(rom_path: Path, output: Path) -> None:
    rom = rom_path.read_bytes()
    prg, _ = load_rom(rom_path)
    entries = read_dictionary(prg)
    unique_pointers = len({entry["pointer"] for entry in entries})
    document = {
        "format": "captain-majid-dictionary-v1",
        "source_name": rom_path.name,
        "source_sha256": hashlib.sha256(rom).hexdigest(),
        "pointer_table_address": POINTER_TABLE_ADDRESS,
        "pointer_table_address_hex": f"{POINTER_TABLE_ADDRESS:04X}",
        "entry_count": len(entries),
        "unique_pointer_count": unique_pointers,
        "entries": entries,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    print(
        f"{output}: {len(entries)} entries, {unique_pointers} unique pointers, "
        f"table=${POINTER_TABLE_ADDRESS:04X}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    export_dictionary(args.rom, args.output)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Export a self-indexed $A000-$BFFF scenario-script PRG bank as JSON."""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path

from render_text_glyphs import load_rom

PRG_BANK_SIZE = 8 * 1024
CPU_BASE = 0xA000
CPU_END = 0xC000


def extract_bank(prg: bytes, bank_index: int) -> tuple[list[int], list[dict[str, object]]]:
    start = bank_index * PRG_BANK_SIZE
    bank = prg[start : start + PRG_BANK_SIZE]
    if len(bank) != PRG_BANK_SIZE:
        raise ValueError(f"PRG bank {bank_index} is missing or truncated")

    first_pointer = bank[0] | bank[1] << 8
    table_size = first_pointer - CPU_BASE
    if table_size <= 0 or table_size % 2 or table_size > PRG_BANK_SIZE:
        raise ValueError(
            f"Bank {bank_index} does not begin with a self-indexed $A000 table"
        )
    count = table_size // 2
    pointers = [bank[i] | bank[i + 1] << 8 for i in range(0, table_size, 2)]
    invalid = [pointer for pointer in pointers if not CPU_BASE <= pointer < CPU_END]
    if invalid:
        raise ValueError(f"Invalid script pointer ${invalid[0]:04X}")

    pointer_counts = Counter(pointers)
    records: list[dict[str, object]] = []
    for index, pointer in enumerate(pointers):
        later = [candidate for candidate in pointers if candidate > pointer]
        end_pointer = min(later) if later else CPU_END
        data = bank[pointer - CPU_BASE : end_pointer - CPU_BASE]
        controls = Counter(value for value in data if value >= 0xE0)
        records.append(
            {
                "index": index,
                "index_hex": f"{index:02X}",
                "address": pointer,
                "address_hex": f"{pointer:04X}",
                "end_address": end_pointer,
                "end_address_hex": f"{end_pointer:04X}",
                "end_at_bank_boundary": end_pointer == CPU_END,
                "aliases": [i for i, candidate in enumerate(pointers) if candidate == pointer],
                "alias_count": pointer_counts[pointer],
                "length": len(data),
                "bytes": list(data),
                "hex": data.hex(" "),
                "controls": {
                    f"{code:02X}": occurrences
                    for code, occurrences in sorted(controls.items())
                },
            }
        )
    return pointers, records


def export_bank(rom_path: Path, output: Path, bank_index: int) -> None:
    rom = rom_path.read_bytes()
    prg, _ = load_rom(rom_path)
    pointers, records = extract_bank(prg, bank_index)
    unique_count = len(set(pointers))
    document = {
        "format": "captain-majid-script-bank-v1",
        "source_name": rom_path.name,
        "source_sha256": hashlib.sha256(rom).hexdigest(),
        "prg_bank": bank_index,
        "mapped_cpu_range": "A000-BFFF",
        "record_count": len(records),
        "unique_record_count": unique_count,
        "pointer_order_monotonic": all(
            left <= right for left, right in zip(pointers, pointers[1:])
        ),
        "records": records,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    print(
        f"{output}: bank={bank_index}, {len(records)} entries, "
        f"{unique_count} unique records"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--bank", type=int, required=True)
    args = parser.parse_args()
    export_bank(args.rom, args.output, args.bank)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Export the bank-24 dialogue pointer table and records as JSON."""

from __future__ import annotations

import argparse
import hashlib
import json
from collections import Counter
from pathlib import Path

from dialogue import (
    DEFAULT_RECORD_COUNT,
    DEFAULT_TABLE_ADDRESS,
    read_dialogue_record,
)
from render_text_glyphs import load_rom


def export_dialogue(
    rom_path: Path,
    output: Path,
    first_bank: int,
    table_address: int,
    count: int,
) -> None:
    rom = rom_path.read_bytes()
    prg, _ = load_rom(rom_path)
    records = []
    unique_addresses: set[int] = set()

    for index in range(count):
        record = read_dialogue_record(
            prg,
            index,
            first_bank=first_bank,
            table_address=table_address,
            count=count,
        )
        controls = Counter(value for value in record.data if value >= 0xE0)
        unique_addresses.add(record.address)
        records.append(
            {
                "index": index,
                "index_hex": f"{index:02X}",
                "address": record.address,
                "address_hex": f"{record.address:04X}",
                "end_address": record.end_address,
                "end_address_hex": f"{record.end_address:04X}",
                "end_inferred": record.end_inferred,
                "aliases": list(record.aliases),
                "length": len(record.data),
                "bytes": list(record.data),
                "hex": record.data.hex(" "),
                "controls": {
                    f"{code:02X}": occurrences
                    for code, occurrences in sorted(controls.items())
                },
            }
        )

    document = {
        "format": "captain-majid-dialogue-v1",
        "source_name": rom_path.name,
        "source_sha256": hashlib.sha256(rom).hexdigest(),
        "first_prg_bank": first_bank,
        "mapped_cpu_range": "8000-BFFF",
        "pointer_table_address": table_address,
        "pointer_table_address_hex": f"{table_address:04X}",
        "record_count": count,
        "unique_record_count": len(unique_addresses),
        "records": records,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(document, indent=2) + "\n", encoding="utf-8")
    print(
        f"{output}: {count} entries, {len(unique_addresses)} unique records, "
        f"table=${table_address:04X}, banks={first_bank}/{first_bank + 1}"
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--bank", type=int, default=24)
    parser.add_argument(
        "--table", type=lambda value: int(value, 0), default=DEFAULT_TABLE_ADDRESS
    )
    parser.add_argument("--count", type=lambda value: int(value, 0), default=DEFAULT_RECORD_COUNT)
    args = parser.parse_args()
    export_dialogue(args.rom, args.output, args.bank, args.table, args.count)


if __name__ == "__main__":
    main()

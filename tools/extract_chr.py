#!/usr/bin/env python3
"""Extract NES CHR-ROM banks into grayscale tile sheets."""

from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path

from PIL import Image

INES_HEADER_SIZE = 16
TRAINER_SIZE = 512
PRG_UNIT = 16 * 1024
CHR_UNIT = 8 * 1024
TILE_BYTES = 16
TILES_PER_BANK = CHR_UNIT // TILE_BYTES
SHEET_COLUMNS = 16
SHEET_ROWS = TILES_PER_BANK // SHEET_COLUMNS
PALETTE = (0, 85, 170, 255)


def decode_tile(tile: bytes) -> list[int]:
    if len(tile) != TILE_BYTES:
        raise ValueError("NES tiles must contain exactly 16 bytes")
    pixels: list[int] = []
    for row in range(8):
        low = tile[row]
        high = tile[row + 8]
        for bit in range(7, -1, -1):
            color = ((high >> bit) & 1) << 1 | ((low >> bit) & 1)
            pixels.append(PALETTE[color])
    return pixels


def read_chr(rom_path: Path) -> tuple[bytes, dict[str, object]]:
    rom = rom_path.read_bytes()
    header = rom[:INES_HEADER_SIZE]
    if len(header) != INES_HEADER_SIZE or header[:4] != b"NES\x1a":
        raise ValueError(f"{rom_path} is not an iNES ROM")

    trainer_size = TRAINER_SIZE if header[6] & 0x04 else 0
    prg_size = header[4] * PRG_UNIT
    chr_size = header[5] * CHR_UNIT
    chr_start = INES_HEADER_SIZE + trainer_size + prg_size
    chr_data = rom[chr_start : chr_start + chr_size]
    if len(chr_data) != chr_size:
        raise ValueError(f"{rom_path} is truncated in its CHR region")

    metadata = {
        "source_name": rom_path.name,
        "source_sha256": hashlib.sha256(rom).hexdigest(),
        "prg_size": prg_size,
        "chr_size": chr_size,
        "chr_banks": chr_size // CHR_UNIT,
    }
    return chr_data, metadata


def render_bank(bank: bytes, scale: int) -> Image.Image:
    sheet = Image.new("L", (SHEET_COLUMNS * 8, SHEET_ROWS * 8))
    for tile_index in range(TILES_PER_BANK):
        start = tile_index * TILE_BYTES
        tile = Image.new("L", (8, 8))
        tile.putdata(decode_tile(bank[start : start + TILE_BYTES]))
        x = (tile_index % SHEET_COLUMNS) * 8
        y = (tile_index // SHEET_COLUMNS) * 8
        sheet.paste(tile, (x, y))
    if scale > 1:
        sheet = sheet.resize(
            (sheet.width * scale, sheet.height * scale),
            Image.Resampling.NEAREST,
        )
    return sheet


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--scale", type=int, default=3)
    args = parser.parse_args()
    if args.scale < 1:
        parser.error("--scale must be at least 1")

    chr_data, metadata = read_chr(args.rom)
    args.output.mkdir(parents=True, exist_ok=True)
    for bank_index in range(metadata["chr_banks"]):
        start = bank_index * CHR_UNIT
        image = render_bank(chr_data[start : start + CHR_UNIT], args.scale)
        image.save(args.output / f"chr-bank-{bank_index:02d}.png")
    (args.output / "metadata.json").write_text(
        json.dumps(metadata, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps(metadata))


if __name__ == "__main__":
    main()

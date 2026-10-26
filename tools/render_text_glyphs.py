#!/usr/bin/env python3
"""Render the game's byte-to-8x16-glyph mapping as a labeled atlas."""

from __future__ import annotations

import argparse
import json
from pathlib import Path

from PIL import Image, ImageDraw
from py65.devices.mpu6502 import MPU

INES_HEADER_SIZE = 16
PRG_UNIT = 16 * 1024
CHR_UNIT = 8 * 1024
GLYPH_MAPPER = 0xCBC2
RETURN_SENTINEL = 0x0200


def decode_tile(tile: bytes) -> Image.Image:
    pixels = []
    for row in range(8):
        low = tile[row]
        high = tile[row + 8]
        for bit in range(7, -1, -1):
            color = ((high >> bit) & 1) << 1 | ((low >> bit) & 1)
            pixels.append((0, 85, 170, 255)[color])
    image = Image.new("L", (8, 8))
    image.putdata(pixels)
    return image


def load_rom(rom_path: Path) -> tuple[bytes, bytes]:
    rom = rom_path.read_bytes()
    header = rom[:INES_HEADER_SIZE]
    if len(header) != INES_HEADER_SIZE or header[:4] != b"NES\x1a":
        raise ValueError(f"{rom_path} is not an iNES ROM")
    if header[6] & 0x04:
        raise ValueError("Trainer-bearing ROMs are not supported yet")
    mapper = (header[6] >> 4) | (header[7] & 0xF0)
    if mapper != 4:
        raise ValueError(f"Expected MMC3 mapper 4, found mapper {mapper}")
    prg_size = header[4] * PRG_UNIT
    chr_size = header[5] * CHR_UNIT
    prg = rom[INES_HEADER_SIZE : INES_HEADER_SIZE + prg_size]
    chr_data = rom[
        INES_HEADER_SIZE + prg_size : INES_HEADER_SIZE + prg_size + chr_size
    ]
    if len(prg) != prg_size or len(chr_data) != chr_size:
        raise ValueError("ROM is truncated")
    return prg, chr_data


def map_glyph(prg: bytes, code: int) -> tuple[int, int]:
    mpu = MPU()
    mpu.memory[0xC000:0xE000] = prg[-0x4000:-0x2000]
    mpu.memory[0xE000:0x10000] = prg[-0x2000:]
    mpu.a = code
    mpu.y = 0
    mpu.pc = GLYPH_MAPPER
    mpu.sp = 0xFD
    return_address = RETURN_SENTINEL - 1
    mpu.memory[0x01FE] = return_address & 0xFF
    mpu.memory[0x01FF] = return_address >> 8
    for _ in range(100):
        mpu.step()
        if mpu.pc == RETURN_SENTINEL:
            return mpu.y, mpu.a
    raise RuntimeError(f"Glyph mapper did not return for code ${code:02X}")


def render(rom_path: Path, output: Path, scale: int) -> None:
    prg, chr_data = load_rom(rom_path)
    tile_cache = [
        decode_tile(chr_data[index * 16 : index * 16 + 16]) for index in range(256)
    ]
    columns = 16
    rows = 14
    cell_width = 28
    cell_height = 44
    atlas = Image.new("L", (columns * cell_width, rows * cell_height), 24)
    draw = ImageDraw.Draw(atlas)
    mappings: dict[str, dict[str, int]] = {}

    for code in range(0xE0):
        top, bottom = map_glyph(prg, code)
        mappings[f"{code:02X}"] = {"top": top, "bottom": bottom}
        x = (code % columns) * cell_width
        y = (code // columns) * cell_height
        draw.text((x + 2, y + 1), f"{code:02X}", fill=220)
        atlas.paste(tile_cache[top].resize((8 * scale, 8 * scale)), (x + 2, y + 10))
        atlas.paste(
            tile_cache[bottom].resize((8 * scale, 8 * scale)),
            (x + 2, y + 10 + 8 * scale),
        )

    output.parent.mkdir(parents=True, exist_ok=True)
    atlas.save(output)
    output.with_suffix(".json").write_text(
        json.dumps(mappings, indent=2) + "\n", encoding="utf-8"
    )
    print(output)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--scale", type=int, default=2)
    args = parser.parse_args()
    if args.scale < 1 or args.scale > 3:
        parser.error("--scale must be between 1 and 3")
    render(args.rom, args.output, args.scale)


if __name__ == "__main__":
    main()

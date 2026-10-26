#!/usr/bin/env python3
"""Render a message referenced by a 16-bit pointer table in a PRG bank."""

from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image, ImageDraw

from dialogue import read_dialogue_record, text_payload
from render_text_glyphs import decode_tile, load_rom, map_glyph


def tokenize(message: bytes) -> list[list[int | str]]:
    lines: list[list[int | str]] = [[]]
    index = 0
    while index < len(message):
        code = message[index]
        index += 1
        if code < 0xE0:
            lines[-1].append(code)
        elif code == 0xFC:
            lines.append([])
        elif code == 0xFF:
            break
        elif code == 0xF7 and index < len(message):
            lines[-1].extend([0x7C] * message[index])
            index += 1
        else:
            lines[-1].append(f"[{code:02X}]")
    return lines


def render_message(
    prg: bytes, chr_data: bytes, message: bytes, output: Path, scale: int
) -> None:
    lines = tokenize(message)
    glyph_width = 8 * scale
    glyph_height = 16 * scale
    control_width = 28
    line_height = glyph_height + 8
    line_widths = [
        sum(glyph_width if isinstance(token, int) else control_width for token in line)
        for line in lines
    ]
    width = max(320, max(line_widths, default=0) + 16)
    height = max(64, len(lines) * line_height + 12)
    canvas = Image.new("RGB", (width, height), "black")
    draw = ImageDraw.Draw(canvas)
    tile_cache = [
        decode_tile(chr_data[tile * 16 : tile * 16 + 16]).resize(
            (8 * scale, 8 * scale), Image.Resampling.NEAREST
        )
        for tile in range(256)
    ]

    y = 6
    for line in lines:
        x = 8
        for token in line:
            if isinstance(token, int):
                top, bottom = map_glyph(prg, token)
                canvas.paste(tile_cache[top].convert("RGB"), (x, y))
                canvas.paste(tile_cache[bottom].convert("RGB"), (x, y + 8 * scale))
                x += glyph_width
            else:
                draw.rectangle((x, y + 5, x + control_width - 3, y + 20), outline="#666")
                draw.text((x + 2, y + 6), token, fill="#f2c94c")
                x += control_width
        y += line_height
    output.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(output)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument("--bank", type=int, default=24)
    parser.add_argument("--table", type=lambda value: int(value, 0), default=0x9220)
    parser.add_argument("--index", type=lambda value: int(value, 0), required=True)
    parser.add_argument("--scale", type=int, default=3)
    args = parser.parse_args()
    prg, chr_data = load_rom(args.rom)
    record = read_dialogue_record(prg, args.index, args.bank, args.table)
    message = text_payload(record.data)
    render_message(prg, chr_data, message, args.output, args.scale)
    print(
        f"index=${args.index:02X} address=${record.address:04X} "
        f"end=${record.end_address:04X} aliases={list(record.aliases)} "
        f"bytes={record.data.hex(' ')}"
    )


if __name__ == "__main__":
    main()

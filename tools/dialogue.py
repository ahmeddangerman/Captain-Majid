#!/usr/bin/env python3
"""Shared helpers for the game's banked dialogue data."""

from __future__ import annotations

from dataclasses import dataclass

PRG_BANK_SIZE = 8 * 1024
SCRIPT_WINDOW_START = 0x8000
SCRIPT_WINDOW_END = 0xC000
DEFAULT_TABLE_ADDRESS = 0x9220
DEFAULT_RECORD_COUNT = 0xF0


@dataclass(frozen=True)
class DialogueRecord:
    index: int
    address: int
    end_address: int
    data: bytes
    aliases: tuple[int, ...]
    end_inferred: bool = False


def script_window(prg: bytes, first_bank: int) -> bytes:
    """Return the two consecutive 8 KiB banks mapped at $8000-$BFFF."""
    start = first_bank * PRG_BANK_SIZE
    end = start + 2 * PRG_BANK_SIZE
    window = prg[start:end]
    if len(window) != 2 * PRG_BANK_SIZE:
        raise ValueError(
            f"PRG does not contain banks {first_bank} and {first_bank + 1}"
        )
    return window


def read_pointer_table(
    prg: bytes,
    first_bank: int = 24,
    table_address: int = DEFAULT_TABLE_ADDRESS,
    count: int = DEFAULT_RECORD_COUNT,
) -> tuple[bytes, list[int]]:
    if count < 1:
        raise ValueError("Dialogue pointer count must be positive")
    window = script_window(prg, first_bank)
    table_offset = table_address - SCRIPT_WINDOW_START
    table_end = table_offset + count * 2
    if table_offset < 0 or table_end > len(window):
        raise ValueError("Dialogue pointer table falls outside $8000-$BFFF")

    pointers = [
        window[offset] | window[offset + 1] << 8
        for offset in range(table_offset, table_end, 2)
    ]
    invalid = [
        pointer
        for pointer in pointers
        if not SCRIPT_WINDOW_START <= pointer < SCRIPT_WINDOW_END
    ]
    if invalid:
        raise ValueError(f"Invalid dialogue pointer ${invalid[0]:04X}")
    return window, pointers


def _infer_last_record_end(window: bytes, start_address: int) -> int:
    """Find the last record's scene terminator when no following pointer exists."""
    start = start_address - SCRIPT_WINDOW_START
    terminator = window.find(b"\xF0", start)
    if terminator < 0:
        raise ValueError(f"No $F0 terminator found after ${start_address:04X}")
    return SCRIPT_WINDOW_START + terminator + 1


def read_dialogue_record(
    prg: bytes,
    index: int,
    first_bank: int = 24,
    table_address: int = DEFAULT_TABLE_ADDRESS,
    count: int = DEFAULT_RECORD_COUNT,
) -> DialogueRecord:
    window, pointers = read_pointer_table(prg, first_bank, table_address, count)
    if not 0 <= index < len(pointers):
        raise IndexError(f"Dialogue index {index} is outside 0-{len(pointers) - 1}")

    address = pointers[index]
    aliases = tuple(i for i, pointer in enumerate(pointers) if pointer == address)
    later = [pointer for pointer in pointers if pointer > address]
    end_inferred = not later
    end_address = min(later) if later else _infer_last_record_end(window, address)
    start = address - SCRIPT_WINDOW_START
    end = end_address - SCRIPT_WINDOW_START
    return DialogueRecord(
        index=index,
        address=address,
        end_address=end_address,
        data=window[start:end],
        aliases=aliases,
        end_inferred=end_inferred,
    )


def text_payload(record: bytes) -> bytes:
    """Return the inline text portion of a common dialogue record.

    $F2 may precede the timing byte. Other scene commands can carry arguments,
    so records beginning with them are left empty until their handlers are fully
    specified.
    """
    cursor = 0
    while cursor < len(record) and record[cursor] == 0xF2:
        cursor += 1
    if cursor + 2 > len(record) or record[cursor] >= 0xF0:
        return b""
    cursor += 2  # timing, then window/layout
    payload = record[cursor:]
    if payload.endswith(b"\xF0"):
        payload = payload[:-1]
    return payload

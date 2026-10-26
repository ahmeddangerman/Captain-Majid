#!/usr/bin/env python3
"""Inspect the fixed-bank vectors and reset path of an MMC3 NES ROM."""

from __future__ import annotations

import argparse
from pathlib import Path

from py65.devices.mpu6502 import MPU
from py65.disassembler import Disassembler

INES_HEADER_SIZE = 16
PRG_UNIT = 16 * 1024


def load_fixed_banks(rom_path: Path) -> tuple[MPU, dict[str, int]]:
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
    prg = rom[INES_HEADER_SIZE : INES_HEADER_SIZE + prg_size]
    if len(prg) != prg_size or prg_size < PRG_UNIT:
        raise ValueError("ROM has an invalid or truncated PRG region")

    mpu = MPU()
    mpu.memory[0xC000:0xE000] = prg[-0x4000:-0x2000]
    mpu.memory[0xE000:0x10000] = prg[-0x2000:]
    vectors = {
        "nmi": mpu.memory[0xFFFA] | mpu.memory[0xFFFB] << 8,
        "reset": mpu.memory[0xFFFC] | mpu.memory[0xFFFD] << 8,
        "irq": mpu.memory[0xFFFE] | mpu.memory[0xFFFF] << 8,
    }
    return mpu, vectors


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("rom", type=Path)
    parser.add_argument("--instructions", type=int, default=12)
    parser.add_argument(
        "--follow-jumps",
        action="store_true",
        help="follow absolute JMP instructions within the fixed banks",
    )
    args = parser.parse_args()
    if args.instructions < 1:
        parser.error("--instructions must be at least 1")

    mpu, vectors = load_fixed_banks(args.rom)
    print(
        f"NMI=${vectors['nmi']:04X} "
        f"RESET=${vectors['reset']:04X} IRQ=${vectors['irq']:04X}"
    )
    disassembler = Disassembler(mpu)
    pc = vectors["reset"]
    for _ in range(args.instructions):
        size, text = disassembler.instruction_at(pc)
        raw = " ".join(f"{mpu.memory[pc + i]:02X}" for i in range(size))
        print(f"{pc:04X}  {raw:<8} {text}")
        if args.follow_jumps and mpu.memory[pc] == 0x4C:
            pc = mpu.memory[pc + 1] | mpu.memory[pc + 2] << 8
        else:
            pc = (pc + size) & 0xFFFF


if __name__ == "__main__":
    main()

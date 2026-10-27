#!/usr/bin/env python3
"""Compare result-three possession handoffs with the private Japanese ROM."""
from __future__ import annotations

import argparse
import hashlib
import itertools
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256
from verify_shot_state import MirroredMemory


def original(prg: bytes, side: int, owner: int, defender: int, action: int):
    cpu = MPU(memory=MirroredMemory())
    ram = cpu.memory
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        ram[address:address + 8192] = prg[bank * 8192:(bank + 1) * 8192]
    ram[0x5FB], ram[0x441], ram[0x442], ram[0x43D] = side, owner, defender, action
    cpu.pc, cpu.sp, cpu.p = 0x844B, 0xFD, cpu.UNUSED
    terminals = {0xC60F: 3, 0xC630: 4, 0xC633: 2}
    for _ in range(256):
        if cpu.pc in terminals:
            return (terminals[cpu.pc], ram[0x5FB], ram[0x441])
        if cpu.pc == 0x8BC8:  # Experience is outside the possession handoff.
            cpu.pc = cpu.stPopWord() + 1
        elif cpu.pc == 0xD093:  # Sound selection is outside this state check.
            cpu.pc = cpu.stPopWord() + 1
        else:
            cpu.step()
    raise RuntimeError(f"Defender result-three handler did not reach a handoff: ${cpu.pc:04X}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")
    prg = rom[16:16 + 262144]
    cases = list(itertools.product((0, 11), (0, 7, 255), (0, 1, 10, 11, 12, 255), (0, 1, 2, 3)))
    rows = [(3, side, owner, defender, action) for side, owner, defender, action in cases]
    native = subprocess.run([str(args.binary.resolve()), "--defender-outcome-batch"],
                            input="".join(" ".join(map(str, row)) + "\n" for row in rows),
                            text=True, capture_output=True, check=True, timeout=15)
    actual = [tuple(map(int, line.split())) for line in native.stdout.splitlines()]
    expected = [original(prg, *case) for case in cases]
    if actual != expected:
        for index, (got, want) in enumerate(zip(actual, expected)):
            if got != want:
                raise RuntimeError(f"Case {index} {cases[index]}: native={got}, ROM={want}")
        raise RuntimeError(f"Result count differs: native={len(actual)}, ROM={len(expected)}")
    print(f"Defender result-three possession handoff matched {len(cases)} original-ROM cases")


if __name__ == "__main__":
    main()

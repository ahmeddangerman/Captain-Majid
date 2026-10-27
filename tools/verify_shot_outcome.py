#!/usr/bin/env python3
"""Compare goalkeeper handoff writes with isolated instructions in a private ROM."""
from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256
from verify_shot_state import MirroredMemory


def original(prg, fields):
    cpu = MPU(memory=MirroredMemory())
    ram = cpu.memory
    ram[0x8000:0xA000] = prg[26 * 8192:27 * 8192]
    ram[0xA000:0xC000] = prg[27 * 8192:28 * 8192]
    ram[0xC000:0xE000] = prg[30 * 8192:31 * 8192]
    ram[0xE000:0x10000] = prg[31 * 8192:32 * 8192]
    (classification, side, opponent, story, subtype, defense, first, second,
     selector, timer, penalty, duration, stun, left, right, owner, ball_x,
     ball_y, defenders, active, kickoff, *intervals) = fields
    for address, value in ((0x5FB, side), (0x2B, opponent), (0x446, story),
                           (0x43C, subtype), (0x43D, defense), (0xE2, first),
                           (0xE3, second), (0x443, selector), (0x28, left),
                           (0x29, right), (0x441, owner), (0x635, ball_x),
                           (0x637, ball_y), (0x600, defenders), (0x5FF, active),
                           (0x629, kickoff), (0x44B, 0)):
        ram[address] = value
    defending_record = 0x384 if side == 0 else 0x300
    scoring_record = 0x300 if side == 0 else 0x384
    for record, values in ((defending_record, (timer, duration, penalty, stun)),
                           (scoring_record, (91, 0, 201, 179))):
        for offset, value in zip((5, 6, 7, 10), values):
            ram[record + offset] = value
    ram[0x86B1:0x86B9] = intervals  # ROM table is explicit native input.
    cpu.y, cpu.pc, cpu.sp, cpu.p = classification, 0x856B, 0xFD, cpu.UNUSED
    for _ in range(100):
        if cpu.pc == 0x8599:
            break
        cpu.step()
    else:
        raise RuntimeError("Story decision did not reach handler dispatch")
    code = ram[0x612]
    if code == 0 or (code == 1 and (subtype & 63) < 3):
        entry, terminal, event = 0x85AC, 0x85DD, 0
    elif code == 1:
        entry, terminal, event = 0x8605, 0x8616, 1
    elif code == 2:
        entry, terminal, event = 0x861C, 0x8640, 2
    elif defense == 1:
        entry, terminal, event = 0x8646, 0x86AE, 4
    else:
        entry, terminal, event = 0x8646, 0x865B, 3
    cpu.pc, cpu.sp, cpu.p = entry, 0xFD, cpu.UNUSED
    # Calls related to scenarios, sound, and experience are outside the
    # immediate match-state handoff. The original arithmetic and pointer
    # helpers execute unchanged.
    ignored_calls = {0x8BD4, 0x8BC8, 0xC54E, 0x987B, 0xC52A, 0xC56F}
    for _ in range(2000):
        if cpu.pc == terminal:
            break
        if cpu.pc in ignored_calls:
            cpu.pc = (cpu.stPopWord() + 1) & 65535
        else:
            cpu.step()
    else:
        raise RuntimeError(f"Handler did not reach ${terminal:04X}: ${cpu.pc:04X}")
    return (event, code, ram[0x5FB], ram[0x441], ram[0x28], ram[0x29],
            ram[defending_record + 5], ram[defending_record + 7],
            ram[defending_record + 6], ram[defending_record + 10],
            ram[0x635], ram[0x637], ram[0x600], ram[0x5FF], ram[0x629])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")
    prg = rom[16:16 + 262144]
    intervals = tuple(prg[26 * 8192 + 0x6B1:26 * 8192 + 0x6B9])
    rng = random.Random(0xC0A1)
    cases = []
    for code in range(4):
        for side in (0, 11):
            for subtype in (0, 2, 3, 131):
                for defense in (0, 1, 2):
                    for _ in range(4):
                        cases.append((code, side, rng.choice((4, 5, 6)), rng.choice((0, 1, 3, 4)),
                                      subtype, defense, rng.randrange(256), rng.randrange(256),
                                      rng.randrange(256), rng.randrange(256), rng.randrange(80),
                                      rng.randrange(256), rng.randrange(256), rng.randrange(256),
                                      rng.randrange(256), rng.randrange(256), rng.randrange(256),
                                      rng.randrange(256), rng.randrange(256), rng.randrange(256),
                                      rng.randrange(256), *intervals))
    expected = [original(prg, case) for case in cases]
    process = subprocess.run([str(args.binary.resolve()), "--outcome-batch"],
                             input="".join(" ".join(map(str, case)) + "\n" for case in cases),
                             capture_output=True, text=True, check=True, timeout=20)
    actual = [tuple(map(int, line.split())) for line in process.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f"Native returned {len(actual)} results for {len(expected)} cases")
    for index, (reference, native) in enumerate(zip(expected, actual)):
        if reference != native:
            raise RuntimeError(f"Case {index}: {cases[index]}\nROM={reference}\nNative={native}")
    print(f"Goalkeeper outcome handoff matched {len(cases)} original-ROM cases")


if __name__ == "__main__":
    main()

#!/usr/bin/env python3
"""Compare isolated skill, action-power and keeper routines with native code."""
from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256


def cpu_for(prg: bytes, first_bank: int) -> MPU:
    cpu = MPU()
    for bank, address in ((first_bank, 0x8000), (first_bank + 1, 0xA000), (30, 0xC000), (31, 0xE000)):
        cpu.memory[address : address + 8192] = prg[bank * 8192 : (bank + 1) * 8192]
    return cpu


def enter(cpu: MPU, address: int, stack_argument: int | None = None):
    cpu.pc, cpu.sp = address, 0xFD
    cpu.memory[0x1FE], cpu.memory[0x1FF] = 0xFF, 0x01
    if stack_argument is not None:
        cpu.stPush(stack_argument)


def finish(cpu: MPU, stop: int = 0x0200, limit: int = 3000):
    for _ in range(limit):
        if cpu.pc == stop:
            return
        cpu.step()
    raise RuntimeError(f"Original routine did not reach ${stop:04X}: ${cpu.pc:04X}")


def compare(binary: Path, mode: str, inputs, expected):
    result = subprocess.run([str(binary.resolve()), mode],
        input="".join(" ".join(map(str, fields)) + "\n" for fields in inputs),
        text=True, capture_output=True, check=True, timeout=30)
    actual = [tuple(map(int, line.split())) for line in result.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f"{mode}: result count {len(actual)} != {len(expected)}")
    for i, (reference, native) in enumerate(zip(expected, actual)):
        if reference != native:
            raise RuntimeError(f"{mode} case {i}: {inputs[i]}\nROM={reference}\nNative={native}")
    print(f"{mode}: {len(inputs)} original-ROM cases matched", flush=True)


def verify_skills(prg, binary, rng):
    cpu = cpu_for(prg, 28)
    cases = [(base, level, base ^ level) for base in range(256) for level in range(256)]
    cases += [(rng.randrange(256), rng.randrange(256), rng.randrange(256)) for _ in range(4096)]
    expected = []
    for base, level, modifier in cases:
        cpu.memory[0x32], cpu.memory[0x33] = 0x00, 0x02  # source byte at RAM $0200
        cpu.memory[0x34], cpu.memory[0x35] = 0x00, 0x03  # player record at RAM $0300
        cpu.memory[0x200], cpu.memory[0x303] = base, level
        cpu.p, cpu.x, cpu.y = cpu.UNUSED, 1, 0  # non-stamina standard skill
        enter(cpu, 0x80F9)
        finish(cpu)
        index = cpu.y
        cpu.x = modifier
        enter(cpu, 0x8278, stack_argument=index)
        finish(cpu)
        expected.append((cpu.x,))  # original lookup uses the final index in X
    compare(binary, "--skill-index-batch", cases, expected)


def verify_action(prg, binary, rng):
    cpu = cpu_for(prg, 26)
    cases = [(base, first, second, weak, subtype, carry)
             for base in (0, 1, 255, 256, 1024, 32768, 65535)
             for first in (0, 7, 8, 127, 128, 255)
             for second in (0, 1, 127, 128, 255)
             for weak in (0, 1) for subtype in (0, 3, 128, 255) for carry in (0, 1)]
    cases += [(rng.randrange(65536), rng.randrange(256), rng.randrange(256),
               rng.randrange(2), rng.randrange(256), rng.randrange(2)) for _ in range(4096)]
    expected = []
    for base, first, second, weak, subtype, carry in cases:
        cpu.memory[0x32], cpu.memory[0x33] = base & 255, base >> 8
        cpu.memory[0xE2], cpu.memory[0xE3] = first, second
        cpu.memory[0x3A], cpu.memory[0x43C] = weak << 7, subtype
        cpu.p = cpu.UNUSED | carry
        enter(cpu, 0x8F9A)
        finish(cpu, stop=0x8FE5)
        multiplier = cpu.memory[0x67] | cpu.memory[0x68] << 8
        finish(cpu)
        expected.append((cpu.memory[0x32] | cpu.memory[0x33] << 8, multiplier,
                         cpu.memory[0x61C] | cpu.memory[0x61D] << 8, cpu.memory[0x43C]))
    compare(binary, "--action-power-batch", cases, expected)


def verify_keeper(prg, binary, rng):
    cpu = cpu_for(prg, 26)
    width, offset = cpu.memory[0x8DE9 : 0x8DEB]
    thresholds = cpu.memory[0x86B9 : 0x86BD]
    # (incoming, base, random1, random2, weak, critical, context, slot, subtype, timer, elapsed, penalty, side)
    cases = [(incoming, base, first, second, weak, critical, 4, slot, 3, timer, elapsed, penalty, side)
             for incoming in (0, 1, 100, 341, 342, 343, 1023, 1024, 65535)
             for base in (0, 1, 256, 65535)
             for first, second, weak, critical in ((100, 100, 0, 100), (101, 100, 0, 100),
                                                   (255, 255, 1, 0), (0, 0, 0, 0))
             for timer, elapsed, penalty in ((0, 0, 0), (10, 3, 4), (2, 3, 255), (255, 0, 255))
             for slot, side in ((0, 0), (11, 11))]
    cases += [(rng.randrange(65536), rng.randrange(65536), rng.randrange(256), rng.randrange(256),
               rng.randrange(2), rng.choice((0, 100, 203, 243, 255)), rng.choice((0, 4)),
               rng.choice((0, 1, 11)), rng.randrange(256), rng.randrange(256), rng.randrange(256),
               rng.randrange(256), rng.choice((0, 11))) for _ in range(4096)]
    inputs, expected = [], []
    for incoming, base, first, second, weak, critical, context, slot, subtype, timer, elapsed, penalty, side in cases:
        inputs.append((incoming, base, first, second, width, offset, weak, critical,
                       context, slot, subtype, timer, elapsed, penalty, *thresholds))
        cpu.memory[0x32], cpu.memory[0x33] = base & 255, base >> 8
        cpu.memory[0x61C], cpu.memory[0x61D] = incoming & 255, incoming >> 8
        for address, value in ((0xE2, first), (0xE3, second), (0x3A, weak << 7),
                               (0x621, context), (0x442, slot), (0x43E, subtype),
                               (0x5FB, side), (0x62B, elapsed)):
            cpu.memory[address] = value
        other_slot = side ^ 11
        player = cpu.memory[0xCD89 + other_slot * 2] | cpu.memory[0xCD8A + other_slot * 2] << 8
        cpu.memory[player + 5], cpu.memory[player + 7] = timer, penalty
        cpu.p, cpu.x = cpu.UNUSED, critical
        enter(cpu, 0x854A)
        special = False
        adjustment = None
        for _ in range(3000):
            if cpu.pc == 0x856B:
                break
            special = special or cpu.pc == 0x8D3A
            if cpu.pc == 0x8F4F:
                adjustment = cpu.a
            cpu.step()
        else:
            raise RuntimeError(f"Keeper comparison did not finish: ${cpu.pc:04X}")
        if adjustment is None:
            raise RuntimeError("Keeper state-adjustment checkpoint was not reached")
        expected.append((cpu.memory[0x32] | cpu.memory[0x33] << 8, cpu.memory[0x71],
                         cpu.memory[0x3A], adjustment, cpu.a, cpu.y, cpu.memory[0x43E], int(special)))
    compare(binary, "--keeper-batch", inputs, expected)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")
    prg = rom[16 : 16 + 262144]
    rng = random.Random(0x53484F54)
    verify_skills(prg, args.binary, rng)
    verify_action(prg, args.binary, rng)
    verify_keeper(prg, args.binary, rng)


if __name__ == "__main__":
    main()

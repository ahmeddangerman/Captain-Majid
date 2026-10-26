#!/usr/bin/env python3
"""Check native defender calculations against isolated Japanese ROM execution."""
from __future__ import annotations

import argparse
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU

from verify_action_strength import PAYLOAD_SHA256
import hashlib


def run(cpu: MPU, entry: int, limit: int = 3000) -> bool:
    cpu.pc = entry
    cpu.sp = 0xFD
    cpu.memory[0x1FE] = 0xFF
    cpu.memory[0x1FF] = 0x01  # RTS returns to $0200
    special = False
    for _ in range(limit):
        if cpu.pc == 0x0200:
            return special
        special = special or cpu.pc == 0x8D3A
        cpu.step()
    raise RuntimeError(f"Original routine did not return: ${cpu.pc:04X}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")
    prg = rom[16 : 16 + 262144]
    cpu = MPU()
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        cpu.memory[address : address + 8192] = prg[bank * 8192 : (bank + 1) * 8192]

    # Read existing pairs. The table-data region ends before executable code $8E33.
    pointers = [cpu.memory[0x8D93 + i * 2] | cpu.memory[0x8D94 + i * 2] << 8 for i in range(11)]
    bounds = sorted(set(pointers) | {0x8E33})
    pairs = []
    for selector, pointer in enumerate(pointers):
        end = min(address for address in bounds if address > pointer)
        for offset in range(0, end - pointer, 2):
            width, bias = cpu.memory[pointer + offset : pointer + offset + 2]
            if width:
                pairs.append((selector, offset, width, bias))

    rng = random.Random(0x444546)
    # Deliberately sweep branch conditions and random extremes for each existing pair.
    cases = []
    for pair in pairs:
        for base in (0, 1, 255, 256, 1023, 1024, 32767, 65535):
            for first, second in ((0, 0), (100, 100), (247, 247), (255, 248), (255, 255)):
                for weak, eligible, context, defender in ((0, 0, 0, 1), (1, 1, 4, 0),
                                                          (0, 1, 4, 0), (0, 1, 4, 11),
                                                          (0, 1, 0, 1)):
                    cases.append((pair, base, 300, first, second, weak, eligible,
                                  context, defender, 3, 0))
        # Independently check numerator wrap in the original divider, not just in native assertions.
        for incoming in (0, 1, 1023, 1024, 1025, 65535):
            for base in (256, 65535):
                for first, second in ((0, 0), (100, 100), (255, 255)):
                    cases.append((pair, base, incoming, first, second, 0, 0, 0, 1, 3, 0))
    for _ in range(4096):
        cases.append((rng.choice(pairs), rng.randrange(65536), rng.randrange(65536),
                      rng.randrange(256), rng.randrange(256), rng.randrange(2), rng.randrange(2),
                      rng.choice((0, 4)), rng.choice((0, 1, 11)), rng.randrange(256), rng.randrange(3)))

    native_inputs, expected = [], []
    counts = {"special": 0, "weakened": 0, "zero_strength": 0, "loss_branch": 0}
    for pair, base, incoming, first, second, weak, eligible, context, defender, subtype, action in cases:
        selector, offset, width, bias = pair
        thresholds = cpu.memory[0x828C + action * 4 : 0x8290 + action * 4]
        fields = (incoming, base, first, second, width, bias, weak, eligible, context,
                  defender, subtype, *thresholds)
        native_inputs.append(" ".join(map(str, fields)))
        cpu.a, cpu.x, cpu.y = selector, 0, 0
        cpu.p = cpu.UNUSED | (cpu.NEGATIVE if eligible else 0)
        for address, value in ((0x32, base & 255), (0x33, base >> 8),
                               (0x61C, incoming & 255), (0x61D, incoming >> 8),
                               (0xE2, first), (0xE3, second), (0x3A, weak << 7),
                               (0x3B, offset), (0x621, context), (0x442, defender),
                               (0x43E, subtype), (0x43D, action)):
            cpu.memory[address] = value
        special = run(cpu, 0x8EE9)
        adjusted = cpu.memory[0x32] | cpu.memory[0x33] << 8
        strength, loss, score = cpu.memory[0x71], cpu.memory[0x619], cpu.a
        run(cpu, 0x8132)
        result_code = cpu.memory[0x612]
        if result_code == 1:
            cpu.pc = 0x819F
            for _ in range(32):
                if cpu.pc == 0x81B8:
                    break
                cpu.step()
            else:
                raise RuntimeError("Contact-loss stage did not reach exit")
        remaining = cpu.memory[0x61C] | cpu.memory[0x61D] << 8
        expected.append((adjusted, strength, loss, score, result_code, remaining,
                         cpu.memory[0x43E], int(special)))
        counts["special"] += special
        counts["weakened"] += weak
        counts["zero_strength"] += strength == 0
        counts["loss_branch"] += result_code == 1

    result = subprocess.run([str(args.binary.resolve()), "--defender-batch"],
                            input="\n".join(native_inputs) + "\n", text=True,
                            capture_output=True, check=True, timeout=30)
    actual = [tuple(map(int, line.split())) for line in result.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f"Native result count {len(actual)} != {len(expected)}")
    for i, (reference, native) in enumerate(zip(expected, actual)):
        if reference != native:
            raise RuntimeError(f"Case {i}: {native_inputs[i]}\nROM={reference}\nNative={native}")
    print(f"Defender ROM comparison passed: {len(cases)} cases, all 8 outputs matched")
    print(f"Coverage: {counts}; {len(pairs)} nonzero-range pairs read from the user's ROM")


if __name__ == "__main__":
    main()

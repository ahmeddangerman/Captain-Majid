#!/usr/bin/env python3
"""Compare the original control-travel fragment using a supplied loaded speed.

The host substitutes the supplied speed at $E3D6, just before the original
speed loader; it does not execute that loader, frame scheduling or audio.
"""
import argparse
import hashlib
import itertools
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from reference_identity import PAYLOAD_SHA256


def cases(exhaustive):
    speeds = range(65536) if exhaustive else (0, 1, 3, 4, 7, 928, 1023, 32768, 65535)
    for speed, facing in itertools.product(speeds, (0, 64)):
        yield ((speed * 73 + 0xdead) & 65535, speed, 0, 1, facing)
    for side, held, facing, speed, travel in itertools.product(
            (0, 11), (0, 1, 2, 4, 8, 15, 128, 255), (0, 1, 64, 255),
            (0, 1, 3, 4, 928, 65535), (0, 1, 32768, 65535)):
        yield travel, speed, side, held, facing


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--binary', required=True, type=Path)
    parser.add_argument('--exhaustive', action='store_true', help='test every 16-bit loaded speed')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b'NES\x1a' or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('ROM payload does not match the verified Japanese reference')
    memory = ReferenceCartridge(rom[16:262160])
    cpu = MPU(memory=memory)
    inputs, expected = [], []
    for travel, speed, side, held, facing in cases(args.exhaustive):
        memory[0x642], memory[0x643] = travel & 255, travel >> 8
        memory[0x32], memory[0x33] = speed & 255, speed >> 8
        memory[0x5fb], memory[0x1c], memory[0x517] = side, held, facing
        memory[0x1fe], memory[0x1ff] = 255, 1
        cpu.pc, cpu.sp, cpu.p = 0xe3ca, 0xfd, cpu.UNUSED
        for _ in range(80):
            if cpu.pc == 0x200:
                break
            if cpu.pc == 0xe3d6:
                cpu.pc = 0xe3de  # Explicitly supplied loader result in $32/$33.
            cpu.step()
        else:
            raise RuntimeError('Original control-travel fragment exceeded its instruction bound')
        inputs.append((travel, speed, side, held, facing))
        expected.append(memory[0x642] | memory[0x643] << 8)
    text = ''.join(' '.join(map(str, row)) + '\n' for row in inputs)
    run = subprocess.run([str(args.binary.resolve()), '--batch'], input=text, text=True,
                         capture_output=True, check=True, timeout=120)
    actual = [int(line) for line in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f'Native result count {len(actual)} != original {len(expected)}')
    for index, (got, want) in enumerate(zip(actual, expected)):
        if got != want:
            raise RuntimeError(f'Case {index}, inputs {inputs[index]}: native {got}, original {want}')
    print(f'Matched {len(expected)} original control-travel fixtures with explicitly supplied speed.')


if __name__ == '__main__':
    main()

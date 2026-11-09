#!/usr/bin/env python3
"""Compare native kickoff formation coordinates with two private RAM captures.

The caller must identify the before/after placement frames; this checks only
the 44 represented coordinate words and does not establish instruction timing.
"""
import argparse
import subprocess
import sys
from pathlib import Path


def read_ram(path):
    data = path.read_bytes()
    if len(data) != 2048:
        raise ValueError(f'{path}: RAM capture must contain exactly 2048 bytes')
    return data


def read_formation(path):
    parts = path.read_text(encoding='utf-8').split()
    if len(parts) != 89 or parts[0] != 'CMJKICKFORMATION1':
        raise ValueError('Formation table must have CMJKICKFORMATION1 and exactly 88 cells')
    try:
        cells = [int(part, 10) for part in parts[1:]]
    except ValueError as error:
        raise ValueError('Formation cells must be decimal integers') from error
    if any(not 0 <= cell < 240 for cell in cells):
        raise ValueError('Formation cells must be in the range 0..239')
    return cells


def coordinates(ram):
    result = []
    for slot in range(22):
        base = 0x300 + 12 * slot
        result.extend((int.from_bytes(ram[base + 5:base + 7], 'little'),
                       int.from_bytes(ram[base + 7:base + 9], 'little')))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('before', type=Path)
    parser.add_argument('after', type=Path)
    parser.add_argument('formation', type=Path)
    parser.add_argument('--side', type=int, choices=(0, 11), required=True)
    parser.add_argument('--binary', type=Path, required=True,
                        help='Path to the built kickoff-formation-tests batch driver')
    args = parser.parse_args()
    try:
        before, after = read_ram(args.before), read_ram(args.after)
        table = read_formation(args.formation)
        if after[0x5fb] != args.side:
            raise ValueError('After-capture possession side does not match --side')
        formations = (before[0x2c], before[0x2e])
        if any(value >= 4 for value in formations):
            raise ValueError('Before-capture formations must be in the range 0..3')
        if formations != (after[0x2c], after[0x2e]):
            raise ValueError('Formations changed between captures')
        values = [args.side, *formations, *table, *coordinates(before)]
        result = subprocess.run([str(args.binary.resolve()), '--batch'],
                                input=' '.join(map(str, values)) + '\n', text=True,
                                capture_output=True, check=True, timeout=30)
        native = [int(value, 10) for value in result.stdout.split()]
        if len(native) != 44 or any(not 0 <= value <= 65535 for value in native):
            raise ValueError('Native batch driver did not return exactly 44 coordinate words')
        original = coordinates(after)
        for index, (actual, expected) in enumerate(zip(native, original)):
            if actual != expected:
                raise ValueError(f'formation boundary, player[{index // 2}].{"x" if index % 2 == 0 else "y"}: '
                                 f'native {actual}, original {expected}')
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        parser.exit(1, f'Formation comparison stopped: {error}\n')
    print('Matched 44 coordinate words at the supplied formation boundary; other fields were not compared.')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare ordinary pass target candidate scans against original D8F7..D949.

Uses two private original target RAM seeds. Constructed positions and carriers
are supplied; this checks the scan before original presentation and chooser
logic, not the target confirmation or a live match.
"""
import argparse
import hashlib
import random
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from export_shot_content import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--target-seed', type=Path, action='append', required=True)
    parser.add_argument('--cxx', default='clang++')
    args = parser.parse_args()
    if len(args.target_seed) < 2:
        raise ValueError('At least two independent target RAM seeds are required')
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        raise ValueError('ROM does not match the verified Japanese reference')
    seeds = [path.read_bytes() for path in args.target_seed]
    if any(len(seed) != 2048 for seed in seeds):
        raise ValueError('Target RAM seeds must be exactly 2048 bytes')
    generator = random.Random(0xd8f7)
    cases, expected = [], []
    for seed in seeds:
        memory = ObservedCartridge(rom[16:262160], seed, None, set())
        memory.registers[6:8] = [26, 27]
        cpu = MPU(memory=memory)
        for case in range(1024):
            cursor = (case * 37) % 240
            carrier = case % 10 + 1
            x = 48 + (cursor // 12) * 8 + 4
            y = 80 + (cursor % 12) * 8 + 4
            # Include empty, all-player, single-player and random occupancy patterns.
            mask = (0 if case == 0 else (1 << 22) - 1 if case == 1 else
                    1 << (case % 22) if case < 24 else generator.getrandbits(22))
            positions = [(x, y) if mask & (1 << player) else (0, 0) for player in range(22)]
            memory[0x441], memory[0x624] = carrier, cursor
            for player, (px, py) in enumerate(positions):
                record = 0x300 + 12 * player
                memory[record + 6], memory[record + 8] = px, py
            for slot in range(4):
                memory[0x431 + slot] = 240 + slot
            cpu.pc, cpu.sp, cpu.p = 0xd8f7, 0xfd, cpu.UNUSED
            for _ in range(10000):
                if cpu.pc == 0xd949:
                    break
                if cpu.pc < 0x8000:
                    raise RuntimeError('Original candidate scan left PRG')
                cpu.step()
            else:
                raise RuntimeError(f'Original candidate scan exceeded instruction bound in case {len(cases) + 1}')
            count, allies = memory[0x430], memory[0x625]
            if count > 4 or allies > count:
                raise ValueError(f'Original produced invalid candidate counts {count}/{allies}')
            expected.append((count, allies, *(memory[0x431 + i] if i < count else 0 for i in range(4))))
            cases.append((carrier, cursor, positions))
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-pass-candidates-') as temporary:
        binary = Path(temporary) / 'compare'
        subprocess.run([args.cxx, '-std=c++17', '-O2', '-I' + str(root / 'engine/include'),
                        str(root / 'engine/tests/pass_candidates_trace_driver.cpp'),
                        str(root / 'engine/src/pass_target.cpp'), str(root / 'engine/src/defender_selection.cpp'),
                        '-o', str(binary)], check=True, timeout=60)
        data = ''.join(f'{carrier} {cursor} ' + ' '.join(f'{x} {y}' for x, y in positions) + '\n'
                       for carrier, cursor, positions in cases)
        run = subprocess.run([str(binary)], input=data, capture_output=True, text=True, check=True, timeout=60)
    actual = [tuple(map(int, row.split())) for row in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise ValueError(f'Native result count {len(actual)} differs from original {len(expected)}')
    names = ('count', 'allied_count', 'ids[0]', 'ids[1]', 'ids[2]', 'ids[3]')
    for index, (wanted, got) in enumerate(zip(expected, actual)):
        for name, expected_value, actual_value in zip(names, wanted, got):
            if actual_value != expected_value:
                raise ValueError(f'case {index + 1}, boundary target.candidates, {name}: native {actual_value}, original {expected_value}')
    print(f'Ordinary pass candidate scan matched {len(cases)} controlled original CPU cases across {len(seeds)} target seeds.')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare standard field defender preparation with the complete ROM $8224 loader."""
from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256


def cpu_for(prg):
    cpu = MPU()
    for bank, address in ((28, 0x8000), (29, 0xA000), (30, 0xC000), (31, 0xE000)):
        cpu.memory[address:address + 8192] = prg[bank * 8192:(bank + 1) * 8192]
    return cpu


def word(memory, address):
    return memory[address] | memory[address + 1] << 8


def original(cpu, case):
    player_id, player_type, level, action, situation, subtype = case
    ram = cpu.memory
    record = 0x0300 + player_id * 12
    ram[record:record + 12] = [0] * 12
    ram[record], ram[record + 3] = player_type, level
    ram[0x0442], ram[0x043D] = player_id, action
    ram[0x044E], ram[0x043E] = situation, subtype
    # Poison prior outputs so a skipped write cannot silently pass.
    ram[0x0445], ram[0x043F], ram[0x0440] = 253, 254, 255
    cpu.pc, cpu.sp, cpu.p = 0x8224, 0xFD, cpu.UNUSED
    cpu.a = cpu.x = cpu.y = 0
    ram[0x01FE], ram[0x01FF] = 0xFF, 0x01
    selector = index = None
    for _ in range(1000):
        if cpu.pc == 0x8232:
            selector = cpu.x
        if cpu.pc == 0x8284:
            index = cpu.a
        if cpu.pc == 0x0200:
            if selector is None or index is None or word(ram, 0x34) != record:
                raise RuntimeError('ROM missed its standard skill path or selected player pointer')
            return (selector, index, word(ram, 0x32), ram[0x0445], word(ram, 0x043F))
        cpu.step()
    raise RuntimeError(f'ROM preparation did not return: ${cpu.pc:04X}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--binary', type=Path, default=Path('build/captain-prototype'))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if (rom[:4] != b'NES\x1a' or len(rom) != 393232 or
            hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256):
        parser.error('ROM payload does not match the verified Japanese reference')
    cpu = cpu_for(rom[16:16 + 262144])
    ram = cpu.memory
    selectors = ram[0x824C:0x825B]
    pointers = [word(ram, 0x9554 + 2 * action) for action in range(5)] + [0x959E]
    moves = [ram[start:end] for start, end in zip(pointers, pointers[1:])]
    if any(len(data) % 4 for data in moves):
        raise RuntimeError('Move table bounds are not aligned')
    counts = [len(data) // 4 for data in moves]
    content = list(selectors)
    for count, data in zip(counts, moves):
        content.extend((count, *data))
    content.extend(ram[0x9E4E:0x9F0E])
    profiles = {}
    for player_type in range(1, 35):
        profile = ram[0x95D6 + 4 * player_type]
        start = 0x9FCE + 24 * profile
        profiles[player_type] = list(ram[start:start + 24]) + [0] * 7
    options = [(action, situation, subtype) for action in range(5)
               for situation in range(3) if selectors[3 * action + situation] != 255
               for subtype in range(counts[action])]
    field_ids = (*range(1, 11), *range(12, 22))
    cases = []
    for player_type in profiles:
        for level in (0, 127, 128, 255):
            for action, situation, subtype in options:
                for high_bits in (0, 0xC0):
                    cases.append((field_ids[len(cases) % 20], player_type, level,
                                  action, situation, subtype | high_bits))
    rng = random.Random(0x8224)
    for _ in range(4096):
        action, situation, subtype = rng.choice(options)
        cases.append((rng.choice(field_ids), rng.randrange(1, 35), rng.randrange(256),
                      action, situation, subtype | rng.choice((0, 0x40, 0x80, 0xC0))))
    lines, expected = [], []
    for case in cases:
        player_id, player_type, level, action, situation, subtype = case
        fields = [action, situation, subtype, level, *profiles[player_type], *content]
        lines.append(' '.join(map(str, fields)))
        expected.append(original(cpu, case))
    run = subprocess.run([str(args.binary.resolve()), '--field-defender-preparation-batch'],
                         input='\n'.join(lines) + '\n', text=True, capture_output=True,
                         check=True, timeout=60)
    actual = [tuple(map(int, line.split())) for line in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f'Native output count {len(actual)} != {len(expected)}')
    for index, (want, got) in enumerate(zip(expected, actual)):
        if want != got:
            raise AssertionError(f'Case {index} {cases[index]}: ROM={want}, native={got}')
    print(f'Field defender preparation matched {len(cases)} original-ROM cases: '
          'selected skill/index, strength, effect, cost, and original player pointer.')


if __name__ == '__main__':
    main()

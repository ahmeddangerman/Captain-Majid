#!/usr/bin/env python3
"""Compare ordinary-pass target initialization with private original evidence.

Two private original menu RAM seeds and a verified Japanese ROM are required.
Constructed carrier positions and low bytes are supplied; cooperative host
acknowledgements replace display/frame waits. This is not a live match.
"""
import argparse
import hashlib
import itertools
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_action_menu import trace_menu
from trace_field_script import ObservedCartridge
from export_shot_content import PAYLOAD_SHA256


def original_target(prg, seed, x, y):
    raw = bytearray(seed)
    owner = raw[0x441]
    if owner not in range(1, 11):
        raise ValueError('Menu seed must have a side-zero field carrier')
    record = 0x300 + 12 * owner
    raw[record + 5], raw[record + 6] = x & 255, x >> 8
    raw[record + 7], raw[record + 8] = y & 255, y >> 8
    # Stale values verify the original refreshes the ball from the carrier.
    raw[0x635], raw[0x637], raw[0x5fe] = 52, 84, 0
    _, selected = trace_menu(prg, raw, 'poll', 2)
    memory = ObservedCartridge(prg, selected, None, set())
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1), (6, 0x50),
                           (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26, 27]
    memory[0x1c] = memory[0x1e] = 128
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = 0xd5ea, 0x50, cpu.UNUSED
    for _ in range(2000000):
        if cpu.pc < 0x8000:
            raise RuntimeError('Original pass target trace left PRG')
        bank = memory.bank_at(cpu.pc)
        if bank == 30 and cpu.pc == 0xd80a:
            return tuple(memory[address] for address in (0x635, 0x637, 0x5fe, 0x624, 0x62d, 0x43c))
        if cpu.pc == 0xcaae:
            memory.instruction = None
            memory[0x1b] |= 128
            memory[0x515] = 0
            memory[0x1c] = memory[0x1e] = 128
            if memory[0x498]:
                memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc)
        cpu.step()
    raise RuntimeError('Original pass target trace exceeded instruction bound')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--menu-seed', type=Path, action='append', required=True)
    parser.add_argument('--cxx', default='clang++')
    args = parser.parse_args()
    if len(args.menu_seed) < 2:
        raise ValueError('At least two independently observed menu seeds are required')
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        raise ValueError('ROM does not match the verified Japanese reference')
    seeds = [path.read_bytes() for path in args.menu_seed]
    if any(len(seed) != 2048 for seed in seeds):
        raise ValueError('Menu seed RAM files must be 2048 bytes')
    positions = ((48, 80), (48, 175), (64, 96), (108, 108), (127, 132), (160, 140), (207, 80), (207, 175))
    cases = [(seed, (x << 8) | fraction_x, (y << 8) | fraction_y)
             for seed, (x, y), (fraction_x, fraction_y) in itertools.product(seeds, positions, ((0, 0), (255, 127)))]
    expected = [original_target(rom[16:262160], *case) for case in cases]
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-pass-init-') as temporary:
        binary = Path(temporary) / 'compare'
        subprocess.run([args.cxx, '-std=c++17', '-O2', '-I' + str(root / 'engine/include'),
                        str(root / 'engine/tests/pass_target_init_trace_driver.cpp'),
                        str(root / 'engine/src/pass_target.cpp'), str(root / 'engine/src/defender_selection.cpp'),
                        '-o', str(binary)], check=True, timeout=60)
        data = ''.join(f'{x} {y}\n' for _, x, y in cases)
        run = subprocess.run([str(binary)], input=data, capture_output=True, text=True, check=True, timeout=60)
    actual = [tuple(map(int, row.split())) for row in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f'Native result count {len(actual)} differs from original {len(expected)}')
    names = ('ball.x', 'ball.y', 'ball.cell', 'pass.cursor', 'clock.ball_mode', 'attack.subtype')
    for index, (wanted, got) in enumerate(zip(expected, actual)):
        for name, expected_value, actual_value in zip(names, wanted, got):
            if actual_value != expected_value:
                raise ValueError(f'case {index + 1}, boundary target.initial, {name}: native {actual_value}, original {expected_value}')
    print(f'Ordinary pass target initialization matched {len(cases)} constructed original CPU cases across {len(seeds)} menu seeds.')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare pass cursor arithmetic at original D80F..D832, without UI calls.

Uses private ROM and two independent target RAM seeds. The original direction
arithmetic runs instruction by instruction, stopping before the cursor redraw
or confirmation/cancellation logic; this does not emulate frames or UI.
"""
import argparse
import hashlib
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
    expected = []
    for seed in seeds:
        memory = ObservedCartridge(rom[16:262160], seed, None, set())
        memory.registers[6:8] = [26, 27]
        cpu = MPU(memory=memory)
        for cursor in range(240):
            for held in range(256):
                memory[0x624], memory[0x1c] = cursor, held
                cpu.pc, cpu.p = 0xd80f, cpu.UNUSED
                for _ in range(40):
                    if cpu.pc in (0xd832, 0xd837):
                        break
                    cpu.step()
                else:
                    raise RuntimeError(f'Original cursor arithmetic did not stop: {cursor} {held}')
                expected.append(memory[0x624])
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-pass-cursor-') as temporary:
        binary = Path(temporary) / 'compare'
        subprocess.run([args.cxx, '-std=c++17', '-O2', '-I' + str(root / 'engine/include'),
                        str(root / 'engine/tests/pass_cursor_trace_driver.cpp'),
                        str(root / 'engine/src/pass_target.cpp'), str(root / 'engine/src/defender_selection.cpp'),
                        '-o', str(binary)], check=True, timeout=60)
        data = ''.join(f'{cursor} {held}\n' for _ in seeds for cursor in range(240) for held in range(256))
        run = subprocess.run([str(binary)], input=data, capture_output=True, text=True, check=True, timeout=60)
    actual = [int(row) for row in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise ValueError(f'Native result count {len(actual)} differs from original {len(expected)}')
    for index, (wanted, got) in enumerate(zip(expected, actual)):
        if wanted != got:
            seed, within = divmod(index, 240 * 256)
            cursor, held = divmod(within, 256)
            raise ValueError(f'case seed {seed + 1}, cursor {cursor}, held {held}, boundary cursor.move: native {got}, original {wanted}')
    print(f'Pass cursor arithmetic matched {len(expected)} original CPU cases across {len(seeds)} target seeds, all 240 cells and 256 held bytes.')


if __name__ == '__main__':
    main()

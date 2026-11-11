#!/usr/bin/env python3
"""Compare candidate2's field fallback decision with controlled original CPU runs."""
import argparse
import hashlib
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from reference_cartridge import ReferenceCartridge
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original_choice(prg, raw):
    memory = ObservedCartridge(prg, raw, None, set())
    memory.registers[6:8] = [28, 29]
    memory[0x24], memory[0x25] = 28, 29
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory[0x1fe], memory[0x1ff] = 255, 1
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = 0x875d, 0xfd, cpu.UNUSED
    for _ in range(10000):
        if cpu.pc == 0x200:
            return bytes(memory.ram[:2048])
        if cpu.pc < 0x8000 or cpu.pc in (0xcaae, 0xcb0f):
            raise RuntimeError(f'Unexpected original scheduler or return at {cpu.pc:04x}')
        cpu.step()
    raise RuntimeError('Original field choice instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'ram', 'exe'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--pass-gates', action='store_true', help='Expect pass choices with original zero option pointers')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    base = bytearray(args.ram.read_bytes())
    if len(base) != 2048 or base[0x22] & 64:
        parser.error('Expected compatible original internal RAM')
    stats = ReferenceCartridge(rom[16:262160])
    stats.registers[6:8] = [28, 29]
    word = lambda address: stats[address] | (stats[address + 1] << 8)
    cases, expected = [], []
    for kind in (1, 7, 23, 35, 70, 117):
        for flags, alternate in ((0, 0), (128, 35)):
            for x in (112, 159, 160, 200):
                for first in range(8):
                    for second in (0, 127, 255):
                        for pattern in (0, 1):
                            raw = bytearray(base)
                            actor, y = 19, 124
                            raw[0x441], raw[0x5fb], raw[0x621], raw[0x44e] = actor, 11, 0, 0
                            raw[0x300 + 12 * actor:0x303 + 12 * actor] = bytes((kind, flags, alternate))
                            raw[0x635], raw[0x637], raw[0xe2], raw[0xe3] = x, y, first, second
                            raw[0x5fc], raw[0x638] = 250, 250
                            positions = []
                            for player in range(12, 22):
                                px, py = ((200 if pattern == 0 else 60 + 10 * (player - 12)), 80 + 8 * (player - 12))
                                raw[0x306 + 12 * player], raw[0x308 + 12 * player] = px, py
                                positions.extend((px, py))
                            candidate = original_candidate(rom[16:262160], raw)[2]
                            final = original_choice(rom[16:262160], raw) if candidate == 2 else None
                            if final is not None and final[0x43b] == 2:
                                if final[0x43c] or final[0x5fc] != 250 or final[0x638] != 250:
                                    raise RuntimeError('Unexpected original fallback effects')
                                want = f'2 0 {final[0xe2]} -1 -1'
                            elif (args.pass_gates and final is not None and final[0x43b] == 1 and kind < 118
                                  and word(word(0x8e1b + 2 * kind) + 2) == 0):
                                if final[0x43c] or final[0x5fc] not in range(12, 22):
                                    raise RuntimeError('Unexpected original zero-option pass')
                                want = f'1 0 {final[0xe2]} {final[0x5fc]} {final[0x638]}'
                            else:
                                want = 'unsupported'
                            cases.append((kind, flags, alternate, actor, x, y, first, second, *positions))
                            expected.append(want)
    result = subprocess.run([str(args.exe.resolve()), '--field-plan', str(args.pack.resolve())],
                            input=''.join(' '.join(map(str, row)) + '\n' for row in cases),
                            capture_output=True, text=True, check=True, timeout=30)
    actual = result.stdout.splitlines()
    if len(actual) != len(expected):
        raise RuntimeError('Native field plan result count differs')
    for case, observed, want in zip(cases, actual, expected):
        if observed != want:
            raise RuntimeError(f'Field plan differs for {case}: {observed} != {want}')
    fallbacks = sum(value.startswith('2 ') for value in expected)
    passes = sum(value.startswith('1 ') for value in expected)
    print(f'{fallbacks} original field AI fallback and {passes} pass plans matched; {len(expected)-fallbacks-passes} other choices rejected')


if __name__ == '__main__':
    main()

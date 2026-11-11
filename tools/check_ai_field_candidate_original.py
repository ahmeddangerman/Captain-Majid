#!/usr/bin/env python3
"""Compare field-contact opponent AI candidate selection with a private ROM.

Only the numeric candidate boundary is checked; choice adjustments, animation,
action execution and real NMI/audio scheduling are outside this comparison.
"""
import argparse
import hashlib
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original_candidate(prg, raw):
    memory = ObservedCartridge(prg, raw, None, set())
    memory.registers[6:8] = [28, 29]
    memory[0x24], memory[0x25] = 28, 29
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = 0x875d, 0x50, cpu.UNUSED
    for _ in range(10000):
        if memory.bank_at(cpu.pc) == 28 and cpu.pc == 0x87b0:
            pointer = memory[0x3c] | (memory[0x3d] << 8)
            offset = pointer - 0xb12e
            if offset < 0 or offset % 48:
                raise RuntimeError('Original field candidate row is outside the represented table')
            return memory[0x3e], offset // 48, cpu.a
        if cpu.pc < 0x8000 or cpu.pc in (0xcaae, 0xcb0f):
            raise RuntimeError(f'Unexpected original scheduler or return at {cpu.pc:04x}')
        cpu.step()
    raise RuntimeError('Original field candidate instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'ram', 'exe'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    base = bytearray(args.ram.read_bytes())
    if len(base) != 2048 or base[0x22] & 64:
        parser.error('Expected compatible original internal RAM')
    prg = rom[16:262160]
    expected, inputs = [], []
    for kind in (1, 7, 23, 35, 70, 117):
        for flags, alternate in ((0, 0), (128, 35)):
            for x in (52, 80, 95, 96, 112, 159, 160, 200):
                for y in (84, 124, 131, 172):
                    for first in range(8):
                        raw = bytearray(base)
                        actor = 19
                        raw[0x441], raw[0x5fb], raw[0x621] = actor, 11, 0
                        raw[0x300 + 12 * actor:0x303 + 12 * actor] = bytes((kind, flags, alternate))
                        raw[0x635], raw[0x637], raw[0xe2] = x, y, first
                        expected.append(original_candidate(prg, raw))
                        inputs.append((kind, flags, alternate, x, y, first))
    run = subprocess.run([str(args.exe.resolve()), '--field', str(args.pack.resolve())],
                         input=''.join(' '.join(map(str, row)) + '\n' for row in inputs),
                         capture_output=True, text=True, check=True, timeout=30)
    actual = run.stdout.splitlines()
    if len(actual) != len(expected):
        raise RuntimeError('Native result count differs')
    for case, observed, wanted in zip(inputs, actual, expected):
        if observed != ' '.join(map(str, wanted)):
            raise RuntimeError(f'Field AI candidate differs for {case}: {observed} != {wanted}')
    print(f'{len(expected)} constructed original field AI candidate selections matched')


if __name__ == '__main__':
    main()

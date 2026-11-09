#!/usr/bin/env python3
"""Compare profiled control travel to the original CPU with private inputs.

Runs $E3CA through RTS, including the original banked speed loader. The base
RAM and selected-team pack are supplied explicitly; this does not simulate
frame scheduling, NMI, presentation or sound.
"""
import argparse
import hashlib
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from reference_identity import PAYLOAD_SHA256
from trace_field_script import ObservedCartridge


def cases():
    n = 0
    for player in (1, 9, 12, 19, 21):
        side = 0 if player < 11 else 11
        for player_type in range(1, 256):
            for level in (0, 127, 255):
                yield player, player_type, 0, 0, level, side, 1, 64 if n & 1 else 0, (n * 911) & 65535
                n += 1
    for player in range(12, 22):
        for level in (0, 127, 255):
            yield player, 0, 0, 0, level, 11, 0, 64 if n & 1 else 0, (n * 911) & 65535
            n += 1
    for alternate in range(35, 256):
        yield 19, 35, 128, alternate, 0, 11, 0, 64 if n & 1 else 0, (n * 911) & 65535
        n += 1
    for held in (0, 16, 32, 64, 128, 240):
        yield 9, 1, 0, 0, 0, 0, held, 64, 12345


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('pack', type=Path)
    parser.add_argument('ram', type=Path)
    parser.add_argument('--binary', type=Path, required=True)
    args = parser.parse_args()
    rom, base = args.rom.read_bytes(), args.ram.read_bytes()
    if rom[:4] != b'NES\x1a' or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('ROM payload does not match the verified Japanese reference')
    if len(base) != 2048 or base[0x22] & 64:
        parser.error('Expected 2048-byte private RAM with supported mapper mode')
    prg = rom[16:262160]
    inputs, expected = [], []
    for case in cases():
        player, player_type, low, high, level, side, held, facing, travel = case
        raw = bytearray(base)
        record = 0x300 + 12 * player
        raw[record:record + 4] = bytes((player_type, low, high, level))
        raw[0x441], raw[0x5fb], raw[0x1c], raw[0x517] = player, side, held, facing
        raw[0x642], raw[0x643] = travel & 255, travel >> 8
        raw[0x1fe], raw[0x1ff] = 255, 1
        memory = ObservedCartridge(prg, raw, None, set())
        memory.registers[6:8] = [raw[0x24], raw[0x25]]
        memory.select = raw[0x23]
        cpu = MPU(memory=memory)
        cpu.pc, cpu.sp, cpu.p = 0xe3ca, 0xfd, cpu.UNUSED
        loaded_speed = None
        for _ in range(2000):
            if cpu.pc == 0x200:
                break
            if cpu.pc == 0xe3de:
                loaded_speed = memory[0x32] | memory[0x33] << 8
            cpu.step()
        else:
            raise RuntimeError(f'Original speed/travel instruction bound for {case}')
        speed = -1 if loaded_speed is None else loaded_speed
        inputs.append(case)
        expected.append((speed, memory[0x642] | memory[0x643] << 8))
    data = ''.join(' '.join(map(str, case)) + '\n' for case in inputs)
    run = subprocess.run([str(args.binary.resolve()), '--profile-batch', str(args.pack.resolve())],
                         input=data, text=True, capture_output=True, check=True, timeout=120)
    actual = [tuple(map(int, line.split())) for line in run.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f'Native result count {len(actual)} != original {len(expected)}')
    for index, (got, want) in enumerate(zip(actual, expected)):
        if got != want:
            raise RuntimeError(f'Case {index}, {inputs[index]}: native {got}, original {want}')
    print(f'Matched {len(expected)} original profiled speed/travel fixtures with supplied team context.')


if __name__ == '__main__':
    main()

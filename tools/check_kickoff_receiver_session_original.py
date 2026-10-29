#!/usr/bin/env python3
"""Compare saved kickoff receiver handoff to the approved original CPU segment."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'formations_dir', 'output_dir'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    rom = a.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True, exist_ok=False)
    m = ObservedCartridge(rom[16:262160], bytes(2048), None, set())
    m.registers[6:8] = [26, 27]
    table = [m[i] for i in range(0xdc82, 0xdc87)]
    binary = str(Path('build/captain-phase-session').resolve())
    count = 0
    for side in (0, 11):
        source = a.formations_dir / f's{side}-l0-r0-n0-native.cmjsession'
        original = bytearray(source.read_bytes())
        if original[258] != 61 or original[10] not in (ord('R'), ord('S'), ord('T'), ord('U'), ord('V'), ord('W'), ord('X'), ord('Y'), ord('Z'), ord('a'), ord('b')):
            raise ValueError('Expected saved placed formation')
        if original[10] == ord('R'):
            original[10] = ord('b')
            original += bytes(2)
        original[10] = ord('b')
        for random in range(256):
            m[0x5fb] = side
            m[0xe2] = random
            cpu = MPU(memory=m)
            cpu.pc = 0xdaf5
            cpu.p = cpu.UNUSED
            for steps in range(50):
                if cpu.pc == 0xdb0b:
                    break
                cpu.step()
            else:
                raise RuntimeError('Original receiver instruction bound')
            expected = bytearray(original)
            expected[258] = 62
            expected[368:370] = bytes([1, m[0x5fc]])
            target = out / f's{side}-r{random}.cmjsession'
            subprocess.run([binary, str(a.pack.resolve()), str(source.resolve()),
                            '--prepare-kickoff-receiver', str(random), *map(str, table),
                            '--capture', str(target)], capture_output=True, check=True, timeout=30)
            actual = target.read_bytes()
            if actual != expected:
                raise RuntimeError(f'{side=} {random=} mismatch: '
                                   f'{[(i, x, y) for i, (x, y) in enumerate(zip(actual, expected)) if x != y]}')
            count += 1
    (out / 'verification.json').write_text(json.dumps({
        'cases': count,
        'scope': 'Original DAF5..DB0B executes every instruction; saved session fields remain unchanged except phase and receiver. Earlier reset, ball placement, presentation and live frames are outside scope.'
    }, indent=2) + '\n')
    print(f'{count} saved receiver comparisons match: {out}')


if __name__ == '__main__':
    main()

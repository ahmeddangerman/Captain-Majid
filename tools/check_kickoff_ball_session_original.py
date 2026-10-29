#!/usr/bin/env python3
"""Compare saved kickoff ball placement with the approved original CPU."""
import argparse
import hashlib
import json
import random
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'receivers_dir', 'output_dir'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    rom = a.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True, exist_ok=False)
    rng = random.Random(0xE6EC)
    binary = str(Path('build/captain-phase-session').resolve())
    points = [(48, 80), (207, 175), (48, 175), (207, 80), (55, 87), (56, 88)]
    points += [(rng.randrange(48, 208), rng.randrange(80, 176)) for _ in range(122)]
    count = 0
    for side in (0, 11):
        saved = bytearray((a.receivers_dir / f's{side}-r0.cmjsession').read_bytes())
        if saved[258] != 62 or saved[20] != side or saved[21] != side + 8:
            raise ValueError('Expected saved kickoff receiver and initial owner')
        saved[10] = ord('l')
        saved += bytes(372-len(saved))
        motion = 122 + (7 if side == 0 else 17) * 6
        for index, (x, y) in enumerate(points):
            source_bytes = bytearray(saved)
            source_bytes[motion:motion + 4] = bytes([rng.randrange(256), x, rng.randrange(256), y])
            source = out / f's{side}-{index}-input.cmjsession'
            source.write_bytes(source_bytes)
            ram = bytearray(2048)
            ram[0x441] = side + 8
            record = 0x300 + 12 * (side + 8)
            ram[record + 5:record + 9] = source_bytes[motion:motion + 4]
            ram[0x634], ram[0x636] = 0x91, 0x72
            m = ObservedCartridge(rom[16:262160], ram, None, set())
            m.registers[6:8] = [26, 27]
            cpu = MPU(memory=m)
            cpu.pc = 0xdb0b
            cpu.sp = 0xfd
            cpu.p = cpu.UNUSED
            for steps in range(500):
                if cpu.pc == 0xdb0e:
                    break
                cpu.step()
            else:
                raise RuntimeError('Original kickoff ball instruction bound')
            if m[0x634] != 0x91 or m[0x636] != 0x72:
                raise RuntimeError('Original unexpectedly modified fractional ball bytes')
            expected = bytearray(source_bytes)
            expected[22:25] = bytes([m[0x635], m[0x637], m[0x5fe]])
            expected[258] = 63
            target = out / f's{side}-{index}-native.cmjsession'
            subprocess.run([binary, str(a.pack.resolve()), str(source), '--prepare-kickoff-ball',
                            '--capture', str(target)], capture_output=True, check=True, timeout=30)
            actual = target.read_bytes()
            if actual != expected:
                raise RuntimeError(f'{side=} {x=} {y=} mismatch: '
                                   f'{[(i, a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b]}')
            count += 1
    (out / 'verification.json').write_text(json.dumps({
        'cases': count,
        'scope': 'Original DB0B..DB0E and called E6EC execute every instruction. Full represented snapshot compared; original fractional ball bytes remain unchanged but are not saved by this session. Only interior field coordinates are covered. Presentation, receiver transfer, earlier reset and live frames are outside scope.'
    }, indent=2) + '\n')
    print(f'{count} saved kickoff ball comparisons match: {out}')


if __name__ == '__main__':
    main()

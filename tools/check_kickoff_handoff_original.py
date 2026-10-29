#!/usr/bin/env python3
"""Compare prepared post-display kickoff handoff against the original CPU."""
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
    for name in ('rom', 'pack', 'ball_dir', 'output_dir'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    rom = a.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True, exist_ok=False)
    binary = str(Path('build/captain-phase-session').resolve())
    count = 0
    for side in (0, 11):
        base = bytearray((a.ball_dir / f's{side}-0-native.cmjsession').read_bytes())
        if base[258] != 63 or base[20] != side or base[21] != side + 8 or base[296] != 1:
            raise ValueError('Expected saved kickoff ball and control context')
        base[10] = ord('k')
        base += bytes(372-len(base))
        for slot in (5, 6, 7, 9, 10):
            for old_control in (0, 1, 10, 255):
                source_bytes = bytearray(base)
                source_bytes[369] = side + slot
                source_bytes[299] = old_control
                source = out / f's{side}-p{slot}-c{old_control}-input.cmjsession'
                source.write_bytes(source_bytes)
                ram = bytearray(2048)
                ram[0x5fc], ram[0x5fd], ram[0x441] = side + slot, old_control, side + 8
                m = ObservedCartridge(rom[16:262160], ram, None, set())
                m.registers[6:8] = [26, 27]
                cpu = MPU(memory=m)
                cpu.pc = 0xdb13
                cpu.p = cpu.UNUSED
                for steps in range(20):
                    if cpu.pc == 0xdb1e:
                        break
                    cpu.step()
                else:
                    raise RuntimeError('Original kickoff handoff instruction bound')
                expected = bytearray(source_bytes)
                expected[21], expected[299], expected[258] = m[0x441], m[0x5fd], 64
                target = out / f's{side}-p{slot}-c{old_control}-native.cmjsession'
                subprocess.run([binary, str(a.pack.resolve()), str(source), '--continue-prepared-kickoff',
                                '--capture', str(target)], capture_output=True, check=True, timeout=30)
                actual = target.read_bytes()
                if actual != expected:
                    raise RuntimeError(f'{side=} {slot=} {old_control=} mismatch: '
                                       f'{[(i, a, b) for i, (a, b) in enumerate(zip(actual, expected)) if a != b]}')
                count += 1
    (out / 'verification.json').write_text(json.dumps({
        'cases': count,
        'scope': 'Original DB13..DB1E executes every instruction after an explicitly acknowledged display36. Full represented snapshots compared. Display effects and control setup at E0DF are not simulated.'
    }, indent=2) + '\n')
    print(f'{count} saved kickoff handoff comparisons match: {out}')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare known ball fractions across the first-fixture keeper rebound.

The original starts from a supplied private save state. Cooperative waits are
acknowledged; this does not simulate rendering, NMI, audio, or live timing.
"""
import argparse
import subprocess
from pathlib import Path
from trace_ball_recovery_phase import trace_recovery

p = argparse.ArgumentParser(description=__doc__)
for name in ('rom', 'ram', 'session', 'pack', 'exe'):
    p.add_argument('--' + name, type=Path, required=True)
a = p.parse_args()
rom = a.rom.read_bytes()
if len(rom) != 393232:
    raise ValueError('Expected original cartridge image')
prg = rom[16:262160]
raw = a.ram.read_bytes()
for x in (0, 1, 127, 255):
    for y in (0, 1, 127, 255):
        before = bytearray(raw)
        before[0x634], before[0x636] = x, y
        report, after = trace_recovery(prg, bytes(before))
        if report['boundary'] != 'recovered_owner_handoff' or (after[0x441], after[0x5fb], after[0x635], after[0x637], after[0x634], after[0x636]) != (17, 11, 52, 100, x, y):
            raise ValueError(f'Original recovery differs for fractions {x}, {y}')
subprocess.run([str(a.exe.resolve()), '--check-first-keeper-recovery-fraction', str(a.session.resolve()), str(a.pack.resolve())], check=True)
print('16 original/native first-fixture keeper rebounds preserve known fractions')

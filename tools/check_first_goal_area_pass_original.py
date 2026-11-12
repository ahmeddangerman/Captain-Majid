#!/usr/bin/env python3
"""Compare the controlled first-fixture rebound pass to private original RAM.

The original trace injects provisional random bytes and acknowledges waits. It
is not evidence of original presentation, NMI, audio, or live timing.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--trace', type=Path, required=True)
p.add_argument('--contact', type=Path, required=True)
a = p.parse_args()
def check(native, original):
    s = (a.trace / native).read_bytes()
    r = (a.trace / original).read_bytes()
    if len(r) != 2048:
        raise ValueError('Expected private original RAM')
    compare_represented(s, export_session(r, 0), False, 'first fixture', original)
    return s, r
for name, original in (('native-dispatch.cmjsession', '83f5-1.bin'), ('native-moved.cmjsession', '83f8-1.bin')):
    check(name, original)
for index in range(1, 12):
    name = 'native-flight.cmjsession' + (f'.{index}' if index > 1 else '')
    state, raw = check(name, f'e773-{index}.bin')
    expected = raw[0x634:0x638] + raw[0x639:0x63d] + bytes((raw[0x62c], raw[0x61b]))
    if state[345:355] != expected:
        raise ValueError(f'Trajectory differs at crossing {index}')
s = (a.trace / 'native-flight.cmjsession.12').read_bytes()
r = (a.contact / 'e7cc-1.bin').read_bytes()
compare_represented(s, export_session(r, 0), False, 'first fixture', 'arrival')
if s[345:355] != r[0x634:0x638] + r[0x639:0x63d] + bytes((r[0x62c], r[0x61b])):
    raise ValueError('Arrival trajectory differs')
s = (a.trace / 'native-flight.cmjsession.contact').read_bytes()
r = (a.contact / '8840-1.bin').read_bytes()
compare_represented(s, export_session(r, 0), False, 'first fixture', 'defender entry')
print('Matched dispatch, movement, setup, 10 crossings, arrival and four-defender entry')

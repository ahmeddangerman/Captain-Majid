#!/usr/bin/env python3
"""Compare the first two five-defender pass contacts to private original RAM.

The original CPU starts from an injected represented native preparation; its
presentation waits are acknowledged and live timing is not simulated.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--trace', type=Path, required=True)
p.add_argument('--second-prepared', type=Path, required=True)
p.add_argument('--third-prepared', type=Path, required=True)
a = p.parse_args()
def compare(native, original, boundary):
    s = native.read_bytes(); r = original.read_bytes()
    if len(r) != 2048:
        raise ValueError('Expected private original RAM')
    compare_represented(s, export_session(r, 0), True, 'five defenders', boundary)
compare(a.second_prepared, a.trace / '80b6-2.bin', 'second preparation')
compare(a.third_prepared, a.trace / '80b6-3.bin', 'third preparation')
for index, name in ((1, 'first'), (2, 'second')):
    for kind, pc in (('compared', '80d9'), ('clocked', '80dc')):
        raw = a.trace / f'{pc}-{index}.bin'
        compare(a.trace / f'native-{name}-{kind}.cmjsession', raw, name + ' ' + kind)
        r = raw.read_bytes()
        if r[0x612] != 1 or r[0x619] != 0 or (r[0x61c], r[0x61d]) != (7, 0):
            raise ValueError('Unexpected result, loss or attack power')
compare(a.trace / 'native-third-compared.cmjsession', a.trace / '80d9-3.bin', 'third comparison')
for name, original in (('blocked', '8bdf-1.bin'), ('recovered', '8c31-1.bin'),
                       ('handoff', 'e0df-1.bin'), ('ready', 'control-ready.bin')):
    s = (a.trace / f'native-third-{name}.cmjsession').read_bytes()
    r = (a.trace / original).read_bytes()
    compare_represented(s, export_session(r, 0), False, 'five defenders', name)
if (r[0x441], r[0x5fb]) != (3, 0):
    raise ValueError('Original recovery owner differs')
print('Matched two result1 contacts, result2 recovery and player3 control-ready')

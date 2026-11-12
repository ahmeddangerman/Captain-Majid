#!/usr/bin/env python3
"""Compare the controlled player2 pass timeout and next player1 encounter.

Original CPU observations inject represented field state and random samples;
cooperative presentation waits are acknowledged, without NMI or live rendering.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--trace', type=Path, required=True)
p.add_argument('--contact-native', type=Path, required=True)
p.add_argument('--contact-original', type=Path, required=True)
p.add_argument('--menu-original', type=Path, required=True)
a = p.parse_args()
for name, original, extended in (('handoff', 'e0df-1.bin', True), ('ready', 'e145-1.bin', False)):
    state = (a.trace / f'native-{name}.cmjsession').read_bytes()
    raw = (a.trace / original).read_bytes()
    compare_represented(state, export_session(raw, 0), extended, 'player2 timeout', name)
    if (raw[0x441], raw[0x5fb], raw[0x634], raw[0x636]) != (1, 0, 173, 87):
        raise ValueError('Original timeout possession or fractions differ')
for state_path, raw_path, label in ((a.contact_native, a.contact_original, 'contact'),
                                    (Path(str(a.contact_native) + '.menu'), a.menu_original, 'menu')):
    state, raw = state_path.read_bytes(), raw_path.read_bytes()
    compare_represented(state, export_session(raw, 0), False, 'player1', label)
    if (raw[0x616], raw[0x600], raw[0x601]) != (1, 1, 12):
        raise ValueError('Original retained cursor or defender differs')
print('Matched player1 timeout recovery, control, defender12 contact and menu')

#!/usr/bin/env python3
"""Check the controlled player1 defended shot against private original RAM.

The original opponent decision uses injected random bytes; presentation waits
and two observed motion/facing changes are acknowledged, without NMI or audio.
"""
import argparse
import hashlib
from pathlib import Path
from export_phase_session import export_session
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256
from verify_player7_pass_end_to_end import compare_represented

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--rom', type=Path, required=True)
p.add_argument('--trace', type=Path, required=True)
a = p.parse_args()
rom = a.rom.read_bytes()
if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
    raise ValueError('Expected verified original ROM')
m = ObservedCartridge(rom[16:262160], bytes(2048), None, set());m.registers[6:8] = [26, 27]
ptr = m[0x8d93] | m[0x8d94] << 8
if (bytes(m[ptr+2:ptr+4]), m[0x827d] & 128) != (bytes(m[ptr+10:ptr+12]), m[0x8281] & 128):
    raise ValueError('Shot and pass tackle comparison entries differ')
def compare(name, original, extended=True):
    state = (a.trace / f'native-{name}.cmjsession').read_bytes()
    raw = (a.trace / original).read_bytes()
    compare_represented(state, export_session(raw, 0), extended, 'player1 shot', name)
    return state, raw
for name, pc in (('selected','8637'),('prepared','804f'),('defender','80b6'),('compared','80d9'),('clocked','80dc'),('dispatch','80ed'),('moved','c618'),('target','dd08')):
    compare(name, pc+'-1.bin')
for index in range(1,15):
    name = 'flight' if index == 1 else f'flight-{index}'
    original = f'e773-{index}.bin' if index <= 13 else 'e7cc-1.bin'
    state, raw = compare(name, original)
    if state[346:356] != raw[0x634:0x638] + raw[0x639:0x63d] + bytes((raw[0x62c], raw[0x61b])):
        raise ValueError(f'Shot trajectory differs at {index}')
for name, pc in (('recovered','8c31'),('area','8b4a'),('area-prepared','8b73')):
    state, raw = compare(name, pc+'-1.bin', False)
    if (raw[0x441],raw[0x5fb],raw[0x616]) != (8,0,1):
        raise ValueError('Original recovered owner, side or cursor differs')
print('Matched defender choice, shot preparation, flight and player8 goal-area recovery')

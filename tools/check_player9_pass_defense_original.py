#!/usr/bin/env python3
"""Compare a controlled player9 five-defender pass against original AI."""
import argparse
import hashlib
from pathlib import Path
from check_generic_pass_defense_original import original
from export_phase_session import export_session
from verify_action_strength import PAYLOAD_SHA256
from verify_player7_pass_end_to_end import compare_represented

p=argparse.ArgumentParser(description=__doc__)
for name in ('rom','injected','trace','menu','contacts','prepared','player6-area'):p.add_argument('--'+name,type=Path,required=True)
a=p.parse_args();rom=a.rom.read_bytes();base=bytearray(a.injected.read_bytes())
if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:raise ValueError('Expected verified original ROM and private RAM')
menu=(a.trace/'native-player9-menu.cmjsession').read_bytes();compare_represented(menu,export_session(a.menu.read_bytes(),0),False,'player9','five-defender menu')
choices=[]
for i,actor in enumerate((13,14,18,19,20)):
 s=(a.trace/f'native-player9-defense-{i}.cmjsession').read_bytes();raw=bytearray(base);raw[0xe2],raw[0xe3]=s[26:28];raw[0x442]=actor
 action,subtype=original(rom[16:262160],raw,actor);choices.append(action)
 if subtype or s[-11]!=i+1 or list(s[-10:-5])[:i+1]!=choices or any(s[-5:]):raise ValueError('Original defender choices disagree')
 compare_represented(s,export_session(raw,0),True,'player9',f'defender {i+1}')
compare_represented((a.trace/'native-player9-pass-prepared.cmjsession').read_bytes(),export_session(a.prepared.read_bytes(),0),True,'player9','pass preparation')
for i,label in enumerate(('first','second','third','contact-4','contact-5'),1):
 for kind,pc in (('prepared','80b6'),('compared','80d9'),('clocked','80dc')):
  s=(a.trace/f'native-player9-{label}-{kind}.cmjsession').read_bytes();raw=(a.contacts/f'{pc}-{i}.bin').read_bytes()
  compare_represented(s,export_session(raw,0),True,'player9',f'contact {i} {kind}')
  if kind!='prepared' and raw[0x612]!=0:raise ValueError('Unexpected original defender result')
for name,pc in (('dispatch','80ed-1'),('pass-moved','80f0-1'),('pass-target','de64-1'),('flight','e773-1'),('flight-step-1','e773-2'),('flight-step-3','e773-3'),('flight-step-5','e773-4'),('flight-step-7','e773-5'),('flight-step-8','8845-1'),('arrival','888a-1'),('race','8c31-1')):
 s=(a.trace/f'native-player9-{name}.cmjsession').read_bytes();raw=(a.contacts/f'{pc}.bin').read_bytes()
 compare_represented(s,export_session(raw,0),True,'player9',name)
 if name in ('dispatch','pass-moved') and (s[413],raw[0x616])!=(5,5):raise ValueError('Original exhausted defender cursor differs')
 if name=='race' and (raw[0x441],raw[0x5fb])!=(6,0):raise ValueError('Unexpected original pass recovery owner')
for name,pc in (('area','8b4a-1'),('area-prepared','8b73-1'),('menu','d5e5-1')):
 raw=(a.player6_area/f'{pc}.bin').read_bytes()
 compare_represented((a.trace/f'native-player6-{name}.cmjsession').read_bytes(),export_session(raw,0),False,'player6',name)
 if raw[0x441]!=6 or raw[0x600]!=0:raise ValueError('Unexpected player6 goal-area state')
print('Matched player9 menu, five defender choices and contacts, flight and player6 recovery:',choices)

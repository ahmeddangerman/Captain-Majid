#!/usr/bin/env python3
"""Compare native recipient selection, random mutation and field cell to ROM."""
import argparse,hashlib,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from trace_ai_area_recipient import observe_recipient
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','keeper_ram'):p.add_argument(n,type=Path)
 p.add_argument('--pack',type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
 prg=rom[16:262160];m=ReferenceCartridge(prg);m.registers[6:8]=[28,29]
 base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last']);cases=[];profiles=[];expected=[]
 for kind in range(35,204):
  for pattern in ('left','right','spread'):
   for first in (*range(8),128,255):
    for second in (0,127,255):
     raw=bytearray(base);actor=12+(kind%10);bx=(80,120,200)[kind%3]
     raw[0x441],raw[0x621],raw[0x635],raw[0xe2],raw[0xe3]=actor,2,bx,first,second
     positions=[]
     for player in range(12,22):
      x=80 if pattern=='left' else 160 if pattern=='right' else 60+10*(player-12)
      y=80+8*(player-12);raw[0x306+12*player],raw[0x308+12*player]=x,y;positions.extend((x,y))
     row=m[0x9662+12*(kind-35)+10];entry=m[0xb72e+8*row+(first&7)]
     result=observe_recipient(prg,raw,kind)
     cases.append((entry,actor,bx,first,second,*positions));profiles.append((kind,0,0,actor,bx,first,second,*positions));expected.append(f'{result["action"]} {result["random_first"]} {result["recipient"] if result["recipient"] is not None else -1} {result["cell"] if result["cell"] is not None else -1}')
 run=subprocess.run(['build/ai-action-candidate-tests','--recipient'],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 lines=run.stdout.splitlines()
 if len(lines)!=len(expected):raise RuntimeError('Native output count mismatch')
 for case,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{case}: {x} != {y}')
 if a.pack:
  run=subprocess.run(['build/ai-action-candidate-tests','--recipient-profile',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in profiles),capture_output=True,text=True,check=True,timeout=30)
  if run.stdout.splitlines()!=expected:raise RuntimeError('Profile-selected recipient results differ from original')
 print(f'{len(cases)} original recipient/action/cell/random-state cases match; subtype lookup pending')
if __name__=='__main__':main()

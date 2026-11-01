#!/usr/bin/env python3
"""Compare supported complete opponent area choice before attack preparation."""
import argparse,hashlib,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from trace_area_attack_choice import observe_area_attack_choice
from verify_action_strength import PAYLOAD_SHA256
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
 prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last'])
 base[0x60b:0x60f]=bytes([4]*4);base[0x606:0x60a]=bytes(4);base[0x61e]=4;cases=[];expected=[]
 for kind in range(35,118):
  for pattern in range(3):
   for first in range(8):
    for second in (0,127,255):
     raw=bytearray(base);actor=12+kind%10;flags=128 if kind%2 else 0;alt=35+(kind+7)%169
     x,y=((51,124),(204,131),(75,100))[pattern];variant=1+kind%2
     raw[0x441],raw[0x635],raw[0x637],raw[0xe2],raw[0xe3],raw[0x44e]=actor,x,y,first,second,variant
     raw[0x300+12*actor:0x303+12*actor]=bytes((kind,flags,alt));positions=[]
     for player in range(12,22):
      px=80 if pattern==0 else 160 if pattern==1 else 60+10*(player-12);py=80+8*(player-12)
      raw[0x306+12*player],raw[0x308+12*player]=px,py;positions.extend((px,py))
     result,final=observe_area_attack_choice(prg,raw)
     target=final[0x5fc] if result['action']==1 else -1;cell=final[0x638] if target!=-1 else -1
     if target==-1 and (final[0x5fc]!=raw[0x5fc] or final[0x638]!=raw[0x638]):raise RuntimeError('Unexpected target mutation')
     cases.append((kind,flags,alt,actor,x,y,first,second,variant,*positions));expected.append(f'{result["action"]} {result["subtype"]} {final[0xe2]} {target} {cell}')
 run=subprocess.run(['build/ai-action-candidate-tests','--plan',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 lines=run.stdout.splitlines()
 if len(lines)!=len(expected):raise RuntimeError('Native count mismatch')
 for c,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{c}: {x} != {y}')
 print(f'{len(cases)} original complete area choices match action/subtype/target/cell/random; attack execution pending')
if __name__=='__main__':main()

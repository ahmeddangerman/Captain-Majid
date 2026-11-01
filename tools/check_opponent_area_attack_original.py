#!/usr/bin/env python3
"""Compare opponent area attack numeric preparation through original8F72."""
import argparse,hashlib,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from trace_area_attack_choice import observe_area_attack_choice
from verify_action_strength import PAYLOAD_SHA256
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last'])
 base[0x60b:0x60f]=bytes([4]*4);base[0x606:0x60a]=bytes(4);base[0x61e]=4;cases=[];expected=[]
 for kind in (35,50,70,117):
  for level in (0,15,127,255):
   for variant in (1,2):
    for first in (*range(8),128,255):
     for second in (0,255):
      raw=bytearray(base);actor=raw[0x441];off=0x300+12*actor;raw[off:off+4]=bytes((kind,0,0,level));raw[0x44e],raw[0xe2],raw[0xe3]=variant,first,second
      choice,chosen=observe_area_attack_choice(prg,raw);report,final=observe_area_attack_choice(prg,raw,prepare_attack=True)
      cases.append((actor,kind,0,0,level,choice['action'],variant,choice['subtype'],chosen[0xe2],second))
      cost=final[0x43f]+256*final[0x440];power=final[0x61c]+256*final[0x61d];stamina=final[off+1]+256*final[off+2]
      expected.append(f'{cost} {final[0x444]} {final[0x443]} {power} {final[0x43c]} {stamina}')
 run=subprocess.run(['build/ai-action-candidate-tests','--prepare',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 lines=run.stdout.splitlines()
 if len(lines)!=len(cases):raise RuntimeError('Native count mismatch')
 for c,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{c}: {x} != {y}')
 print(f'{len(cases)} opponent attack cost/effect/flags/power/subtype/stamina preparations match original')
if __name__=='__main__':main()

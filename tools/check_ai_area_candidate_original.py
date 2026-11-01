#!/usr/bin/env python3
"""Compare route2 AI profile/category/candidate before final action adjustments."""
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
 base[0x60b:0x60f]=bytes([4]*4);base[0x606:0x60a]=bytes(4);base[0x61e]=4
 cases=[];expected=[]
 for kind in range(35,204):
  for flags in (0,128):
   alternate=35+(kind-35+7)%169
   for x in (51,59,67,75):
    for y in (100,124):
     for r in range(8):
      raw=bytearray(base);actor=raw[0x441];offset=0x300+12*actor
      raw[offset:offset+3]=bytes((kind,flags,alternate));raw[0x635],raw[0x637],raw[0xe2]=x,y,r
      report,_=observe_area_attack_choice(prg,raw,candidate_only=True)
      cases.append((kind,flags,alternate,x,y,r));expected.append(f'{report["category"]} {report["row"]} {report["candidate"]}')
 invalid=[(0,0,0,51,124,0),(34,0,0,51,124,0),(204,0,0,51,124,0),(35,128,34,51,124,0),(35,0,0,0,0,0)]
 cases+=invalid;expected+=['unsupported']*len(invalid)
 run=subprocess.run(['build/ai-action-candidate-tests','--area',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n' for c in cases),capture_output=True,text=True,check=True,timeout=30)
 lines=run.stdout.splitlines()
 if len(lines)!=len(expected):raise RuntimeError('Native output count mismatch')
 for c,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{c}: {x} != {y}')
 print(f'{len(cases)-len(invalid)} original AI profile/category/candidate cases match; {len(invalid)} unsupported inputs reject')
if __name__=='__main__':main()

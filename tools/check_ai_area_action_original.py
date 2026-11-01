#!/usr/bin/env python3
"""Compare supported route2 final AI branches; recipient selection stays pending."""
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
 cases=[(kind,flags,35+(kind-35+7)%169,x,y,r)for kind in range(35,204)for flags in (0,128)for x in (51,59,204,196)for y in (100,131)for r in range(8)]
 run=subprocess.run(['build/ai-action-candidate-tests','--action',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 results=run.stdout.splitlines()
 if len(results)!=len(cases):raise RuntimeError('Native output count mismatch')
 matched=0;pending=0
 for case,output in zip(cases,results):
  kind,flags,alternate,x,y,r=case;raw=bytearray(base);actor=raw[0x441];offset=0x300+12*actor
  raw[offset:offset+3]=bytes((kind,flags,alternate));raw[0x635],raw[0x637],raw[0xe2]=x,y,r
  candidate,_=observe_area_attack_choice(prg,raw,candidate_only=True)
  if candidate['candidate']==2:
   if output!='unsupported':raise RuntimeError('Recipient branch silently substituted')
   pending+=1;continue
  report,final=observe_area_attack_choice(prg,raw)
  if output!=f'{report["action"]} {report["subtype"]}':raise RuntimeError(f'{case}: {output} differs from original')
  if final[0xe2]!=r:raise RuntimeError('Unexpected random mutation in supported branch')
  matched+=1
 print(f'{matched} final AI action/subtype pairs match original; {pending} recipient-choice cases explicitly pending')
if __name__=='__main__':main()

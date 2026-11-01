#!/usr/bin/env python3
"""Compare native area defender skill preparation and stamina debit to original."""
import argparse,hashlib,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from verify_action_strength import PAYLOAD_SHA256
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active']);cases=[];expected=[]
 for kind in (23,24,32):
  for level in (0,15,255):
   for defense,subtype in ((2,0),(2,1),(4,0)):
    for variant in (1,2):
     for attack in (1,4,6):
      for attack_subtype in (0,128,129):
       for stamina in (0,1,80,180,65535):
        actor=1+kind%10;raw=bytearray(base);off=0x300+12*actor;raw[off:off+4]=bytes((kind,stamina&255,stamina>>8,level))
        raw[0x601],raw[0x60b],raw[0x606],raw[0x44e],raw[0x43b],raw[0x43c],raw[0x616]=actor,defense,subtype,variant,attack,attack_subtype,0
        m=ObservedCartridge(prg,raw,None,set());m.registers[6:8]=[26,27];m[0x24],m[0x25]=26,27;m[0x23]=m[0x22]|7;m.select=m[0x23]
        c=MPU(memory=m);c.pc,c.sp,c.p=0x89da,0xfd,c.UNUSED
        for step in range(4000):
         if m.bank_at(c.pc)==26 and c.pc==0x8a05:break
         if c.pc<0x8000 or c.pc in (0xcb0f,0xcaae):raise RuntimeError('Unsupported defender preparation path')
         c.step()
        else:raise RuntimeError('Defender preparation instruction bound')
        power=m[0x32]+256*m[0x33];cost=m[0x43f]+256*m[0x440];remaining=m[off+1]+256*m[off+2]
        cases.append((actor,kind,stamina&255,stamina>>8,level,attack,attack_subtype,variant,defense,subtype));expected.append(f'{power} {m[0x445]} {cost} {remaining} {int(bool(m[0x3a]&128))} {c.p&1}')
 run=subprocess.run(['build/ai-action-candidate-tests','--defender',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 lines=run.stdout.splitlines()
 if len(lines)!=len(cases):raise RuntimeError('Native count mismatch')
 for c,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{c}: {x} != {y}')
 invalid=[(0,23,80,0,0,4,0,1,2,0),(11,23,80,0,0,4,0,1,2,0),(1,23,80,0,0,0,0,1,2,0),(1,23,80,0,0,4,0,0,2,0),(1,23,80,0,0,4,0,1,5,0),(1,23,80,0,0,4,0,1,2,2),(1,23,80,0,0,4,0,1,4,1)]
 rejected=subprocess.run(['build/ai-action-candidate-tests','--defender',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in invalid),capture_output=True,text=True,check=True,timeout=30)
 if rejected.stdout.splitlines()!=['unsupported']*len(invalid):raise RuntimeError('Invalid defender preparation accepted')
 print(f'{len(cases)} defender strength/effect/cost/stamina/weakened/carry results match original; contact comparison pending')
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare area defender preparation, energy and comparison through original8A28."""
import argparse, hashlib, subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from verify_action_strength import PAYLOAD_SHA256

def observe_contact(prg,raw):
 m=ObservedCartridge(prg,raw,None,set());m.registers[6:8]=[26,27]
 m[0x24],m[0x25]=26,27;m[0x23]=m[0x22]|7;m.select=m[0x23]
 c=MPU(memory=m);c.pc,c.sp,c.p=0x89da,0xfd,c.UNUSED
 for step in range(6000):
  if m.bank_at(c.pc)==26 and c.pc==0x8a28:return m,c
  if c.pc<0x8000 or c.pc in (0xcb0f,0xcaae):raise RuntimeError('Unsupported contact path')
  c.step()
 raise RuntimeError('Contact instruction bound')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active']);cases=[];expected=[];codes=set()
 randoms=((0,0),(1,0),(7,255),(127,128),(255,0),(0,255),(247,247),(248,248),(255,255),(128,1))
 for kind in (23,24,32):
  for level in (0,255):
   for defense,subtype in ((2,0),(2,1),(4,0)):
    for variant in (1,2):
     for attack in (1,4,6):
      for stamina in (0,80,65535):
       for first,second in randoms:
        for power in (0,257,65535):
         actor=1+kind%10;raw=bytearray(base);off=0x300+12*actor
         raw[off:off+4]=bytes((kind,stamina&255,stamina>>8,level))
         raw[0x601],raw[0x60b],raw[0x606],raw[0x44e],raw[0x43b],raw[0x43c],raw[0x616]=actor,defense,subtype,variant,attack,0,0
         raw[0x61c:0x61e]=power.to_bytes(2,'little');raw[0xe2],raw[0xe3]=first,second
         m,c=observe_contact(prg,raw);cost=m[0x43f]+256*m[0x440];remaining=m[off+1]+256*m[off+2];base_power=m[0x32]+256*m[0x33]
         cases.append((actor,kind,stamina&255,stamina>>8,level,attack,0,variant,defense,subtype,power,first,second))
         expected.append(f'{remaining} {cost} {m[0x445]} {base_power} {m[0x71]} {m[0x619]} {c.a} {c.y} {m[0x43e]} {m[0x61c]+256*m[0x61d]}');codes.add(c.y)
 run=subprocess.run(['build/ai-action-candidate-tests','--contact',str(a.pack)],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=45)
 lines=run.stdout.splitlines()
 if len(lines)!=len(cases):raise RuntimeError('Native contact count mismatch')
 for c,x,y in zip(cases,lines,expected):
  if x!=y:raise RuntimeError(f'{c}: {x} != {y}')
 if codes!=set(range(4)):raise RuntimeError(f'Incomplete result coverage: {codes}')
 packed=a.pack.read_bytes()
 if packed[:8]==b'CMJSHOTK':packed=b'CMJSHOTJ'+packed[8:21322]
 if packed[:8]!=b'CMJSHOTJ' or len(packed)!=21322:raise RuntimeError('Unexpected private J pack')
 previous=a.pack.with_name('japanese-cycle-v18-area-attack.cmjpack')
 if previous.exists() and packed[:7]+b'I'+packed[8:-36]!=previous.read_bytes():raise RuntimeError('Earlier I content changed')
 import tempfile
 with tempfile.TemporaryDirectory(dir=a.pack.parent) as temp:
  malformed=[packed[:-1],packed+b'\0']
  bad=bytearray(packed);bad[-36+1]=255;malformed.append(bad)
  bad=bytearray(packed);bad[-1]=1;malformed.append(bad)
  for i,value in enumerate(malformed):
   path=Path(temp)/f'bad-{i}.cmjpack';path.write_bytes(value)
   run=subprocess.run(['build/ai-action-candidate-tests','--contact',str(path)],capture_output=True,text=True,timeout=10)
   if not run.returncode:raise RuntimeError('Malformed area content accepted')
 print(f'{len(cases)} complete area defender calculations match original, all four comparison results covered; result dispatch/presentation pending')
if __name__=='__main__':main()

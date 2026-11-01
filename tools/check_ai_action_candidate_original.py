#!/usr/bin/env python3
"""Compare native packed AI candidates against original bank28:8B0B.

Uses synthetic replacement rows supplied in RAM. Category/profile selection and
later action adjustment are deliberately outside this component comparison.
"""
import argparse,hashlib,random,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path)
 p.add_argument('--binary',type=Path,default=Path('build/ai-action-candidate-tests'));a=p.parse_args()
 rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
 rng=random.Random(20261101)
 rows=[(0x01,0x23,0x45,0x67),(0x99,0x99,0x99,0x92),*(tuple(rng.randrange(256) for _ in range(4))for _ in range(30))]
 cases=[(*row,r)for row in rows for r in range(256)]
 run=subprocess.run([str(a.binary.resolve()),'--batch'],input=''.join(' '.join(map(str,c))+'\n'for c in cases),capture_output=True,text=True,check=True,timeout=30)
 outputs=run.stdout.splitlines()
 if len(outputs)!=len(cases):raise RuntimeError('Native count mismatch')
 m=ReferenceCartridge(rom[16:262160]);m.registers[6:8]=[28,29];cpu=MPU(memory=m)
 for case,output in zip(cases,outputs):
  m[0x400:0x404]=case[:4];m[0x3c],m[0x3d],m[0x3e],m[0xe2]=0,4,0,case[4]
  m[0x1fe],m[0x1ff]=255,1;cpu.pc,cpu.sp,cpu.p=0x8b0b,0xfd,cpu.UNUSED
  for _ in range(40):
   if cpu.pc==0x200:break
   cpu.step()
  else:raise RuntimeError('Original instruction bound')
  if int(output)!=cpu.a:raise RuntimeError(f'Candidate mismatch: {case}')
 print(f'{len(cases)} packed AI candidate selections match original; category/subtype flow not covered')
if __name__=='__main__':main()

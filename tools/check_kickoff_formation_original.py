#!/usr/bin/env python3
"""Compare DC07 formation placement for all supported formations/sides."""
import argparse,hashlib,json,random,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256

def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path);p.add_argument('output_dir',type=Path);a=p.parse_args()
 rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];reference=ObservedCartridge(prg,bytes(2048),None,set());reference.registers[6:8]=[26,27]
 on=[reference[i] for i in range(0xdc87,0xdcb3)];off=[reference[i] for i in range(0xdcb3,0xdcdf)]
 rng=random.Random(0x107);native_inputs=[];expected=[]
 for side in (0,11):
  for left in range(4):
   for right in range(4):
    for sample in range(4):
     ram=bytearray(2048);ram[0x5fb]=side;ram[0x2c]=left;ram[0x2e]=right
     positions=[]
     for slot in range(22):
      x,y=rng.randrange(65536),rng.randrange(65536);positions.extend([x,y]);base=0x300+slot*12
      ram[base+5:base+7]=x.to_bytes(2,'little');ram[base+7:base+9]=y.to_bytes(2,'little')
     m=ObservedCartridge(prg,ram,None,set());m.registers[6:8]=[26,27];cpu=MPU(memory=m);cpu.pc=0xdc07;cpu.sp=0xfd;cpu.p=cpu.UNUSED
     m[0x1fe]=255;m[0x1ff]=1
     for steps in range(20000):
      if cpu.pc==0x200:break
      cpu.step()
     else:raise RuntimeError('Original formation instruction bound')
     values=[]
     for slot in range(22):
      base=0x300+slot*12;values.extend([m[base+5]|m[base+6]<<8,m[base+7]|m[base+8]<<8])
     native_inputs.append([side,left,right,*on,*off,*positions]);expected.append(values)
 run=subprocess.run(['build/kickoff-formation-tests','--batch'],input=''.join(' '.join(map(str,row))+'\n' for row in native_inputs),text=True,capture_output=True,check=True)
 actual=[[int(x) for x in line.split()] for line in run.stdout.splitlines()]
 if len(actual)!=len(expected):raise RuntimeError('Formation comparison count mismatch')
 for i,(got,want) in enumerate(zip(actual,expected)):
  if got!=want:raise RuntimeError(f'Formation mismatch case{i}: {[(j,x,y) for j,(x,y) in enumerate(zip(got,want)) if x!=y]}')
 (out/'verification.json').write_text(json.dumps({'cases':len(expected),'players_per_case':22,'scope':'Original DC07 through RTS; all instructions execute. Sixteen formation combinations per side, randomized prior raw words, including keeper aliases. No UI, frame or kickoff flow.'},indent=2)+'\n')
 print(f'{len(expected)} original kickoff formation comparisons passed: {out}')
if __name__=='__main__':main()

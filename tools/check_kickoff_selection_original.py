#!/usr/bin/env python3
"""Compare original DAF5..DB0B kickoff receiver selection with the native function."""
import argparse,hashlib,json,subprocess
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
 out.mkdir(parents=True,exist_ok=False);m=ObservedCartridge(rom[16:262160],bytes(2048),None,set());m.registers[6:8]=[26,27]
 table=[m[i] for i in range(0xdc82,0xdc87)]
 if table!=[5,6,7,9,10]:raise RuntimeError('Native test driver content does not match original table')
 cases=[]
 for side in (0,11):
  for random in range(256):
   m[0x5fb]=side;m[0xe2]=random;cpu=MPU(memory=m);cpu.pc=0xdaf5;cpu.p=cpu.UNUSED
   for steps in range(50):
    if cpu.pc==0xdb0b:break
    cpu.step()
   else:raise RuntimeError('Original kickoff selection instruction bound')
   cases.append((side,random,m[0x5fc]))
 run=subprocess.run(['build/kickoff-selection-tests','--batch'],input=''.join(f'{s} {r}\n' for s,r,_ in cases),text=True,capture_output=True,check=True)
 actual=[int(x) for x in run.stdout.splitlines()]
 if actual!=[x[2] for x in cases]:raise RuntimeError('Kickoff receiver selection mismatch')
 team_cases=[];cpu=MPU(memory=m)
 for team in range(256):
  for random in range(256):
   m[0x2b]=team;m[0xe2]=random;cpu.pc=0xdb4b;cpu.p=cpu.UNUSED
   for steps in range(20):
    if cpu.pc==0xdb61:break
    cpu.step()
   else:raise RuntimeError('Original kickoff side selection instruction bound')
   if m[0x44f]!=m[0x5fb]:raise RuntimeError('Original kickoff side fields differ')
   team_cases.append((team,random,m[0x5fb]))
 run=subprocess.run(['build/kickoff-selection-tests','--side-batch'],input=''.join(f'{s} {r}\n' for s,r,_ in team_cases),text=True,capture_output=True,check=True)
 if [int(x) for x in run.stdout.splitlines()]!=[x[2] for x in team_cases]:raise RuntimeError('Kickoff side selection mismatch')
 (out/'verification.json').write_text(json.dumps({'cases':len(cases),'side_cases':len(team_cases),'table':table,'scope':'Original DAF5..DB0B and DB4B..DB61; both sides/all random bytes. Formation, earlier reset, animation, ball setup and live input are not executed.'},indent=2)+'\n')
 print(f'{len(cases)} receiver and {len(team_cases)} side comparisons match: {out}')
if __name__=='__main__':main()

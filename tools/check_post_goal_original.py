#!/usr/bin/env python3
"""Compare original post-menu goal cleanup to a saved kickoff boundary."""
import argparse, hashlib, json, subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_keeper_entry_original import keeper_checkpoint
from check_keeper_saves_original import post_save_checkpoint
from verify_action_strength import PAYLOAD_SHA256

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','pack','ram','output_dir'):p.add_argument(name,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected side0 shot RAM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);binary=str(Path('build/captain-phase-session').resolve());results=[]
 for stun in (0,1,127,255):
  for timer,penalty,duration in ((0,0,0),(255,79,9),(7,255,255)):
   name=f'stun{stun}-timer{timer}';ram=bytearray(base)
   for addr,value in ((0x5FB,0),(0x43B,0),(0x43C,0),(0x43D,0),(0x43E,0),(0x445,0),(0x612,0),(0x616,0),(0x442,11),(0x5FF,0),(0x62D,0),(0x600,int(stun==0)),(0x38E,stun),(0x389,timer),(0x38A,duration),(0x38B,penalty),(0x28,12),(0x29,7),(0x629,2),(0x635,204),(0x637,124),(0x5FE,233),(0x638,233),(0x61A,255),(0x61B,0)):ram[addr]=value
   ram[0x601:0x606]=bytes([11,0,0,0,0]) if not stun else bytes(5)
   entry=out/(name+'-entry.cmjsession');entry.write_bytes(keeper_checkpoint(ram,59))
   m=ObservedCartridge(rom[16:262160],ram,None,set());m.registers[6:8]=[26,27];m[0x24],m[0x25]=26,27;m[0x23]=m[0x22]|7;m.select=m[0x23]
   cpu=MPU(memory=m);cpu.pc=0x85BF;cpu.sp=0xFD;cpu.p=cpu.UNUSED
   for steps in range(1000):
    if cpu.pc==0x85DD:break
    cpu.step()
   else:raise RuntimeError('Post-goal original bound')
   final=bytes(m.ram[:2048]);wanted=bytearray(post_save_checkpoint(final,60));wanted[361:365]=bytes([1,final[0x28],final[0x29],final[0x629]])
   dest=out/(name+'-native.cmjsession');subprocess.run([binary,str(a.pack.resolve()),str(entry),'--continue-prepared-goal-restart','--capture',str(dest)],check=True,capture_output=True)
   actual=dest.read_bytes()
   if actual!=wanted:raise RuntimeError(f'{name} mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual,wanted)) if x!=y]}')
   results.append({'stun':stun,'timer':timer,'match':True,'instructions':steps})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Prepared post-presentation and post-menu entry85BF to85DD; no calls skipped. Presentation/menu effects are explicit inputs and are not executed.'},indent=2)+'\n')
 print(f'{len(results)} prepared post-goal handoffs match: {out}')
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare full prepared numeric flight with native save/reload at every cell."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from flight_checkpoint import flight_checkpoint
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','projection','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected ordinary side0 shot RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());projection=str(a.projection.resolve())
    def original(ram):
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xE73E;cpu.sp=0xFD;cpu.p=cpu.UNUSED;return m,cpu
    def until(cpu,targets):
        for count in range(2_000_000):
            if cpu.pc in targets:return count
            cpu.step()
        raise RuntimeError('Prepared flight original instruction bound')
    def native(source,operation,dest):
        subprocess.run([binary,pack,str(source),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
    cases=[]
    for name,x,y,dest,zero in [('loose-with-defenders',120,100,221,False),('keeper-bound',160,96,233,True)]:
        ram=bytearray(base);ram[0x634:0x638]=((x<<8)|64).to_bytes(2,'little')+((y<<8)|192).to_bytes(2,'little')
        ram[0x5FE]=((x-48)//8)*12+(y-80)//8;ram[0x638]=dest;ram[0x61A]=255;ram[0x61B]=0;ram[0x617]=0;ram[0xE2]=1;ram[0xE3]=0
        if zero:ram[0x600:0x606]=bytes(6);ram[0x612]=0
        # Freeze candidates in the first visited cell; positions are explicit input.
        probe,cp=original(ram);until(cp,{0xE773});cp.pc=0xE778;until(cp,{0xE7B4,0xE7CC})
        for slot in range(12,22):
            r=0x300+12*slot;ram[r+6]=probe[0x635];ram[r+8]=probe[0x637];ram[r+10]=int(slot==12)
        m,cpu=original(ram);until(cpu,{0xE773})
        source=out/(name+'-entry.cmjsession');source.write_bytes(flight_checkpoint(ram,46,x<128));(out/(name+'-entry.bin')).write_bytes(ram)
        ready=out/(name+'-ready.cmjsession');native(source,['--begin-ball-flight',projection,'64','192'],ready)
        if ready.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),47,x<128):raise RuntimeError('Setup mismatch')
        boundaries=[]
        for cell in range(100):
            cpu.pc=0xE778  # each frame wait acknowledged explicitly; no task/NMI update
            until(cpu,{0xE7B4,0xE7CC});phase=48 if cpu.pc==0xE7B4 else 49
            stepped=out/(name+f'-{cell}-step.cmjsession');native(ready,['--advance-ball-cell'],stepped)
            if stepped.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),phase,x<128):raise RuntimeError(f'{name}step{cell} mismatch')
            boundaries.append({'cell':m[0x5FE],'phase':phase,'selected':m[0x600]})
            if phase==49:break
            until(cpu,{0xE773})  # execute complete original800F/87E1 callback and return
            ready=out/(name+f'-{cell}-continued.cmjsession');native(stepped,['--resolve-ball-cell'],ready)
            if ready.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),47,x<128):raise RuntimeError(f'{name}callback{cell} mismatch')
        else:raise RuntimeError('Prepared flight did not arrive within100cell callbacks')
        (out/(name+'-final.bin')).write_bytes(m.ram[:2048]);cases.append({'case':name,'boundaries':boundaries,'final_selected':list(m.ram[0x601:0x601+m[0x600]]),'match':True})
    (out/'verification.json').write_text(json.dumps({'cases':cases,'scope':'Full numeric destination flight with original cell selection and native save/reload every step. Explicit frozen frame acknowledgements; no live NMI/task/input updates or arrival encounter/keeper/outcome callback.'},indent=2)+'\n')
    print(f'{len(cases)} complete prepared numeric flights match at every saved cell/callback: {out}')


if __name__=='__main__':main()

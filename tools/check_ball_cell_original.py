#!/usr/bin/env python3
"""Compare original87E1 flight defender accumulation with saved native callback."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_attack_clock_original import checkpoint
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected ordinary side0 flight RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must be outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve())
    def encode(ram,phase):
        b=bytearray(checkpoint(ram,phase));b[324:330]=bytes(6)
        b[339:344]=bytes([1,ram[0x62B],ram[0x638],ram[0x61A],1])
        b[344:355]=bytes([1])+ram[0x634:0x638]+ram[0x639:0x63D]+bytes([ram[0x62C],ram[0x61B]])
        return bytes(b)
    results=[]
    for name,prior,first,second,stun in [('empty',0,1,0,False),('retained3',3,1,0,False),
            ('retained4',4,1,0,False),('full',5,1,0,False),('sample255',0,0,0,False),
            ('sample254',0,0,1,False),('stunned',0,1,0,True)]:
        ram=bytearray(base);ram[0x600]=prior;ram[0x601:0x606]=bytes([12+i if i<prior else 0 for i in range(5)])
        ram[0xE2],ram[0xE3]=first,second
        for slot in range(12,22):
            record=0x300+slot*12;ram[record+6]=ram[0x635];ram[record+8]=ram[0x637];ram[record+10]=int(stun and slot in (12,14))
        entry=out/(name+'-entry.cmjsession');entry.write_bytes(encode(ram,48));(out/(name+'-entry.bin')).write_bytes(ram)
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0x87E1;cpu.sp=0xFD;cpu.p=cpu.UNUSED;m[0x1FE],m[0x1FF]=255,1
        for step in range(10000):
            if cpu.pc==0x200:break
            cpu.step()
        else:raise RuntimeError('Original cell callback instruction bound')
        final=out/(name+'-native.cmjsession');subprocess.run([binary,pack,str(entry),'--resolve-ball-cell','--capture',str(final)],capture_output=True,check=True,timeout=30)
        expected=encode(bytes(m.ram[:2048]),47);actual=final.read_bytes()
        if expected!=actual:raise RuntimeError(f'{name}: mismatch{[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
        if subprocess.run([binary,pack,str(final),'--resolve-ball-cell'],capture_output=True).returncode==0:raise RuntimeError('Repeated cell callback accepted')
        (out/(name+'-final.bin')).write_bytes(m.ram[:2048]);results.append({'case':name,'selected':list(m.ram[0x601:0x601+m[0x600]]),'instructions':step,'match':True})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Original87E1 complete numeric callback, no skipped calls/frames. Prior prefix including duplicates retained; capacity and borrow sample verified. No live input or arrival encounter claim.'},indent=2)+'\n')
    print(f'{len(results)} original/native accumulated flight defender cases match: {out}')


if __name__=='__main__':main()

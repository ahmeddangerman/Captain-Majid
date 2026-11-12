#!/usr/bin/env python3
"""Compare constructed pass-defender candidates with the original CPU (private ROM/data)."""
import argparse
import hashlib
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original(prg, raw):
    memory=ObservedCartridge(prg,raw,None,set());memory.registers[6:8]=[28,29]
    memory[0x24],memory[0x25]=28,29;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p,cpu.a=0x863f,0x50,cpu.UNUSED,17
    for _ in range(10000):
        if memory.bank_at(cpu.pc)==28 and cpu.pc==0x86a2:
            pointer=memory[0x3c]|(memory[0x3d]<<8);offset=pointer-0xb8ae
            if offset<0 or offset%48:raise RuntimeError('Original pass-defense row outside table')
            return memory[0x3e],offset//48,cpu.a
        if cpu.pc<0x8000 or cpu.pc in (0xcaae,0xcb0f):raise RuntimeError(f'Unexpected original boundary {cpu.pc:04x}')
        cpu.step()
    raise RuntimeError('Original pass-defense candidate exceeded instruction limit')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','pack','exe'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();rom=args.rom.read_bytes();base=args.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:parser.error('Expected verified ROM and 2048-byte private RAM')
    samples=[];expected=[]
    for kind in (1,7,23,35,55,70,117):
        for flags,alternate in ((0,35),(128,23)):
            for x in (80,95,108,120,159,200):
                for y in (84,108,124,140,172):
                    for random in range(8):
                        raw=bytearray(base);raw[0x300+12*17:0x303+12*17]=bytes((kind,flags,alternate));raw[0x635],raw[0x637],raw[0xe2],raw[0x621]=x,y,random,0
                        try: answer=original(rom[16:262160],raw)
                        except RuntimeError as error:
                            if 'exceeded instruction limit' in str(error):continue
                            raise
                        samples.append((kind,flags,alternate,x,y,random));expected.append(answer)
    run=subprocess.run([str(args.exe.resolve()),'--pass-defender',str(args.pack.resolve())],input=''.join(' '.join(map(str,row))+'\n' for row in samples),text=True,capture_output=True,check=True)
    actual=run.stdout.splitlines()
    if len(actual)!=len(expected):raise RuntimeError('Native pass-defense output length differs')
    matched=rejected=0
    for sample,want,line in zip(samples,expected,actual):
        if want[0]>44:
            if line!='unsupported':raise RuntimeError(f'Out-of-row category accepted for {sample}: {line}')
            rejected+=1;continue
        if line=='unsupported' or tuple(map(int,line.split()))!=want:raise RuntimeError(f'Candidate differs for {sample}: original {want}, native {line}')
        matched+=1
    print(f'Original/native pass-defense candidate matched {matched} constructed cases; {rejected} out-of-row cases rejected. Scheduling, special moves and final defender action are not covered')


if __name__=='__main__':main()

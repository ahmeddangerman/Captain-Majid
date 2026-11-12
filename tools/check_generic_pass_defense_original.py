#!/usr/bin/env python3
"""Compare first-fixture generic pass-defense decisions with original CPU.

Roster flags, positions, random bytes and ball variant are constructed inputs.
Unsupported action branches remain explicit; live presentation is not run.
"""
import argparse
import hashlib
import subprocess
from collections import Counter
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original(prg,raw,actor):
    memory=ObservedCartridge(prg,raw,None,set());memory.registers[6:8]=[28,29]
    memory[0x24],memory[0x25]=28,29;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    memory[0x1fe],memory[0x1ff]=0x28,0x86
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p,cpu.a=0x863f,0xfd,cpu.UNUSED,actor
    for _ in range(20000):
        if memory.bank_at(cpu.pc)==28 and cpu.pc==0x8629:return memory[0x43d],memory[0x43e]
        if cpu.pc<0x8000 or cpu.pc in (0xcaae,0xcb0f):raise RuntimeError(f'Unexpected original boundary {cpu.pc:04x}')
        memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
    raise RuntimeError('Original generic defense exceeded instruction bound')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','pack','exe'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:
        p.error('Expected verified Japanese ROM and private 2048-byte RAM')
    if base[0x2b]!=3 or any(base[0x300+12*i] for i in range(12,22)):
        p.error('Expected first-fixture generic opponent records')
    samples=[];wanted=[];out_of_scope=Counter()
    for actor in range(12,22):
        for flags,alternate in ((0,0),(128,23)):
            for x in (80,108,124,159,200):
                for y in (84,108,124,172):
                    for random in range(8):
                        for variant in (0,1):
                            raw=bytearray(base);off=0x300+12*actor;raw[off+1:off+3]=bytes((flags,alternate))
                            raw[0x635],raw[0x637],raw[0xe2],raw[0x621],raw[0x44e]=x,y,random,0,variant
                            teammates=[raw[0x300+12*i] for i in range(1,11)]
                            try:result=original(rom[16:262160],raw,actor)
                            except RuntimeError as error:
                                if not str(error).startswith('Unexpected original boundary '):raise
                                out_of_scope[str(error)]+=1;continue
                            samples.append((actor,flags,alternate,x,y,random,variant,*teammates));wanted.append(result)
    run=subprocess.run([str(a.exe.resolve()),'--pass-defender-generic',str(a.pack.resolve())],
        input=''.join(' '.join(map(str,row))+'\n' for row in samples),text=True,capture_output=True,check=True)
    lines=run.stdout.splitlines()
    if len(lines)!=len(wanted):raise RuntimeError('Native output length differs')
    counts=Counter()
    for sample,expected,line in zip(samples,wanted,lines):
        if line=='unsupported':counts['guarded']+=1;continue
        if line not in ('0','1','2') or expected!=(int(line),0):raise RuntimeError(f'Generic choice mismatch {sample}: original {expected}, native {line}')
        counts['matched-action-'+line]+=1
    if not counts['matched-action-0'] or not counts['matched-action-1'] or not counts['matched-action-2']:raise RuntimeError('Missing supported ordinary choices')
    print('Generic original/native pass-defense survey:',dict(counts),'out of scope',dict(out_of_scope))


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare conservatively supported final pass-defense choices with original CPU."""
import argparse
import hashlib
import subprocess
from collections import Counter
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original(prg,raw):
    memory=ObservedCartridge(prg,raw,None,set());memory.registers[6:8]=[28,29]
    memory[0x24],memory[0x25]=28,29;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    memory[0x1fe],memory[0x1ff]=0x28,0x86
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p,cpu.a=0x863f,0xfd,cpu.UNUSED,17
    for _ in range(20000):
        if memory.bank_at(cpu.pc)==28 and cpu.pc==0x8629:return memory[0x43d],memory[0x43e]
        if cpu.pc<0x8000 or cpu.pc in (0xcaae,0xcb0f):raise RuntimeError(f'Unexpected original boundary {cpu.pc:04x}')
        cpu.step()
    raise RuntimeError('Original pass defense exceeded instruction bound')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','pack','exe'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:p.error('Expected verified ROM and private RAM')
    samples=[];wanted=[];skipped=Counter()
    for kind in (1,7,23,35,55,70,117):
        for flags,alternate in ((0,35),(128,23)):
            for x in (80,108,159,200):
                for y in (84,108,124,172):
                    for random in range(8):
                        for variant in (0,1):
                            raw=bytearray(base);raw[0x300+12*17:0x303+12*17]=bytes((kind,flags,alternate));raw[0x635],raw[0x637],raw[0xe2],raw[0x621],raw[0x44e]=x,y,random,0,variant
                            teams=[raw[0x300+12*i] for i in range(1,11)]
                            try:result=original(rom[16:262160],raw)
                            except RuntimeError as error:
                                if not str(error).startswith('Unexpected original boundary '):
                                    raise
                                skipped[str(error)] += 1
                                continue
                            samples.append((kind,flags,alternate,x,y,random,variant,*teams));wanted.append(result)
    run=subprocess.run([str(a.exe.resolve()),'--pass-defender-final',str(a.pack.resolve())],input=''.join(' '.join(map(str,row))+'\n' for row in samples),text=True,capture_output=True,check=True)
    lines=run.stdout.splitlines()
    if len(lines)!=len(wanted):raise RuntimeError('Native choice output length differs')
    counts=Counter()
    for sample,expected,line in zip(samples,wanted,lines):
        if line=='unsupported':counts['unsupported']+=1;continue
        if line not in ('0','1') or expected!=(int(line),0):raise RuntimeError(f'Incorrect supported final defense {sample}: original {expected}, native {line}')
        counts['matched']+=1
        counts['matched-action-'+line]+=1
        if sample[0]==23:counts['matched-type23']+=1
    if not counts['matched'] or not counts['matched-type23']:raise RuntimeError('No supported cases for the playtest defender type')
    print('Final original/native pass-defense survey:',dict(counts),'skipped original boundaries',dict(skipped),'constructed cases; unsupported cases remain guarded')


if __name__=='__main__':main()

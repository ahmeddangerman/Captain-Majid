#!/usr/bin/env python3
"""Compare first-fixture route1 goal-area AI candidates and pass choices.

Roster variants, positions, random bytes and ball variants are constructed;
unsupported candidates remain guarded and presentation is not simulated.
"""
import argparse
import hashlib
import subprocess
from collections import Counter
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original(prg,raw,all_choices=False,through_preparation=False):
    m=ObservedCartridge(prg,raw,None,set());m.registers[6:8]=[26,27]
    m[0x24],m[0x25]=26,27;m[0x23]=m[0x22]|7;m.select=m[0x23]
    c=MPU(memory=m);c.pc,c.sp,c.p=0x8298,0x50,c.UNUSED;candidate=None
    for _ in range(20000):
        if m.bank_at(c.pc)==28 and c.pc==0x87b0:
            pointer=m[0x3c]|m[0x3d]<<8;offset=pointer-0xb12e
            if offset<0 or offset%48:raise RuntimeError('Unexpected original candidate row')
            candidate=(m[0x3e],offset//48,c.a)
            if c.a!=2 and not all_choices:return candidate,None
        if m.bank_at(c.pc)==26 and c.pc==(0x82a0 if through_preparation else 0x829d):return candidate,bytes(m.ram[:2048])
        if c.pc<0x8000 or c.pc in (0xcaae,0xcb0f):raise RuntimeError(f'Unexpected original scheduler at {c.pc:04x}')
        m.instruction=(m.bank_at(c.pc),c.pc);c.step()
    raise RuntimeError('Original goal-area AI instruction limit')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','pack','exe'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:
        p.error('Expected verified Japanese ROM and private 2048-byte RAM')
    if base[0x2b]!=3 or any(base[0x300+12*i] for i in range(12,22)):
        p.error('Expected first-fixture generic opponent records')
    cases=[];wanted=[];counts=Counter();prg=rom[16:262160]
    for actor in range(12,22):
        for flags,alternate in ((0,0),(128,35)):
            for x,y in ((51,124),(204,131),(75,100)):
                for first in range(8):
                    for second in (0,127,255):
                        for variant in (1,2):
                            for pattern in (0,1):
                                r=bytearray(base);r[0x441],r[0x5fb],r[0x621],r[0x44e],r[0x600]=actor,11,1,variant,0
                                off=0x300+12*actor;r[off:off+3]=bytes((0,flags,alternate))
                                r[0x635],r[0x637],r[0xe2],r[0xe3]=x,y,first,second;r[0x5fc]=r[0x638]=250
                                positions=[]
                                for player in range(12,22):
                                    px,py=(80 if pattern==0 else 60+10*(player-12)),80+8*(player-12)
                                    r[0x306+12*player],r[0x308+12*player]=px,py;positions.extend((px,py))
                                candidate,after=original(prg,r);suffix='unsupported'
                                if after is not None:
                                    action,subtype=after[0x43b],after[0x43c]
                                    if action not in (1,4) or subtype:raise RuntimeError('Unexpected original pass-candidate result')
                                    target=after[0x5fc] if action==1 else -1;cell=after[0x638] if action==1 else -1
                                    suffix=f'{action} {subtype} {after[0xe2]} {target} {cell}';counts[f'action{action}']+=1
                                else:counts['guarded']+=1
                                cases.append((flags,alternate,actor,x,y,first,second,variant,*positions))
                                wanted.append(' '.join(map(str,candidate))+' '+suffix)
    run=subprocess.run([str(a.exe.resolve()),'--generic-goal-area',str(a.pack.resolve())],
        input=''.join(' '.join(map(str,row))+'\n' for row in cases),capture_output=True,text=True,check=True,timeout=60)
    lines=run.stdout.splitlines()
    if len(lines)!=len(wanted):raise RuntimeError('Native goal-area result count differs')
    for case,want,got in zip(cases,wanted,lines):
        if want!=got:raise RuntimeError(f'Goal-area mismatch {case}: original {want}, native {got}')
    print('Goal-area original/native survey:',dict(counts),'total',len(cases))

if __name__=='__main__':main()

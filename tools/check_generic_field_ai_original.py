#!/usr/bin/env python3
"""Compare first-fixture generic field-AI candidates and pass plans to original CPU.

Positions, alternate flags and random bytes are constructed. This tests numeric
choices and pass plans only, with unsupported non-pass candidates retained.
"""
import argparse
import hashlib
import subprocess
from collections import Counter
from pathlib import Path
from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','ram','exe'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(base)!=2048:
        p.error('Expected verified Japanese ROM and private internal RAM')
    if base[0x2b]!=3 or any(base[0x300+12*i] for i in range(12,22)):
        p.error('Expected first-fixture generic opponent records')
    prg=rom[16:262160];cases=[];wanted=[];counts=Counter()
    for actor in range(12,22):
        for flags,alt in ((0,0),(128,35)):
            for x in (80,112,160,200):
                for y in (84,124,172):
                    for first in range(8):
                        for second in (0,127,255):
                            for pattern in (0,1):
                                raw=bytearray(base);raw[0x441],raw[0x5fb],raw[0x621],raw[0x44e]=actor,11,0,0
                                off=0x300+12*actor;raw[off:off+3]=bytes((0,flags,alt))
                                raw[0x635],raw[0x637],raw[0xe2],raw[0xe3]=x,y,first,second
                                raw[0x5fc]=raw[0x638]=250;positions=[]
                                for player in range(12,22):
                                    px,py=(200 if pattern==0 else 60+10*(player-12)),80+8*(player-12)
                                    raw[0x306+12*player],raw[0x308+12*player]=px,py;positions.extend((px,py))
                                candidate=original_candidate(prg,raw);suffix='unsupported'
                                if candidate[2]==2:
                                    final=original_choice(prg,raw)
                                    if final[0x43b]==2 and final[0x43c]==0 and final[0x5fc]==250 and final[0x638]==250:
                                        suffix=f'2 0 {final[0xe2]} -1 -1';counts['fallback']+=1
                                    elif final[0x43b]==1 and final[0x43c]==0 and 12<=final[0x5fc]<=21 and final[0x638]<240:
                                        suffix=f'1 0 {final[0xe2]} {final[0x5fc]} {final[0x638]}';counts['pass']+=1
                                    else:counts['guarded-candidate2']+=1
                                elif candidate[2]==1:
                                    final=original_choice(prg,raw)
                                    if (final[0x43b],final[0x43c],final[0xe2])!=(0,0,first):raise RuntimeError('Unexpected generic ordinary shot')
                                    suffix=f'0 0 {first} -1 -1';counts['shot']+=1
                                elif candidate[2]==3:
                                    final=original_choice(prg,raw)
                                    if (final[0x43b],final[0x43c],final[0xe2])!=(2,0,first):raise RuntimeError('Unexpected generic action2')
                                    suffix=f'2 0 {first} -1 -1';counts['action2']+=1
                                else:counts['non-pass-candidate']+=1
                                cases.append((flags,alt,actor,x,y,first,second,*positions));wanted.append(' '.join(map(str,candidate))+' '+suffix)
    result=subprocess.run([str(a.exe.resolve()),'--generic-field-plan',str(a.pack.resolve())],
        input=''.join(' '.join(map(str,row))+'\n' for row in cases),capture_output=True,text=True,check=True,timeout=60)
    actual=result.stdout.splitlines()
    if len(actual)!=len(wanted):raise RuntimeError('Native result count differs')
    for case,expected,got in zip(cases,wanted,actual):
        if got!=expected:raise RuntimeError(f'Generic field AI mismatch {case}: original {expected}, native {got}')
    if not counts['pass'] or not counts['fallback'] or not counts['action2'] or not counts['shot']:raise RuntimeError('Missing required plan coverage')
    print('Generic field AI original/native survey:',dict(counts),'total',len(cases))

if __name__=='__main__':main()

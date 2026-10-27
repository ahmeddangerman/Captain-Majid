#!/usr/bin/env python3
"""Compare original result4 override/reset and immediate restart selection, with no skipped calls."""
import argparse
import hashlib
import itertools
import random
import subprocess
import tempfile
from pathlib import Path
from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def original(prg, fields):
    code,count,slot,action,side,x,y,first,*state=fields
    mem=ReferenceCartridge(prg);cpu=MPU(memory=mem)
    for a,v in ((0x612,code),(0x600,count),(0x442,slot),(0x43D,action),(0x5FB,side),
                (0x635,x),(0x637,y),(0xE2,first),(0x44E,77)):
        mem[a]=v
    for player in range(22):
        record=0x300+player*12
        mem[record+10]=state[player]
        mem[record+7]=state[22 if player==0 else 23] if player in (0,11) else 160+player
    cpu.pc,cpu.sp,cpu.p=0x8E33,0xFD,cpu.UNUSED
    mem[0x1FE:0x200]=[255,1]
    triggered=False
    for _ in range(4000):
        if cpu.pc==0x200:break
        triggered=triggered or cpu.pc==0xCF4F
        cpu.step()
    else:raise RuntimeError(f'Override did not return ${cpu.pc:04X}')
    route=2
    if mem[0x612]==4:
        cpu.pc=0x9366
        for _ in range(100):
            if cpu.pc in (0x9379,0x94CF):
                route=1 if cpu.pc==0x94CF else 0
                break
            cpu.step()
        else:raise RuntimeError('Restart selection did not finish')
    for player in range(1,22):
        if player!=11 and mem[0x300+12*player+7]!=160+player:
            raise AssertionError('Reset changed non-keeper record+7')
    if mem.bank_writes:raise AssertionError('Unexpected bank writes')
    return (mem[0x612],int(triggered),*[mem[0x300+12*i+10] for i in range(22)],
            mem[0x307],mem[0x38B],route,mem[0x44E])


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('rom',type=Path);args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('ROM payload mismatch')
    prg=rom[16:262160]
    positions=((79,95),(79,96),(79,159),(79,160),(80,100),(175,100),(176,100),(120,100))
    cases=[];rng=random.Random(0x8E33)
    for slot,action,code,side,count,(x,y),first in itertools.product((0,11,3,14),range(7),range(5),(0,11),(0,1),positions,(0,15,16,63,64,255)):
        cases.append([code,count,slot,action,side,x,y,first,*[rng.randrange(256) for _ in range(24)]])
    for _ in range(1024):
        cases.append([rng.randrange(5),rng.randrange(11),rng.randrange(22),rng.randrange(7),rng.choice((0,11)),*[rng.randrange(256) for _ in range(3)],*[rng.randrange(256) for _ in range(24)]])
    expected=[original(prg,c) for c in cases]
    root=Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-field-interrupt-') as temp:
        binary=Path(temp)/'native'
        subprocess.run(['c++','-std=c++17','-O2','-Wall','-Wextra','-Wpedantic','-I'+str(root/'engine/include'),str(root/'engine/tests/field_interruption_trace_driver.cpp'),str(root/'engine/src/field_interruption.cpp'),'-o',str(binary)],check=True,timeout=60)
        data=str(len(cases))+'\n'+'\n'.join(' '.join(map(str,c)) for c in cases)+'\n'
        run=subprocess.run([str(binary)],input=data,text=True,capture_output=True,check=True,timeout=60)
    actual=[tuple(map(int,line.split())) for line in run.stdout.splitlines()]
    if len(actual)!=len(expected):raise AssertionError('Output count mismatch')
    for i,(want,got) in enumerate(zip(expected,actual)):
        if want!=got:raise AssertionError(f'Case{i} {cases[i]}: ROM={want}, native={got}')
    print(f'Field interruption matched {len(cases)} cases: {sum(r[1] for r in expected)} resets, {sum(r[-2]==0 for r in expected)} field routes, {sum(r[-2]==1 for r in expected)} goal-area routes; no calls skipped.')


if __name__=='__main__':main()

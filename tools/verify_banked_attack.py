#!/usr/bin/env python3
"""Trace original $8F72 through real PRG bank calls and compare native attack preparation."""
import argparse
import hashlib
import random
import subprocess
import tempfile
from pathlib import Path
from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256
from verify_field_defender_preparation import cpu_for, word


def original(prg, case, team):
    slot, typ, stamina, level, action, situation, subtype, side, defenders, first, second = case
    mem = ReferenceCartridge(prg)
    cpu = MPU(memory=mem)
    mem[0x24], mem[0x25] = 26, 27
    mem[0x38:0x3A] = [0,5]
    mem[0x500:0x500+len(team)] = team
    record = 0x300 + slot * 12
    mem[record:record+4] = [typ,stamina & 255,stamina >> 8,level]
    for a,v in ((0x441,slot),(0x43B,action),(0x44E,situation),(0x43C,subtype),
                (0x5FB,side),(0x600,defenders),(0xE2,first),(0xE3,second)):
        mem[a]=v
    mem[0x1FE:0x200]=[255,1]
    cpu.pc,cpu.sp,cpu.p=0x8F72,0xFD,cpu.UNUSED
    selector=index=base=effect=cost=flags=energy=carry=multiplier=None
    for _ in range(5000):
        bank=mem.bank_at(cpu.pc) if cpu.pc>=0x8000 else None
        if bank==28 and cpu.pc==0x81C2: selector=cpu.x
        if bank==28 and cpu.pc==0x8284: index=cpu.a
        if bank==26 and cpu.pc==0x8F7A:
            base,effect,cost,flags=word(mem,0x32),mem[0x444],word(mem,0x43F),mem[0x443]
        if bank==26 and cpu.pc==0x8F97: cost=word(mem,0x43F)
        if bank==26 and cpu.pc==0x8F9A:
            carry=cpu.p & 1
            energy=(word(mem,record+1),word(mem,0x43F),int(bool(mem[0x3A]&128)),carry)
        if bank==26 and cpu.pc==0x8FDD: multiplier=word(mem,0x67)
        if cpu.pc==0x200:
            if None in (selector,index,base,effect,cost,flags,carry,multiplier) or energy is None:
                raise RuntimeError('Missing trace checkpoint')
            if mem.bank_writes != [(6,28),(7,29),(6,26),(7,27)]:
                raise RuntimeError(f'Unexpected bank call/restore: {mem.bank_writes}')
            result=(selector,index,base,effect,cost,*energy,flags,word(mem,0x32),
                    multiplier,word(mem,0x61C),mem[0x43C])
            return result,carry
        cpu.step()
    raise RuntimeError(f'Trace failed to return at ${cpu.pc:04X}')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('ROM does not match verified Japanese reference')
    prg=rom[16:262160]
    ram=cpu_for(prg).memory
    ordinary=[ram[0x95D6+4*i] for i in range(35)]
    alternate=[ram[0x9662+12*i] for i in range(221)]
    skills=[ram[0x9FCE+24*i:0x9FCE+24*i+31] for i in range(256)]
    slots=ram[0x818E:0x8199];team=[0,0,1,2,3,4]
    selectors=ram[0x8206:0x8215]
    pointers=[word(ram,0x9460+2*i) for i in range(6)]
    moves=[ram[a:b] for a,b in zip(pointers,pointers[1:])]
    if any(len(m)%4 for m in moves):raise RuntimeError('Unaligned move table')
    content=[*ordinary,len(alternate),*alternate,len(skills)]
    for row in skills:content.extend(row)
    content.extend([*slots,len(team),*team,*selectors])
    for row in moves:content.extend([len(row)//4,*row])
    content.extend(ram[0x9E4E:0x9F0E])
    options=[(a,s,t) for a in range(5) for s in range(3) for t in range(len(moves[a])//4)
             if selectors[3*a+(0 if a==0 and t>=3 else s)]!=255]
    rng=random.Random(0x8F72)
    cases=[]
    # Sweep all usable action/move/situation combinations on both sides,
    # with exact exhaustion, weak, zero and high-word stamina boundaries.
    for side in (0,11):
        for stamina in (0,79,80,128,256,1000,32768):
            for a,s,t in options:
                cases.append((3 if side==0 else 14,rng.choice((1,32,33)),stamina,
                              rng.choice((0,127,128,255)),a,s,t,side,rng.randrange(2),100,100))
    for _ in range(2048):
        side=rng.choice((0,11));a,s,t=rng.choice(options)
        cases.append((3 if side==0 else 14,rng.randrange(1,35) if side==0 else rng.randrange(35),
                      rng.randrange(65536),rng.randrange(256),a,s,t|rng.choice((0,128)),
                      side,rng.randrange(2),rng.randrange(256),rng.randrange(256)))
    lines=[];expected=[]
    for case in cases:
        result,carry=original(prg,case,team)
        slot,typ,stamina,level,a,s,t,side,n,first,second=case
        lines.append([slot,typ,stamina&255,stamina>>8,level,a,s,t,side,n,first,second,
                      carry if side else 1])
        expected.append(result)
    root=Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-attack-trace-') as temp:
        binary=Path(temp)/'native'
        sources=['engine/tests/attack_trace_driver.cpp','engine/src/prepared_attack.cpp',
                 'engine/src/field_player_profile.cpp','engine/src/field_defender_preparation.cpp',
                 'engine/src/energy.cpp','engine/src/shot_power.cpp','engine/src/defender_contact.cpp',
                 'engine/src/action_strength.cpp']
        subprocess.run(['c++','-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        '-I'+str(root/'engine/include'),*[str(root/s) for s in sources],'-o',str(binary)],
                       check=True,timeout=60)
        data=' '.join(map(str,content))+'\n'+str(len(lines))+'\n'
        data+='\n'.join(' '.join(map(str,line)) for line in lines)+'\n'
        run=subprocess.run([str(binary)],input=data,text=True,capture_output=True,check=True,timeout=60)
    actual=[tuple(map(int,line.split())) for line in run.stdout.splitlines()]
    if len(actual)!=len(expected):raise AssertionError('Output count mismatch')
    for i,(want,got) in enumerate(zip(expected,actual)):
        if want!=got:raise AssertionError(f'Case{i} {cases[i]}: ROM={want}, native={got}')
    print(f'Banked attacker trace: {len(cases)} cases, all14 outputs match; real loader/energy/power, bank restoration verified.')


if __name__=='__main__':main()

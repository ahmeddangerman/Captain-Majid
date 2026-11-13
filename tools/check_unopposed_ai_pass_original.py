#!/usr/bin/env python3
"""Trace ordinary unopposed opponent passes from an original cell-change decision."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def observe(prg, raw, expected_action=1, stop_at_arrival=False, stop_at_second_arrival=False, stop_at_area=False):
    memory=ObservedCartridge(prg,raw,None,{0x43d})
    memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xe19b,0x50,cpu.UNUSED
    checkpoints={};targets={(31,0xe1b2):'choice',(31,0xe1cb):'prepared',(26,0x80ed):'preclock',
        (30,0xc612):'handoff',(30,0xde64):'target',(31,0xe773):'flight',
        (31,0xe7cc):'arrival',(31,0xe0df):'control',(31,0xe145):'ready'}
    if expected_action==3:targets.update({(31,0xe01d):'swap-before',(31,0xe023):'swap-after',(31,0xe073):'return-target',
        (26,0x88a2):'return-defender-prepared',(26,0x88a8):'return-defender-compared',
        (26,0x8877):'return-defender-clocked',(26,0x888a):'return-defender-cleanup'})
    for _ in range(2_000_000):
        bank=memory.bank_at(cpu.pc);name=targets.get((bank,cpu.pc))
        if stop_at_area and 'return-arrival' in checkpoints and bank==26 and cpu.pc==0x8b4a:
            checkpoints['return-area']=bytes(memory.ram[:2048]);return 'return-area',checkpoints
        if expected_action==3 and 'arrival' in checkpoints:
            if name=='flight':name='return-flight'
            elif name=='arrival':name='return-arrival'
        if name and name not in checkpoints:
            checkpoints[name]=bytes(memory.ram[:2048])
            if name=='choice' and memory[0x43b]!=expected_action:return 'other-action',checkpoints
            if name=='arrival' and memory[0x600]:return 'defended',checkpoints
            if name=='arrival' and stop_at_arrival:return 'arrival',checkpoints
            if name=='return-arrival' and stop_at_second_arrival:
                return ('return-defended' if memory[0x600] else 'return-arrival'),checkpoints
            if name=='ready':
                if memory.writes:return 'metadata-changed',checkpoints
                return 'ready',checkpoints
        if cpu.pc<0x8000:raise RuntimeError(f'Original pass left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=0
            if memory[0x498]:memory[0x498]-=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Original opponent pass instruction limit')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready-dir','pack','distance','velocity','exe','private-temp'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--original-only',action='store_true')
    args=parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Temporary data must remain private')
    prg=rom[16:262160];counts=Counter()
    with tempfile.TemporaryDirectory(prefix='unopposed-ai-pass-',dir=args.private_temp) as temporary:
        session,records=(Path(temporary)/name for name in ('source.cmjsession','records.bin'))
        for fixture in range(4):
            ready=bytearray((args.ready_dir/f'{fixture}-ready.bin').read_bytes());ready[0x614]=0;ready[0x617]=0
            report,decision=trace_recovery(prg,ready,control_input=(0,0))
            if report['boundary']!='ai_cell_decision':raise RuntimeError('Expected original cell decision')
            for x in (80,112,160,200):
                for elapsed in (0,1,2):
                    for retained in (0,3,6):
                        base=bytearray(decision);owner=base[0x441];cell=((x-48)//8)*12+5
                        base[0x635],base[0x637],base[0x5fe],base[0x617],base[0x5ff],base[0x43d]=x,124,cell,cell,elapsed,retained
                        base[0x306+12*owner],base[0x308+12*owner]=x,124
                        found=False
                        for seed in range(1,200):
                            value=seed;value^=(value<<13)&0xffffffff;value^=value>>17;value^=(value<<5)&0xffffffff
                            original=bytearray(base);original[0xe2],original[0xe3]=value&255,(value>>8)&255
                            if original_candidate(prg,original)[2]!=2:continue
                            outcome,states=observe(prg,original)
                            if outcome=='other-action':continue
                            counts[outcome]+=1;found=True
                            if outcome!='ready':break
                            if not args.original_only:
                                session.write_bytes(export_session(base,12))
                                names=('choice','prepared','preclock','handoff','target','flight','arrival','control','ready')
                                records.write_bytes(b''.join(states[name] for name in names))
                                for name in names:
                                    (Path(temporary)/(name+'.cmjsession')).write_bytes(export_session(states[name],1 if name=='control' else 9 if name=='ready' else 0))
                                result=subprocess.run([str(args.exe.resolve()),'--compare-unopposed-ai-pass',str(session),str(records),str(args.pack.resolve()),str(args.distance.resolve()),str(args.velocity.resolve()),str(seed),temporary],capture_output=True,text=True)
                                if result.returncode:raise RuntimeError(f'fixture={fixture}, x={x}, elapsed={elapsed}, retained={retained}: {result.stderr}')
                            break
                        if not found:counts['no-pass']+=1
    print(f'Constructed original unopposed opponent pass survey: {dict(sorted(counts.items()))}')


if __name__=='__main__':main()

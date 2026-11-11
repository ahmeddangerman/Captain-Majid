#!/usr/bin/env python3
"""Compare constructed empty-target pass handoffs and bounded races with the original CPU."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def observe(prg, ready, home_x):
    _, raw = trace_recovery(prg, ready, control_input=(64, 0))
    memory = ObservedCartridge(prg, raw, None, set());memory.ram[:0x1a] = bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xe38a,0x50,cpu.UNUSED
    stage=held=pressed=0;handoff=target=arrival=None;rounds=0
    for _ in range(3000000):
        bank=memory.bank_at(cpu.pc)
        if bank==30 and cpu.pc==0xd5e5:
            if stage==0:held=pressed=1;stage=1
            elif stage==1:held=pressed=128;stage=2
            elif stage==3:held=pressed=2;stage=4
            elif stage==4:held=pressed=128;stage=5
        if bank==30 and cpu.pc==0xd67c:
            if stage==2:stage=3;held=pressed=0
            elif stage==5:stage=6;held=pressed=0
        if bank==30 and cpu.pc==0xd80a:
            if stage==6:
                for player in range(1,22):
                    if player!=11 and player!=memory[0x441]:
                        memory[0x306+12*player]=52;memory[0x308+12*player]=84;memory[0x30a+12*player]=0
                memory[0x306+12*3],memory[0x308+12*3]=home_x,108
                memory[0x306+12*16],memory[0x308+12*16]=100,108
                held=pressed=1;stage=7
            elif stage==7:held=pressed=128;stage=8
        if bank==30 and cpu.pc==0xd84b and stage==8:
            if memory[0x5fc]!=255:raise RuntimeError('Constructed target was not empty')
            stage=9;held=pressed=0
        if stage==9 and bank==30 and cpu.pc==0xc612 and handoff is None:handoff=bytes(memory.ram[:2048])
        if stage==9 and bank==30 and cpu.pc==0xde64 and target is None:target=bytes(memory.ram[:2048])
        if stage==9 and bank==31 and cpu.pc==0xe7cc:
            arrival=bytes(memory.ram[:2048]);stage=10
        if stage==10 and bank==30 and cpu.pc==0xdeb5:rounds+=1
        if stage==10 and bank==30 and cpu.pc in (0xdf0d,0xded4):
            if handoff is None or target is None or arrival is None:raise RuntimeError('Missing original pass boundary')
            return handoff,target,arrival,bytes(memory.ram[:2048]),cpu.pc,rounds
        if cpu.pc<0x8000:raise RuntimeError(f'Original empty pass left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c],memory[0x1e]=held,pressed
            if memory[0x498]:memory[0x498]-=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Original empty pass exceeded instruction limit')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready-dir','pack','distance','exe','private-temp'):p.add_argument('--'+name,type=Path,required=True)
    args=p.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private temporary data must stay outside the checkout')
    target_count=race_count=0;skipped=[]
    with tempfile.TemporaryDirectory(prefix='empty-pass-',dir=args.private_temp) as temp:
        tmp=Path(temp);before=tmp/'before.cmjsession';after=tmp/'after.cmjsession';raw_file=tmp/'raw.bin';target_file=tmp/'target.bin'
        for fixture in range(4):
            ready=(args.ready_dir/f'{fixture}-ready.bin').read_bytes()
            for home_x in (104,108,110):
                handoff,target,arrival,final,boundary,rounds=observe(rom[16:262160],ready,home_x)
                before.write_bytes(export_session(handoff));target_file.write_bytes(target)
                result=subprocess.run([str(args.exe.resolve()),'--compare-target',str(before),str(target_file),str(args.distance.resolve())],capture_output=True,text=True)
                if result.returncode:raise RuntimeError(f'target fixture={fixture}, x={home_x}: {result.stderr}')
                target_count+=1
                if arrival[0x600] or boundary!=0xdf0d or final[0x5fb]!=0:
                    skipped.append((fixture,home_x,'defended, timeout or opposing winner'));continue
                before.write_bytes(export_session(arrival));after.write_bytes(export_session(final));raw_file.write_bytes(arrival)
                result=subprocess.run([str(args.exe.resolve()),'--compare-free-arrival',str(before),str(after),str(raw_file),str(args.pack.resolve())],capture_output=True,text=True)
                if result.returncode:
                    if 'unsupported: action routing' in result.stderr and rounds>3:skipped.append((fixture,home_x,f'unsupported race after {rounds} rounds'));continue
                    raise RuntimeError(f'race fixture={fixture}, x={home_x}: {result.stderr}')
                race_count+=1
    print(f'{target_count} constructed empty-target handoffs and {race_count} bounded original contests matched')
    if skipped:print(f'{len(skipped)} constructed contests remained outside the supported scope: {skipped}')


if __name__=='__main__':main()

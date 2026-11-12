#!/usr/bin/env python3
"""Observe the original automatic outlet following a home goalkeeper catch."""
import argparse
import hashlib
import struct
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_human_keeper_menu_original import menu_trace, next_random
from check_keeper_saves_original import observe_keeper_save, post_save_checkpoint
from check_unopposed_ai_shot_original import observe
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def trace_catch_outlet(prg, caught, stop_after='control_ready', capture_flight=False):
    memory=ObservedCartridge(prg,caught,None,set());memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x865b,0xfd,cpu.UNUSED
    targets={(30,0xddfd):'start',(30,0xde11):'chosen',(30,0xde2b):'setup',
             (30,0xde38):'prepared',(30,0xde69):'flight_start',(31,0xe773):'flight_setup',(31,0xe7b4):'flight_cell',(30,0xde6c):'flight_done',
             (31,0xe0df):'control_entry',(31,0xe14a):'control_ready'}
    states={};flight_cells=[]
    for _ in range(3_000_000):
        bank=memory.bank_at(cpu.pc);name=targets.get((bank,cpu.pc))
        if name and name not in states:states[name]=bytes(memory.ram[:2048])
        if capture_flight and (bank,cpu.pc)==(31,0xe7b4):flight_cells.append([bytes(memory.ram[:2048])])
        if capture_flight and (bank,cpu.pc)==(31,0xe7b7):flight_cells[-1].append(bytes(memory.ram[:2048]))
        if name==stop_after:
            if capture_flight:states['flight_cells']=flight_cells
            return states
        if cpu.pc<0x8000:raise RuntimeError(f'Catch outlet left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=0
            if memory[0x498]:memory[0x498]-=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError(f'Catch outlet instruction bound at {cpu.pc:04x}')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready-dir'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--one',action='store_true');p.add_argument('--all-choice-values',action='store_true')
    p.add_argument('--exe',type=Path);p.add_argument('--pack',type=Path);p.add_argument('--private-temp',type=Path)
    args=p.parse_args();rom=args.rom.read_bytes()
    if args.all_choice_values and (not args.exe or not args.pack or not args.private_temp):p.error('Choice sweep requires exe, pack and private-temp')
    if args.all_choice_values and args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Temporary data must remain private')
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected verified Japanese ROM')
    prg=rom[16:262160];counts=Counter()
    for fixture in range(4):
        ready=bytearray((args.ready_dir/f'{fixture}-ready.bin').read_bytes());ready[0x614]=ready[0x617]=0
        report,decision=trace_recovery(prg,ready,control_input=(0,0))
        if report['boundary']!='ai_cell_decision':raise RuntimeError('Expected original cell decision')
        for x in (85,95):
            base=bytearray(decision);owner=base[0x441];cell=((x-48)//8)*12+7
            base[0x635],base[0x637],base[0x5fe],base[0x617],base[0x5ff],base[0x43d]=x,137,cell,cell,0,6
            base[0x306+12*owner],base[0x308+12*owner]=x,137
            for seed in range(1,500):
                value=next_random(seed);base[0xe2],base[0xe3]=value&255,(value>>8)&255
                if original_candidate(prg,base)[2]==1:break
            outcome,shot=observe(prg,base)
            if outcome!='flight':raise RuntimeError('Expected shot flight')
            flight=shot['flight-records'];entry=next((flight[i+1:i+2049] for i in range(0,len(flight),2049) if flight[i]==3),None)
            if entry is None:counts['defended']+=1;continue
            for stamina in (20,100,400):
                for seed in (1,123,98765):
                    raw=bytearray(entry);raw[0x301:0x303]=stamina.to_bytes(2,'little');raw[0x61c:0x61e]=(2).to_bytes(2,'little')
                    compared=menu_trace(prg,raw,1,seed)['compared']
                    if compared[0x612]!=3:counts['goal']+=1;continue
                    caught,phase,_=observe_keeper_save(prg,compared)
                    if phase!=57:raise RuntimeError('Expected caught keeper handoff')
                    if args.all_choice_values:
                        seeds={}
                        for candidate in range(1,100000):
                            value=next_random(candidate);seeds.setdefault(value&255,(candidate,value))
                            if len(seeds)==256:break
                        if len(seeds)!=256:raise RuntimeError('Could not construct all random-byte values')
                        with tempfile.TemporaryDirectory(prefix='keeper-outlet-choice-',dir=args.private_temp) as temporary:
                            folder=Path(temporary);source=folder/'caught.cmjsession';expected=folder/'choices.bin'
                            source.write_bytes(post_save_checkpoint(caught,57));records=bytearray()
                            for first in range(256):
                                candidate,value=seeds[first];case=bytearray(caught);case[0xe2],case[0xe3]=first,(value>>8)&255
                                chosen=trace_catch_outlet(prg,case,stop_after='chosen')['chosen'][0x5fc]
                                records+=struct.pack('<IB',candidate,chosen)
                            expected.write_bytes(records)
                            run=subprocess.run([str(args.exe.resolve()),'--compare-keeper-outlet-choices',str(source),str(expected),str(args.pack.resolve())],capture_output=True,text=True)
                            if run.returncode:raise RuntimeError('Native keeper outlet choice sweep failed: '+run.stderr)
                        print('Original/native keeper outlet choice matched all 256 random-byte values')
                        return
                    states=trace_catch_outlet(prg,caught);chosen=states['chosen'][0x5fc];end=states['control_ready']
                    counts[(chosen,end[0x441],end[0x5fb])]+=1
                    if args.one:
                        for name,state in states.items():
                            print(name,'owner',state[0x441],'side',state[0x5fb],'recipient',state[0x5fc],
                                  'ball',state[0x635],state[0x637],'cell',state[0x5fe],
                                  'clock',state[0x5f7]|state[0x5f8]<<8,
                                  'fields',[(hex(a),state[a]) for a in (0x43b,0x43c,0x44e,0x5ff,0x600,0x61a,0x61b,0x62c,0x634,0x636,0x638,0x639,0x63a,0x63b,0x63c)])
                        return
    print('Original catch outlet control outcomes:',dict(sorted(counts.items(),key=lambda item:str(item[0]))))


if __name__=='__main__':main()

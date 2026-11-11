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


def observe(prg, ready, home_x, opposing_x, timeout=False, long_timeout=False, late_winner=False, same_cell=False, direction=1, write_log=None, checkpoints=None):
    _, raw = trace_recovery(prg, ready, control_input=(64, 0))
    memory = ObservedCartridge(prg, raw, None, {0x5ff,0x5f7,0x5f8,0x634,0x636} if write_log is not None else set());memory.ram[:0x1a] = bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xe38a,0x50,cpu.UNUSED
    stage=held=pressed=0;handoff=target=arrival=None;rounds=0;moves_remaining=0;timeout_seen=False;vertical_phase=False
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
                memory[0x306+12*3],memory[0x308+12*3]=home_x,84 if long_timeout else 108
                memory[0x306+12*16],memory[0x308+12*16]=(home_x-8 if timeout else opposing_x),84 if long_timeout else 108
                held=pressed=128 if same_cell else 8 if long_timeout else direction;stage=8 if same_cell else 7;moves_remaining=3 if long_timeout else 5 if timeout or late_winner else 1;vertical_phase=long_timeout
            elif stage==7:
                moves_remaining-=1
                if not moves_remaining:
                    if vertical_phase:held=pressed=1;moves_remaining=11;vertical_phase=False
                    else:held=pressed=128;stage=8
        if bank==30 and cpu.pc==0xd84b and stage==8:
            if memory[0x5fc]!=255:raise RuntimeError('Constructed target was not empty')
            stage=9;held=pressed=0
        if checkpoints is not None and stage==9 and ((bank==30 and cpu.pc==0xd193) or (bank==31 and cpu.pc in (0xe074,0xe0d9))):
            checkpoints.append((bank,cpu.pc,cpu.a,bytes(memory.ram[:2048])))
        if stage==9 and bank==30 and cpu.pc==0xc612 and handoff is None:handoff=bytes(memory.ram[:2048])
        if stage==9 and bank==30 and cpu.pc==0xde64 and target is None:target=bytes(memory.ram[:2048])
        if stage==9 and bank==31 and cpu.pc in (0xe7cc,0xe7cf):
            arrival=bytes(memory.ram[:2048]);stage=10
        if stage==10 and bank==30 and cpu.pc==0xdeb5:rounds+=1
        if stage==10 and timeout and bank==30 and cpu.pc==0xded4:timeout_seen=True;stage=11
        if stage==10 and timeout and bank==30 and cpu.pc==0xdf0d:
            if write_log is not None:write_log.extend(memory.writes)
            return handoff,target,arrival,bytes(memory.ram[:2048]),cpu.pc,rounds
        if ((stage==10 and not timeout and bank==30 and cpu.pc in (0xdf0d,0xded4)) or
            (stage==11 and bank==30 and cpu.pc==0xc60f)):
            if handoff is None or target is None or arrival is None:raise RuntimeError('Missing original pass boundary')
            if write_log is not None:write_log.extend(memory.writes)
            return handoff,target,arrival,bytes(memory.ram[:2048]),0xded4 if timeout_seen else cpu.pc,rounds
        if cpu.pc<0x8000:raise RuntimeError(f'Original empty pass left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c],memory[0x1e]=held,pressed
            if memory[0x498]:memory[0x498]-=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Original empty pass exceeded instruction limit')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready-dir','pack','distance','exe','private-temp'):p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--opponent-winner',action='store_true',help='Construct races favoring the opponent')
    p.add_argument('--timeout',action='store_true',help='Construct long empty-target races and compare timeout recovery')
    p.add_argument('--long-timeout',action='store_true',help='Construct five-round timeout recoveries')
    p.add_argument('--late-winner',action='store_true',help='Construct longer races favoring the home player')
    p.add_argument('--long-winner',action='store_true',help='Construct long-distance races favoring the home player')
    p.add_argument('--same-cell',action='store_true',help='Construct passes targeting the ball\'s current cell')
    p.add_argument('--retained-action',type=int,choices=range(7),help='Compare preservation of an explicitly seeded original defensive-action byte')
    args=p.parse_args();rom=args.rom.read_bytes()
    if args.long_timeout:args.timeout=True
    if args.retained_action is not None and (args.timeout or args.opponent_winner):p.error('Retained-action comparison currently covers home race winners')
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private temporary data must stay outside the checkout')
    target_count=race_count=0;skipped=[]
    with tempfile.TemporaryDirectory(prefix='empty-pass-',dir=args.private_temp) as temp:
        tmp=Path(temp);before=tmp/'before.cmjsession';after=tmp/'after.cmjsession';raw_file=tmp/'raw.bin';target_file=tmp/'target.bin'
        for fixture in range(4):
            ready=(args.ready_dir/f'{fixture}-ready.bin').read_bytes()
            if args.retained_action is not None:
                ready=bytearray(ready);ready[0x43d]=args.retained_action
            for home_x in ((60,68,76) if args.timeout else (170,174,178,182,186,190) if args.long_winner else (124,128,132,136,140,142) if args.late_winner else (96,100,104) if args.opponent_winner else (104,108,110)):
                handoff,target,arrival,final,boundary,rounds=observe(rom[16:262160],ready,home_x,52 if args.late_winner or args.long_winner else 108 if args.opponent_winner else 100,args.timeout,args.long_timeout or args.long_winner,args.late_winner,args.same_cell)
                before.write_bytes(export_session(handoff));target_file.write_bytes(target)
                result=subprocess.run([str(args.exe.resolve()),'--compare-target',str(before),str(target_file),str(args.distance.resolve())],capture_output=True,text=True)
                if result.returncode:raise RuntimeError(f'target fixture={fixture}, x={home_x}: {result.stderr}')
                target_count+=1
                expected_side=11 if args.opponent_winner else 0
                if arrival[0x600] or (args.timeout and boundary!=0xded4) or (not args.timeout and (boundary!=0xdf0d or final[0x5fb]!=expected_side)):
                    skipped.append((fixture,home_x,'defended, timeout or different winner'));continue
                before.write_bytes(export_session(arrival));after.write_bytes(export_session(final));raw_file.write_bytes(arrival)
                if args.retained_action is not None and any(raw[0x43d]!=args.retained_action for raw in (handoff,target,arrival,final)):raise RuntimeError('Original pass changed retained action')
                command=[str(args.exe.resolve()),'--compare-same-arrival' if args.same_cell else '--compare-free-arrival',str(before),str(after),str(raw_file),str(args.pack.resolve())]
                if args.retained_action is not None:command.append('--retained')
                result=subprocess.run(command,capture_output=True,text=True)
                if result.returncode:
                    if 'unsupported: action routing' in result.stderr and rounds>(5 if args.long_timeout or args.long_winner else 4 if args.timeout or args.late_winner else 3):skipped.append((fixture,home_x,f'unsupported race after {rounds} rounds'));continue
                    raise RuntimeError(f'race fixture={fixture}, x={home_x}, rounds={rounds}: {result.stderr}')
                race_count+=1
    print(f'{target_count} constructed empty-target handoffs and {race_count} bounded original {"timeout recoveries" if args.timeout else "contests"} matched')
    if skipped:print(f'{len(skipped)} constructed contests remained outside the supported scope: {skipped}')


if __name__=='__main__':main()

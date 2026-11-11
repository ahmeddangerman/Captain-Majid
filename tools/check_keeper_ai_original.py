#!/usr/bin/env python3
"""Compare opposing-keeper AI choices with controlled original CPU execution."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_kickoff_flight_original import until, snapshot as flight_snapshot
from check_kickoff_shot_launch_original import observe_clock
from check_keeper_entry_original import keeper_checkpoint, observe as observe_keeper_entry
from check_scored_goal_original import observe_scored_goal
from check_keeper_saves_original import observe_keeper_save, post_save_checkpoint
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_field_loop import trace_loop
from trace_field_script import ObservedCartridge
from trace_shot_approach import trace_approach
from verify_action_strength import PAYLOAD_SHA256


def keeper_entry(prg, ready):
    ready = bytearray(ready);owner = ready[0x441]
    ready[0x301+12*owner:0x303+12*owner] = (400).to_bytes(2, 'little')
    _, ram = trace_recovery(prg, ready, control_input=(64, 0))
    _, ram = trace_menu(prg, ram);_, ram = trace_menu(prg, ram, 'poll', 129);_, ram = trace_menu(prg, ram, 'basic-shot')
    ram, _, _, _ = observe_clock(prg, ram)
    moved = bytearray();trace_loop(prg, ram, dispatch_only=True, final_ram=moved)
    tail = bytearray();trace_loop(prg, moved, attack_tail=True, final_ram=tail)
    report, ram = trace_approach(prg, tail)
    if report['loose_ball']: raise RuntimeError('Expected a keeper-bound shot')
    memory = ObservedCartridge(prg, ram, None, set());memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory);cpu.pc, cpu.sp, cpu.p = 0xe73e, 0xfc, cpu.UNUSED
    memory[0x1fd],memory[0x1fe],memory[0x1ff] = 0x10,0xdd,cpu.UNUSED
    until(cpu, {0xe773})
    for _ in range(100):
        cpu.pc = 0xe778;until(cpu, {0xe7b4, 0xe7cc})
        if cpu.pc == 0xe7cc: break
        until(cpu, {0xe773})
    else: raise RuntimeError('Shot did not arrive')
    if memory[0x600]: raise RuntimeError('Expected unopposed shot')
    until(cpu, {0xdd1c});until(cpu, {0x8509, 0x8900})
    if cpu.pc != 0x8509: raise RuntimeError('Expected normal opposing keeper')
    return bytes(memory.ram[:2048]), cpu.sp, cpu.p


def observe(prg, base, sp, flags, kind, low, high, route, random, power=None, through_result=False):
    ram = bytearray(base);ram[0x384:0x387] = bytes((kind, low, high));ram[0x621] = route;ram[0xe2] = random
    if power is not None:ram[0x61c:0x61e]=power.to_bytes(2,'little')
    memory = ObservedCartridge(prg, ram, None, set());memory.registers[6:8] = [26, 27]
    memory[0x24],memory[0x25] = 26,27;memory[0x23] = memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x8509,sp,flags
    for _ in range(10000):
        if cpu.pc==(0x8599 if through_result else 0x852f):
            return (memory[0x43d],memory[0x43e],bytes(ram),bytes(memory.ram[:2048])) if through_result else (memory[0x43d],memory[0x43e])
        if cpu.pc<0x8000: raise RuntimeError(f'Keeper AI left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0
        memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
    raise RuntimeError('Keeper AI instruction bound')


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready','pack','exe'):parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--runtime-exe',type=Path,help='Also compare autonomous runtime keeper resolution')
    parser.add_argument('--private-temp',type=Path,help='Private directory for temporary sessions')
    parser.add_argument('--team',type=int,choices=(3,4,5,6),default=3,help='Explicit original team matching the private pack')
    args=parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected verified Japanese ROM')
    prg=rom[16:262160];ready=bytearray(args.ready.read_bytes());ready[0x2b]=args.team
    pointer_offset=28*8192+(0xbab2-0x8000)+2*(args.team-3);ready[0x38:0x3a]=prg[pointer_offset:pointer_offset+2]
    base,sp,flags=keeper_entry(prg,ready)
    cases=[]
    for kind,low,high in ((0,100,0),(0,128,64),(35,100,0),(64,100,0),(95,100,0),(117,100,0)):
        for route in (0,1,3,4):
            for random in (*range(8),127,128,255):
                cases.append((kind,low,high,route,random))
    expected=[observe(prg,base,sp,flags,*case) for case in cases]
    batch=''.join(' '.join(map(str,case))+'\n' for case in cases)
    result=subprocess.run([str(args.exe.resolve()),'--keeper-ai',str(args.pack.resolve())],input=batch,text=True,capture_output=True)
    if result.returncode:raise RuntimeError(f'Native keeper AI failed: {result.stderr}')
    actual=result.stdout.splitlines()
    if len(actual)!=len(expected):raise RuntimeError('Native keeper AI returned the wrong number of cases')
    matched=0;skipped=[]
    for case,wanted,got in zip(cases,expected,actual):
        if wanted[0]>3:
            if got!='unsupported':raise RuntimeError(f'Keeper AI {case}: unsupported original action {wanted}, native {got}')
            skipped.append((case,wanted));continue
        if got=='unsupported' and case[0]==117 and case[3] in (0,4):
            skipped.append((case,wanted));continue
        if got != f'{wanted[0]} {wanted[1]}':raise RuntimeError(f'Keeper AI {case}: original {wanted}, native {got}')
        matched+=1
    print(f'{matched} constructed original opposing-keeper AI choices matched; {len(skipped)} unsupported action cases excluded')
    print(f'Excluded cases by route/action: {dict(sorted(Counter((case[3],wanted[0]) for case,wanted in skipped).items()))}')
    if args.runtime_exe:
        if not args.private_temp or args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):parser.error('A private temporary directory is required')
        count=goals=saves=restarts=0;outcomes=Counter()
        with tempfile.TemporaryDirectory(prefix='keeper-ai-',dir=args.private_temp) as temp:
            before,after=Path(temp)/'before.cmjsession',Path(temp)/'after.cmjsession'
            for stamina in (50,100,120):
                for power in (5,20,80):
                    for random in range(8):
                        action,subtype,source,final=observe(prg,base,sp,flags,0,stamina,0,0,random,power,True)
                        if action>3:continue
                        before.write_bytes(keeper_checkpoint(source,53));after.write_bytes(keeper_checkpoint(final,56))
                        result=subprocess.run([str(args.runtime_exe.resolve()),'--compare-human-shot-keeper',str(before),str(after),str(args.pack.resolve())],capture_output=True,text=True)
                        if result.returncode:raise RuntimeError(f'Keeper result stamina={stamina}, power={power}, random={random}, choice={action}/{subtype}: {result.stderr}')
                        count+=1;outcomes[final[0x612]]+=1
                        if final[0x612]==0:
                            goal,_=observe_scored_goal(prg,final);after.write_bytes(keeper_checkpoint(goal,59))
                            result=subprocess.run([str(args.runtime_exe.resolve()),'--compare-human-shot-goal',str(before),str(after),str(args.pack.resolve()),str(final[0x28]),str(final[0x29]),str(final[0x629])],capture_output=True,text=True)
                            if result.returncode:raise RuntimeError(f'Goal stamina={stamina}, power={power}, random={random}: {result.stderr}')
                            goals+=1
                            if goal[0x62d]==0:
                                restarted,_,_=observe_keeper_entry(prg,goal,0x85bf,{0x85dd});after.write_bytes(flight_snapshot(restarted,60,False))
                                result=subprocess.run([str(args.runtime_exe.resolve()),'--compare-human-shot-goal-restart',str(before),str(after),str(args.pack.resolve()),str(final[0x28]),str(final[0x29]),str(final[0x629])],capture_output=True,text=True)
                                if result.returncode:raise RuntimeError(f'Goal restart stamina={stamina}, power={power}, random={random}: {result.stderr}')
                                restarts+=1
                        elif final[0x612] in (2,3):
                            saved,phase,_=observe_keeper_save(prg,final);after.write_bytes(post_save_checkpoint(saved,phase))
                            result=subprocess.run([str(args.runtime_exe.resolve()),'--compare-human-shot-keeper-save',str(before),str(after),str(args.pack.resolve())],capture_output=True,text=True)
                            if result.returncode:raise RuntimeError(f'Keeper save stamina={stamina}, power={power}, random={random}: {result.stderr}')
                            saves+=1
        print(f'{count} constructed autonomous keeper choices and comparisons matched; result codes {dict(sorted(outcomes.items()))}')
        print(f'{goals} constructed original goal/reward registrations matched')
        print(f'{saves} constructed original goalkeeper save/recovery handoffs matched')
        print(f'{restarts} constructed original post-goal restart handoffs matched')


if __name__=='__main__':main()

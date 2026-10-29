#!/usr/bin/env python3
"""Compare numeric flight from prepared kickoff destinations, saving every cell."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from flight_checkpoint import flight_checkpoint
from check_keeper_entry_original import keeper_checkpoint, observe
from check_keeper_saves_original import post_save_checkpoint
from check_keeper_comparison_original import observe_keeper_comparison
from check_scored_goal_original import observe_scored_goal
from verify_action_strength import PAYLOAD_SHA256


def until(cpu, targets):
    for count in range(100000):
        if cpu.pc in targets:return count
        if cpu.pc<0x8000:raise RuntimeError('Flight left original PRG')
        cpu.step()
    raise RuntimeError('Original flight instruction bound')


def snapshot(ram, phase, loose):
    data=bytearray(post_save_checkpoint(ram,phase) if phase==60 else
                   keeper_checkpoint(ram,phase) if phase in (53,56,59) else flight_checkpoint(ram,phase,loose))
    data[361:365]=bytes([1,ram[0x28],ram[0x29],ram[0x629]])
    data[365:368]=bytes([1,ram[0x308],ram[0x38c]])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','projection','launch_dir','output_dir'):p.add_argument(name,type=Path)
    p.add_argument('--keeper-entry',action='store_true')
    p.add_argument('--keeper-choice',action='store_true')
    p.add_argument('--goal',action='store_true')
    p.add_argument('--post-goal',action='store_true')
    a=p.parse_args()
    if a.keeper_choice and not a.keeper_entry:p.error('Keeper choice requires keeper entry')
    if a.goal and not a.keeper_choice:p.error('Goal registration requires keeper choice')
    if a.post_goal and not a.goal:p.error('Post-goal handoff requires goal registration')
    rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());cases=[]
    entries=json.loads((a.launch_dir/'verification.json').read_text())['cases']
    for entry in entries:
        name=f'l{entry["left"]}-r{entry["right"]}';loose=entry['loose']
        ram=(a.launch_dir/f'{name}-destination.bin').read_bytes()
        source=a.launch_dir/f'{name}-4.cmjsession'
        # Preserve the carry/return expected after original DCFD..DD08.
        m=ObservedCartridge(rom[16:262160],ram,None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xe73e;cpu.sp=0xfc;cpu.p=cpu.UNUSED
        m[0x1fd],m[0x1fe],m[0x1ff]=0x10,0xdd,cpu.UNUSED|int(loose)
        until(cpu,{0xe773})
        index=0

        def compare(phase,operation):
            nonlocal source,index
            index+=1;target=out/f'{name}-{index}.cmjsession'
            run=subprocess.run([binary,pack,str(source),*operation,'--capture',str(target)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'{name} {operation}: {run.stderr}')
            actual,expected=target.read_bytes(),snapshot(bytes(m.ram[:2048]),phase,loose)
            if actual!=expected:raise RuntimeError(f'{name} {operation}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
            source=target

        compare(47,['--begin-ball-flight',str(a.projection.resolve()),str(ram[0x634]),str(ram[0x636])])
        cells=[]
        for _ in range(100):
            # Explicitly acknowledge each original frame wait; no scheduler/NMI update.
            cpu.pc=0xe778
            until(cpu,{0xe7b4,0xe7cc})
            phase=48 if cpu.pc==0xe7b4 else 49
            compare(phase,['--advance-ball-cell'])
            cells.append({'cell':m[0x5fe],'phase':phase,'selected':m[0x600]})
            if phase==49:break
            until(cpu,{0xe773})
            compare(47,['--resolve-ball-cell'])
        else:raise RuntimeError('Flight did not arrive within100cells')
        decisions=[]
        if a.keeper_entry:
            if loose or m[0x600]:raise RuntimeError('Keeper entry requires unopposed keeper-bound arrival')
            until(cpu,{0xdd1c})
            compare(52,['--continue-flight-arrival'])
            until(cpu,{0x8509,0x8900})
            if cpu.pc!=0x8509:raise RuntimeError('Expected normal keeper')
            compare(53,['--prepare-keeper-entry'])
            (out/f'{name}-keeper-entry.bin').write_bytes(m.ram[:2048])
            (out/f'{name}-keeper-entry.cmjsession').write_bytes(source.read_bytes())
            if a.keeper_choice:
                prepared_source=source;prepared_ram=bytes(m.ram[:2048])
                for action in range(4):
                    for subtype in (0,128):
                        original_input=bytearray(prepared_ram)
                        original_input[0x43d],original_input[0x43e]=action,subtype
                        final,instructions=observe_keeper_comparison(rom[16:262160],original_input)
                        source=prepared_source;m.ram[:2048]=final
                        compare(56,['--resolve-prepared-keeper',str(action),str(subtype),str(final[0x2b])])
                        if a.goal:
                            if final[0x612]!=0:raise RuntimeError('Goal comparison requires goal result0')
                            baseline=[final[x] for x in (0x28,0x29,0x629)]
                            scored,goal_instructions=observe_scored_goal(rom[16:262160],final)
                            m.ram[:2048]=scored
                            compare(59,['--prepare-scored-goal',*map(str,baseline)])
                            if a.post_goal:
                                if scored[0x62d]:raise RuntimeError('Prepared post-menu handoff requires captured ball mode0')
                                restarted,_,restart_steps=observe(rom[16:262160],scored,0x85bf,{0x85dd})
                                m.ram[:2048]=restarted
                                compare(60,['--continue-prepared-goal-restart'])
                        decisions.append({'action':action,'subtype':subtype,'result':final[0x612],
                                          'instructions':instructions,'goal_registered':a.goal,'post_goal':a.post_goal})
        cases.append({'left':entry['left'],'right':entry['right'],'boundaries':index,'cells':cells,'loose':loose,'keeper_entry':a.keeper_entry,'keeper_decisions':decisions})
    (out/'verification.json').write_text(json.dumps({'cases':cases,'scope':'Native saved destination feeds flight setup and each cell/callback; all represented bytes match original CPU, including accumulated defenders and keeper Y. Original numeric frame waits explicitly acknowledged with no NMI/PPU/audio or task/random updates. Fractional input bytes captured from original RAM, not invented. Optional empty-arrival routing and normal keeper setup stop before the keeper presentation; optional prepared keeper comparison supplies action/subtype after explicitly acknowledged keeper presentation; keeper display9 explicitly acknowledged before optional score/reward registration, stopping before celebration. Optional post-goal cleanup explicitly assumes completed celebration and post-goal menu with captured ball mode0; no actual presentation is executed.'},indent=2)+'\n')
    print(f'{len(cases)} kickoff flight sequences match: {out}')

if __name__=='__main__':main()

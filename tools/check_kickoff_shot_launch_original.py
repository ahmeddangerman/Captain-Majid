#!/usr/bin/env python3
"""Compare prepared kickoff ordinary shot timing, dispatch and destination."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from trace_field_loop import trace_loop
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_shot_approach import trace_approach
from check_kickoff_control_original import checkpoint
from verify_action_strength import PAYLOAD_SHA256


def observe_clock(prg, ram):
    m = ObservedCartridge(prg, ram, None, {0x517, 0x532})
    m.ram[:0x1a] = bytes(0x1a)
    for addr, value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),
                        (7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        m[addr] = value
    m[0x23] = m[0x22] | 7
    m.select = m[0x23]
    m.registers[6:8] = [26,27]
    cpu = MPU(memory=m)
    cpu.pc, cpu.sp, cpu.p = 0x82a0, 0x50, cpu.UNUSED
    frames = 0
    for step in range(200000):
        if m.bank_at(cpu.pc)==26 and cpu.pc==0x83f5:
            return bytes(m.ram[:2048]), step, frames, m.writes
        if cpu.pc < 0x8000:
            raise RuntimeError('Clock left original PRG')
        if cpu.pc==0xcaae:
            m.instruction=None
            m[0x1b] |= 128
            m[0x515] = 0
            if m[0x498]:m[0x498]-=1
            frames += 1
        m.instruction=(m.bank_at(cpu.pc),cpu.pc)
        cpu.step()
    raise RuntimeError('Clock instruction bound')


def snapshot(ram, phase, loose=False):
    data = bytearray(checkpoint(ram, phase))
    if phase in (27,43,45,46):data[324:330]=bytes([1,0,0,0,0,0])
    if phase==46:data[339:344]=bytes([1,ram[0x62b],ram[0x638],ram[0x61a],int(loose)])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','observations_dir','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];results=[]
    binary,pack=str(Path('build/captain-phase-session').resolve()),str(a.pack.resolve())
    for left in range(4):
        for right in range(4):
            ram=(a.observations_dir/f's0-l{left}-r{right}-after.bin').read_bytes()
            _,ram=trace_recovery(prg,ram,control_input=(64,0))
            _,ram=trace_menu(prg,ram)
            _,ram=trace_menu(prg,ram,mode='poll',pressed=129)
            _,ram=trace_menu(prg,ram,mode='basic-shot')
            name=f'l{left}-r{right}';source=out/f'{name}-entry.cmjsession';source.write_bytes(snapshot(ram,20));index=0

            def compare(after,phase,operation,loose=False):
                nonlocal source,index
                index+=1;target=out/f'{name}-{index}.cmjsession'
                run=subprocess.run([binary,pack,str(source),*operation,'--capture',str(target)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'{name} {operation}: {run.stderr}')
                actual,expected=target.read_bytes(),snapshot(after,phase,loose)
                if actual!=expected:raise RuntimeError(f'{name} {operation}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}; lengths {len(actual)}/{len(expected)}')
                source=target

            ram,steps,frames,writes=observe_clock(prg,ram)
            compare(ram,27,['--dispatch-shot','1'])
            raw=bytearray();trace_loop(prg,ram,dispatch_only=True,final_ram=raw);ram=bytes(raw)
            compare(ram,43,['--resolve-shot-result'])
            # Original script8 remains explicitly acknowledged at this boundary.
            raw=bytearray();report=trace_loop(prg,ram,attack_tail=True,final_ram=raw);ram=bytes(raw)
            if report['boundary']!='shot_approach_entry':raise RuntimeError('Expected ordinary approach')
            compare(ram,45,['--continue-unopposed-attack',str(ram[0x43d])])
            report,ram=trace_approach(prg,ram)
            compare(ram,46,['--prepare-shot-approach'],report['loose_ball'])
            (out/f'{name}-destination.bin').write_bytes(ram)
            results.append({'left':left,'right':right,'boundaries':index,'clock_instructions':steps,
                            'clock_host_frames':frames,'presentation_writes':writes,'destination':ram[0x638],'loose':report['loose_ball']})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Prepared ordinary kickoff shot with no field defenders through destination. Native results feed next commands. Original82A0 includes script1 and final timing under disclosed host frame acknowledgements; script8 explicitly acknowledged. No actual NMI/audio/PPU, special moves, defenders or flight.'},indent=2)+'\n')
    print(f'{len(results)} kickoff shot-launch sequences match: {out}')

if __name__=='__main__':main()

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
from verify_action_strength import PAYLOAD_SHA256


def until(cpu, targets):
    for count in range(100000):
        if cpu.pc in targets:return count
        if cpu.pc<0x8000:raise RuntimeError('Flight left original PRG')
        cpu.step()
    raise RuntimeError('Original flight instruction bound')


def snapshot(ram, phase, loose):
    data=bytearray(flight_checkpoint(ram,phase,loose))
    data[361:365]=bytes([1,ram[0x28],ram[0x29],ram[0x629]])
    data[365:368]=bytes([1,ram[0x308],ram[0x38c]])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','projection','launch_dir','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
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
        cases.append({'left':entry['left'],'right':entry['right'],'boundaries':index,'cells':cells,'loose':loose})
    (out/'verification.json').write_text(json.dumps({'cases':cases,'scope':'Native saved destination feeds flight setup and each cell/callback; all represented bytes match original CPU, including accumulated defenders and keeper Y. Original numeric frame waits explicitly acknowledged with no NMI/PPU/audio or task/random updates. Fractional input bytes captured from original RAM, not invented. Stops before arrival encounter resolution.'},indent=2)+'\n')
    print(f'{len(cases)} kickoff flight sequences match: {out}')

if __name__=='__main__':main()

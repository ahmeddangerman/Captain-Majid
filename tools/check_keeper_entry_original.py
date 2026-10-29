#!/usr/bin/env python3
"""Compare prepared keeper entry/status and all stunned random decisions.

Private ROM/RAM outputs stay outside checkout; no display calls are skipped.
Normal case stops before its first display, stunned case before display12.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from flight_checkpoint import flight_checkpoint
from verify_action_strength import PAYLOAD_SHA256


def keeper_checkpoint(ram, phase):
    # The legacy field exporter intentionally rejects keeper IDs in field prefixes.
    # Export an empty field prefix, then encode the explicitly represented keeper.
    base=bytearray(ram);base[0x600:0x606]=bytes(6)
    data=bytearray(flight_checkpoint(base,phase,False))
    count=ram[0x600];data[28]=count
    data[309:314]=ram[0x601:0x601+count]+bytes(5-count)
    if phase in (55,56):data[355:358]=bytes([1,ram[0x616],ram[0x612]])
    if phase==56:data[358:361]=bytes([ram[0x43D],ram[0x43E],ram[0x445]])
    return bytes(data)


def observe(prg, ram, entry, stops):
    memory=ObservedCartridge(prg,bytes(ram),None,set());memory.registers[6:8]=[26,27]
    memory[0x24],memory[0x25]=26,27;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc=entry;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    for instructions in range(2000):
        if cpu.pc in stops:return bytes(memory.ram[:2048]),cpu.pc,instructions
        if cpu.pc<0x8000:raise RuntimeError('Keeper execution left PRG unexpectedly')
        cpu.step()
    raise RuntimeError('Keeper original instruction bound reached')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected ordinary side0 shot RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];binary=str(a.binary.resolve());pack=str(a.pack.resolve())
    thresholds=tuple(prg[26*8192+0x928:26*8192+0x92A]);routes=[];decisions=[]
    def native(source,operation,dest):
        subprocess.run([binary,pack,str(source),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
    for stun in (0,1,127,255):
        ram=bytearray(base);ram[0x600:0x606]=bytes(6);ram[0x5FF]=0;ram[0x43C]=0
        ram[0x635]=204;ram[0x637]=124;ram[0x5FE]=ram[0x638]=233;ram[0x61A]=255;ram[0x61B]=0
        ram[0x30A]=255 if stun==0 else 0;ram[0x38E]=stun
        entry=out/f'stun{stun}-entry.cmjsession';entry.write_bytes(flight_checkpoint(ram,52,False))
        final,pc,count=observe(prg,ram,0xDD1C,{0x8509,0x8900});phase=53 if pc==0x8509 else 54
        saved=out/f'stun{stun}-prepared.cmjsession';native(entry,['--prepare-keeper-entry'],saved)
        if saved.read_bytes()!=keeper_checkpoint(final,phase):raise RuntimeError(f'Keeper entry mismatch for stun{stun}')
        (out/f'stun{stun}-final.bin').write_bytes(final)
        routes.append({'stun':stun,'phase':phase,'instructions':count,'match':True})
        if subprocess.run([binary,pack,str(saved),'--prepare-keeper-entry'],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Keeper entry replay accepted')
        if stun==1:
            for random in range(256):
                state=bytearray(final);state[0xE2]=random;state[0x616]=4;state[0x612]=3
                source=out/f'random{random}-entry.cmjsession';source.write_bytes(keeper_checkpoint(state,54))
                wanted,pc,steps=observe(prg,state,0x8900,{0x8917})
                dest=out/f'random{random}-native.cmjsession';native(source,['--resolve-stunned-keeper',*map(str,thresholds)],dest)
                if dest.read_bytes()!=keeper_checkpoint(wanted,55):raise RuntimeError(f'Stunned result mismatch at random{random}')
                decisions.append({'random':random,'result':wanted[0x612],'instructions':steps,'match':True})
            # Exercise interactive save and exact command input validation.
            interactive=out/'interactive.cmjsession'
            run=subprocess.run([binary,pack,str(saved),'--interactive'],input=f'resolve-stunned-keeper {thresholds[0]} {thresholds[1]}\nsave {interactive}\nquit\n',text=True,capture_output=True,check=True,timeout=30)
            if 'Error:' in run.stdout or not interactive.exists() or interactive.read_bytes()[258]!=55:raise RuntimeError('Interactive keeper save failed')
            for invalid in (['--resolve-stunned-keeper','0','16'],['--resolve-stunned-keeper','16','1'],['--resolve-stunned-keeper','256','0'],['--resolve-stunned-keeper','16','0','--prepare-keeper-entry']):
                if subprocess.run([binary,pack,str(saved),*invalid],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Invalid keeper CLI accepted')
    (out/'verification.json').write_text(json.dumps({'routes':routes,'decisions':decisions,'thresholds':thresholds,'scope':'Every original instruction executes fromDD1C to8509/8900, and8900 to8917. No presentation/frame calls skipped or executed; numeric keeper setup and stunned decision only.'},indent=2)+'\n')
    print(f'{len(routes)} keeper entry routes and{len(decisions)} stunned decisions match: {out}')


if __name__=='__main__':main()

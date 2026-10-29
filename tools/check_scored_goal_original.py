#!/usr/bin/env python3
"""Compare scorer reward/score writes through original pre-presentation85F0.

Explicit post-keeper presentation state; does not execute celebration30/987B,
side flip, keeper cleanup or kickoff. Private outputs stay outside checkout.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_keeper_entry_original import keeper_checkpoint
from verify_action_strength import PAYLOAD_SHA256


def observe_scored_goal(prg,ram):
    memory=ObservedCartridge(prg,bytes(ram),None,set());memory.registers[6:8]=[26,27]
    memory[0x24],memory[0x25]=26,27;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc=0x85AC;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    for steps in range(20000):
        if cpu.pc==0x85F0:return bytes(memory.ram[:2048]),steps
        if cpu.pc<0x8000:raise RuntimeError('Goal handler left original PRG')
        cpu.step()
    raise RuntimeError('Goal preparation instruction bound')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0 or not 1<=base[0x441]<=10:p.error('Expected ordinary side0 keeper RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());results=[]
    for stunned in (0,1):
        for code in (0,1):
            for subtype in ((0,3,131) if code==0 else (0,2,128)):
                for left in (0,255):
                    for kind in (1,33,64):
                        name=f'case{len(results)}';ram=bytearray(base)
                        count=0 if stunned else 1
                        for address,value in ((0x612,code),(0x616,0),(0x43C,subtype),(0x43D,0),(0x43E,0),(0x445,0),
                          (0x600,count),(0x442,11),(0x5FF,0),(0x38E,stunned),(0x28,left),(0x29,7),(0x629,4),(0x300+12*base[0x441],kind)):
                            ram[address]=value
                        if ram[0x300+12*ram[0x441]]!=kind:raise RuntimeError('Scorer type fixture did not select current ball owner')
                        ram[0x601:0x606]=bytes([11,0,0,0,0]) if count else bytes(5)
                        ram[0x635]=204;ram[0x637]=124;ram[0x5FE]=ram[0x638]=233;ram[0x61A]=255;ram[0x61B]=0
                        if left==255:ram[0x454:0x46A]=(65530).to_bytes(2,'little')*11
                        source=out/(name+'-entry.cmjsession');source.write_bytes(keeper_checkpoint(ram,55 if stunned else 56))
                        final,instructions=observe_scored_goal(rom[16:262160],ram)
                        dest=out/(name+'-native.cmjsession')
                        operation=['--prepare-scored-goal',str(left),'7','4']
                        subprocess.run([binary,pack,str(source),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
                        actual,wanted=dest.read_bytes(),keeper_checkpoint(final,59)
                        if actual!=wanted:raise RuntimeError(f'{name}: goal mismatch {[(i,x,y)for i,(x,y)in enumerate(zip(actual,wanted))if x!=y]}; lengths{len(actual)}/{len(wanted)}')
                        results.append({'case':name,'stunned':stunned,'result':code,'attack_subtype':subtype,'scorer_type':kind,'old_left':left,'new_left':final[0x28],'instructions':instructions,'match':True})
                        if len(results)==1:
                            if subprocess.run([binary,pack,str(dest),*operation],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Goal/reward replay accepted')
                            interactive=out/'interactive.cmjsession'
                            run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'prepare-scored-goal {left} 7 4\nsave {interactive}\nquit\n',text=True,capture_output=True,check=True,timeout=30)
                            if 'Error:' in run.stdout or interactive.read_bytes()!=actual:raise RuntimeError('Interactive goal mismatch')
                            if subprocess.run([binary,pack,str(source),*operation,'--resolve-keeper-save'],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Mixed goal operation accepted')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Explicit post-keeper presentation goal result. All original85AC..85F0 scorer reward/score instructions execute; stop before goal presentation1/celebration30,987B,side switch,keeper cleanup,kickoff. No live presentation claim.'},indent=2)+'\n')
    print(f'{len(results)} prepared scored goals match: {out}')


if __name__=='__main__':main()

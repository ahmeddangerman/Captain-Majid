#!/usr/bin/env python3
"""Compare saved normal keeper preparation/comparison/story before presentation9.

Keeper choice presentation is explicitly acknowledged and action/subtype supplied.
Original starts852F and executes stat-loading script8, energy, comparison and story.
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


def observe_keeper_comparison(prg,ram):
    memory=ObservedCartridge(prg,bytes(ram),None,set());memory.registers[6:8]=[26,27]
    memory[0x24],memory[0x25]=26,27;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc=0x852F;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    for count in range(20000):
        if cpu.pc==0x8599:return bytes(memory.ram[:2048]),count
        if cpu.pc<0x8000:raise RuntimeError('Keeper comparison left PRG')
        cpu.step()
    raise RuntimeError('Prepared keeper original instruction bound')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected side0 ordinary keeper RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());results=[]
    cases=[]
    for action in range(4):
        for keeper_type in (1,33):
            for first,second in ((0,0),(127,128),(205,249),(206,255),(243,247),(244,248),(255,255)):
                for power in (0,100,65535):cases.append((action,keeper_type,first,second,power,4,0,0,0,0))
    for story,subtype,first in ((1,1,0),(1,1,128),(4,1,255),(4,3,127),(4,3,128),(4,131,255)):
        cases.append((0,33,first,249,100,5,story,subtype,250,79))
    cases=[(*case,0,0) for case in cases]
    for action in range(4):
        for context in (0,4):
            for defense_subtype in (0,128):
                for first in (205,206,243,244,255):
                    cases.append((action,64,first,255,100,4,0,0,250,79,defense_subtype,context))
    for index,(action,kind,first,second,power,team,story,attack_subtype,timer,penalty,defense_subtype,context) in enumerate(cases):
        name=f'case{index}';ram=bytearray(base)
        for address,value in ((0x600,1),(0x601,11),(0x442,11),(0x43D,action),(0x43E,defense_subtype),(0x43C,attack_subtype),
                              (0x2B,team),(0x446,story),(0xE2,first),(0xE3,second),(0x384,kind),(0x389,timer),
                              (0x38B,penalty),(0x621,context),(0x38E,0),(0x5FF,0),(0x616,4)):
            ram[address]=value
        if kind==64:ram[0x385:0x387]=(100).to_bytes(2,'little')
        ram[0x602:0x606]=bytes(4);ram[0x61C:0x61E]=power.to_bytes(2,'little')
        ram[0x635]=204;ram[0x637]=124;ram[0x5FE]=ram[0x638]=233;ram[0x61A]=255;ram[0x61B]=0
        source=out/(name+'-entry.cmjsession');source.write_bytes(keeper_checkpoint(ram,53))
        final,instructions=observe_keeper_comparison(rom[16:262160],ram)
        dest=out/(name+'-native.cmjsession')
        subprocess.run([binary,pack,str(source),'--resolve-prepared-keeper',str(action),str(defense_subtype),str(team),'--capture',str(dest)],capture_output=True,check=True,timeout=30)
        actual,wanted=dest.read_bytes(),keeper_checkpoint(final,56)
        if actual!=wanted:raise RuntimeError(f'{name}: mismatch {[(i,x,y)for i,(x,y)in enumerate(zip(actual,wanted))if x!=y]}; lengths{len(actual)}/{len(wanted)}')
        results.append({'case':name,'action':action,'type':kind,'random':[first,second],'power':power,'team':team,'story':story,'attack_subtype':attack_subtype,'result':final[0x612],'defense_subtype':final[0x43E],'effect':final[0x445],'input_defense_subtype':defense_subtype,'menu_context':context,'instructions':instructions,'match':True})
        if index==0:
            if subprocess.run([binary,pack,str(dest),'--resolve-prepared-keeper',str(action),str(defense_subtype),str(team)],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Keeper comparison replay accepted')
            interactive=out/'interactive.cmjsession'
            run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'resolve-prepared-keeper {action} 0 {team}\nsave {interactive}\nquit\n',text=True,capture_output=True,check=True,timeout=30)
            if 'Error:' in run.stdout or interactive.read_bytes()!=actual:raise RuntimeError('Interactive keeper comparison mismatch')
            for invalid in (['--resolve-prepared-keeper','4','0','4'],['--resolve-prepared-keeper','0','0','256'],['--resolve-prepared-keeper','0','0','4','--prepare-keeper-entry']):
                if subprocess.run([binary,pack,str(source),*invalid],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Invalid keeper comparison CLI accepted')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Explicit prepared state after keeper choice presentation. Original852F..8599 executes every instruction, including real stat-loader8/energy/comparison/story, stops before display9. No menu or frame effects inferred.'},indent=2)+'\n')
    print(f'{len(results)} prepared keeper comparisons match: {out}')


if __name__=='__main__':main()

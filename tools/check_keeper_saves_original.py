#!/usr/bin/env python3
"""Compare prepared non-goal keeper handlers, rewards and recovery handoffs.

Original861C/8646 executes through8640/865B/86AE without skipped calls. Display9
was explicitly acknowledged earlier. Goals/score and live keeper control excluded.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_keeper_entry_original import keeper_checkpoint
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def post_save_checkpoint(ram,phase):
    raw=bytearray(ram);raw[0x600:0x606]=bytes(6)
    data=bytearray(export_session(raw,0));data[10]=ord('i');data[258]=phase;data+=bytes(46)
    count=ram[0x600];data[28]=count;data[309:314]=ram[0x601:0x601+count]+bytes(5-count)
    return bytes(data)


def observe_keeper_save(prg,ram):
    code,action=ram[0x612],ram[0x43D]
    if code not in (2,3) or ram[0x616]:raise ValueError('Expected prepared normal keeper result2/3 at cursor0')
    memory=ObservedCartridge(prg,bytes(ram),None,set());memory.registers[6:8]=[26,27]
    memory[0x24],memory[0x25]=26,27;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc=0x861C if code==2 else 0x8646;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    stop=0x8640 if code==2 else 0x86AE if action==1 else 0x865B
    phase=2 if code==2 else 58 if action==1 else 57
    for instructions in range(50000):
        if cpu.pc==stop:return bytes(memory.ram[:2048]),phase,instructions
        if cpu.pc<0x8000:raise RuntimeError('Keeper save left original PRG')
        cpu.step()
    raise RuntimeError('Original keeper save instruction bound')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected ordinary side0 keeper RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());results=[]
    def native(source,operation,dest):
        subprocess.run([binary,pack,str(source),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
    cases=[]
    samples=[(0,0,250,49,0),(63,255,7,79,1),(64,128,255,80,0),(255,15,0,255,1)]
    for code in (2,3):
        for action in range(4):
            for kind in (1,33,64):
                for sample in samples:cases.append((code,action,kind,*sample))
    # All horizontal directions and both vertical signs in the punch branch.
    for first in range(8):
        for second in (0,15,128,255):cases.append((3,1,1,first,second,0,0,0))
    for index,(code,action,kind,first,second,timer,penalty,saturated) in enumerate(cases):
        name=f'case{index}';ram=bytearray(base)
        for address,value in ((0x612,code),(0x616,0),(0x43D,action),(0x43E,0),(0x43B,0),(0x43C,0),
          (0x600,1),(0x601,11),(0x442,11),(0x5FF,0),(0xE2,first),(0xE3,second),(0x384,kind),
          (0x389,timer),(0x38A,9),(0x38B,penalty),(0x38E,0),(0x443,index%4)):
            ram[address]=value
        ram[0x602:0x606]=bytes(4);ram[0x635]=204;ram[0x637]=124;ram[0x5FE]=ram[0x638]=233;ram[0x61A]=255;ram[0x61B]=0
        if kind==64:ram[0x385:0x387]=(100).to_bytes(2,'little')
        if saturated:ram[0x454:0x46A]=(65530).to_bytes(2,'little')*11
        source=out/(name+'-entry.cmjsession');source.write_bytes(keeper_checkpoint(ram,56))
        final,phase,instructions=observe_keeper_save(rom[16:262160],ram)
        dest=out/(name+'-native.cmjsession');native(source,['--resolve-keeper-save'],dest)
        actual,wanted=dest.read_bytes(),post_save_checkpoint(final,phase)
        if actual!=wanted:raise RuntimeError(f'{name} mismatch {[(i,x,y)for i,(x,y)in enumerate(zip(actual,wanted))if x!=y]}; lengths{len(actual)}/{len(wanted)}')
        record={'case':name,'result':code,'defense_action':action,'keeper_type':kind,'random':[first,second],'phase':phase,'instructions':instructions,'match':True}
        if code==3 and action==1 and index%4==0:
            report,recovered=trace_recovery(rom[16:262160],final,skip_jitter=True)
            (out/(name+'-recovery.json')).write_text(json.dumps(report,indent=2)+'\n')
            saved=out/(name+'-recovered.cmjsession');native(dest,['--recover-keeper-punch'],saved)
            if saved.read_bytes()!=post_save_checkpoint(recovered,6):raise RuntimeError(f'{name}: prepared no-jitter recovery mismatch')
            record['recovery_matches']=True
        if index==0:
            if subprocess.run([binary,pack,str(dest),'--resolve-keeper-save'],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Keeper reward/outcome replay accepted')
            interactive=out/'interactive.cmjsession'
            run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'resolve-keeper-save\nsave {interactive}\nquit\n',text=True,capture_output=True,check=True,timeout=30)
            if 'Error:' in run.stdout or interactive.read_bytes()!=actual:raise RuntimeError('Interactive keeper save mismatch')
        results.append(record)
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Explicit post-display9 non-goal result2/3 handlers through recovery/keeper-control handoff. Original reward/status/ownership/sound helpers all execute, no skipped calls; live sound rendering/control excluded. Selected punch cases additionally compare separately prepared8BE5 recovery.'},indent=2)+'\n')
    print(f'{len(results)} keeper save handlers match: {out}')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare saved flight initialization/first-cell boundaries with private original RAM."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_attack_clock_original import checkpoint
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0 or not base[0x600]:p.error('Expected ordinary selected shot RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must remain outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve())
    m=ObservedCartridge(rom[16:262160],bytes(2048),None,set())
    words=[m[0xFB4C+2*i]+256*m[0xFB4D+2*i] for i in range(64)]
    content=out/'projection test.cmjball';content.write_text('CMJBALL1\n'+' '.join(map(str,words))+'\n')
    def encode(ram,phase,loose):
        data=bytearray(checkpoint(ram,phase));data[339:344]=bytes([1,ram[0x62B],ram[0x638],ram[0x61A],int(loose)])
        if phase>=47:
            data[324:330]=bytes(6)
            trajectory=ram[0x634:0x638]+ram[0x639:0x63D]+bytes([ram[0x62C],ram[0x61B]])
            if phase==50:trajectory=trajectory[:4]+bytes(4)+trajectory[8:]
            data[344:355]=bytes([1])+trajectory
        return bytes(data)
    results=[];first_entry=None;first_final=None
    cases=[('ordinary',120,100,64,192,221,False),('toward-goal',160,96,255,1,233,False),
           ('unopposed',180,159,1,255,234,True),('next-cell-arrival',120,100,64,192,122,False),
           ('fractional-arrival',127,100,255,192,122,False),('already-arrived',196,124,64,192,221,False)]
    for name,x,y,lx,ly,dest,zero in cases:
        ram=bytearray(base);ram[0x634:0x638]=((x<<8)|lx).to_bytes(2,'little')+((y<<8)|ly).to_bytes(2,'little')
        ram[0x5FE]=((x-48)//8)*12+(y-80)//8;ram[0x638]=dest;ram[0x61A]=255;ram[0x61B]=0;ram[0x617]=0;ram[0x43C]=0
        if zero:ram[0x600:0x606]=bytes(6);ram[0x612]=0
        already=ram[0x5FE]==dest
        if already:ram[0x639:0x63D]=bytes([0x34,0x12,0xDC,0xFE])
        loose=x<128;entry=out/(name+'-entry.cmjsession');entry.write_bytes(encode(ram,46,loose))
        (out/(name+'-entry.bin')).write_bytes(ram)
        memory=ObservedCartridge(rom[16:262160],bytes(ram),None,set());memory.registers[6:8]=[26,27]
        cpu=MPU(memory=memory);cpu.pc=0xE73E;cpu.sp=0xFD;cpu.p=cpu.UNUSED
        for count in range(20000):
            if cpu.pc in (0xE773,0xE7CF):break
            cpu.step()
        else:raise RuntimeError('Original flight setup bound')
        phase=50 if already else 47;ready=out/(name+'-ready.cmjsession')
        subprocess.run([binary,pack,str(entry),'--begin-ball-flight',str(content),str(lx),str(ly),'--capture',str(ready)],capture_output=True,check=True,timeout=30)
        expected=encode(bytes(memory.ram[:2048]),phase,loose)
        actual=ready.read_bytes()
        if actual!=expected:raise RuntimeError(f'{name}setup mismatch{[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
        (out/(name+'-ready.bin')).write_bytes(memory.ram[:2048])
        if subprocess.run([binary,pack,str(ready),'--begin-ball-flight',str(content),'0','0'],capture_output=True).returncode==0:raise RuntimeError('Repeated setup accepted')
        if not already:
            cpu.pc=0xE778  # explicit post-wait numeric entry, not live frame execution
            for steps in range(2_000_000):
                if cpu.pc in (0xE7B4,0xE7CC):break
                cpu.step()
            else:raise RuntimeError('Original flight step bound')
            phase=48 if cpu.pc==0xE7B4 else 49;final=out/(name+'-final.cmjsession')
            subprocess.run([binary,pack,str(ready),'--advance-ball-cell','--capture',str(final)],capture_output=True,check=True,timeout=30)
            expected=encode(bytes(memory.ram[:2048]),phase,loose);actual=final.read_bytes()
            if actual!=expected:raise RuntimeError(f'{name}step mismatch{[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
            (out/(name+'-final.bin')).write_bytes(memory.ram[:2048])
            if subprocess.run([binary,pack,str(final),'--advance-ball-cell'],capture_output=True).returncode==0:raise RuntimeError('Pending callback replay accepted')
            if first_entry is None:first_entry,first_final=entry,final
        results.append({'case':name,'matches':True,'end_phase':phase,'already_arrived':already,'unused_original_velocity':list(memory.ram[0x639:0x63D]) if already else None})
    for bad in ['BAD\n','CMJBALL1\n'+ '0 '*63,'CMJBALL1\n'+ '0 '*64+'extra','CMJBALL1\n'+ '0 '*63+'65536','CMJBALL1\n'+ '0 '*63+'-1','CMJBALL1\n'+'-4294967295 '*64,'CMJBALL1\n'+'4294967296 '*64]:
        f=out/'bad.cmjball';f.write_text(bad)
        if subprocess.run([binary,pack,str(first_entry),'--begin-ball-flight',str(f),'64','192'],capture_output=True).returncode==0:raise RuntimeError('Malformed content accepted')
    interactive=out/'interactive.cmjsession'
    r=subprocess.run([binary,pack,str(first_entry),'--interactive'],input=f'begin-ball-flight {json.dumps(str(content))} 64 192\nadvance-ball-cell\nsave {interactive}\nquit\n',capture_output=True,text=True,check=True,timeout=30)
    if 'Error:' in r.stdout or interactive.read_bytes()!=first_final.read_bytes():raise RuntimeError('Interactive saved trajectory differs')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Original E73E setup + explicitly prepared post-wait E778 numeric body, no live frame/task/callback execution.324prior represented bytes plus flight data checked; native presence metadata excluded. Already-arrived stale velocity is unused and normalized to zero in native metadata, not claimed equal to original.'},indent=2)+'\n')
    print(f'{len(cases)} saved flight cases match original setup/first-cell boundaries; content/replay/interactive checks pass: {out}')


if __name__=='__main__':main()

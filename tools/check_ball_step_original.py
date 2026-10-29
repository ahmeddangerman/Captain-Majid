#!/usr/bin/env python3
"""Compare prepared numeric ball-cell steps against originalE778 loop."""
import argparse
import hashlib
import json
import random
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path);p.add_argument('output_dir',type=Path)
    p.add_argument('--compiler',default='c++');a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    root=Path(__file__).resolve().parents[1];out=a.output_dir.resolve()
    if out.is_relative_to(root):p.error('Private outputs must be outside checkout')
    out.mkdir(parents=True,exist_ok=False)
    cases=[(120*256+64,100*256+192,256,0,110,150),
           (120*256+64,100*256+192,256,0,110,122),
           (120*256+64,100*256+192,65535,0,110,150),
           (207*256+64,100*256+192,256,0,230,5),
           (120*256+64,100*256+192,0,0,110,110),
           (120*256+64,100*256+192,32768,0,110,233)]
    rng=random.Random(29)
    for _ in range(64):
        x,y=rng.randrange(48*256,208*256),rng.randrange(80*256,176*256)
        cell=((x//256-48)//8)*12+((y//256-80)//8)
        vx,vy=rng.choice([(256,128),(65535,0),(65024,512),(0,512),(2048,64512)])
        dest=rng.randrange(240);cases.append((x,y,vx,vy,cell,dest))
    # Also exercise native projection composed with stepping for all direction bytes.
    projection=ObservedCartridge(rom[16:262160],bytes(2048),None,set());project_cpu=MPU(memory=projection)
    words=[projection[0xFB4C+2*i]+256*projection[0xFB4D+2*i] for i in range(64)]
    def project(pc,angle):
        project_cpu.pc,project_cpu.sp,project_cpu.a,project_cpu.p=pc,0xFD,angle,project_cpu.UNUSED
        projection[0x1FE],projection[0x1FF]=255,1
        for _ in range(100):
            if project_cpu.pc==0x200:return project_cpu.x+256*project_cpu.y
            project_cpu.step()
        raise RuntimeError('Original projection instruction bound')
    directions=[256]*len(cases)
    for angle in range(256):
        vx,vy=project(0xCE4A,angle),project(0xCE4D,angle)
        cases.append((120*256+64,100*256+192,vx,vy,110,(angle*17)%240));directions.append(angle)
    driver=out/'native-ball-step.cpp';driver.write_text('''#include "captain/ball_step.hpp"
#include <iostream>
int main(){captain::BallVelocityContent table{};for(auto& v:table.quarter_wave){unsigned n;if(!(std::cin>>n)||n>65535)return 2;v=n;}unsigned x,y,vx,vy,c,d,angle;while(std::cin>>x>>y>>vx>>vy>>c>>d>>angle){if(angle<256){auto v=captain::calculate_ball_velocity(angle,table);vx=v.x;vy=v.y;}auto r=captain::advance_ball_to_next_cell({static_cast<unsigned short>(x),static_cast<unsigned short>(y),{static_cast<unsigned short>(vx),static_cast<unsigned short>(vy)},static_cast<unsigned char>(c),static_cast<unsigned char>(d)});std::cout<<r.x<<' '<<r.y<<' '<<unsigned(r.cell)<<' '<<r.additions<<' '<<unsigned(r.event)<<' '<<r.forced_destination<<'\\n';}}
''')
    binary=out/'native-ball-step';subprocess.run([a.compiler,'-std=c++17','-I'+str(root/'engine/include'),str(driver),str(root/'build/libcaptain-core.a'),'-o',str(binary)],capture_output=True,check=True,timeout=30)
    native=subprocess.run([str(binary)],input=' '.join(map(str,words))+'\n'+'\n'.join(' '.join(map(str,(*c,angle))) for c,angle in zip(cases,directions)),text=True,capture_output=True,check=True,timeout=30).stdout.splitlines()
    if len(native)!=len(cases):raise RuntimeError('Native result count mismatch')
    reports=[]
    for number,((x,y,vx,vy,cell,dest),line) in enumerate(zip(cases,native)):
        ram=bytearray(2048)
        for address,value in [(0x634,x),(0x636,y),(0x639,vx),(0x63B,vy)]:ram[address:address+2]=value.to_bytes(2,'little')
        ram[0x5FE]=cell;ram[0x638]=dest
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xE746 if cell==dest else 0xE778;cpu.sp=0xFD;cpu.p=cpu.UNUSED
        additions=0;forced=False
        for steps in range(2_000_000):
            if cpu.pc in (0xE7B4,0xE7CC,0xE7CF):break
            if cpu.pc==0xE778:additions+=1
            if cpu.pc==0xE7BA:forced=True
            cpu.step()
        else:raise RuntimeError('Original ball loop bound')
        event={0xE7B4:1,0xE7CC:2,0xE7CF:0}[cpu.pc]
        original=(m[0x634]+256*m[0x635],m[0x636]+256*m[0x637],m[0x5FE],additions,event,int(forced))
        actual=tuple(map(int,line.split()))
        if original!=actual:raise RuntimeError(f'case{number} {cases[number]}: original{original}, native{actual}')
        reports.append({'case':number,'native_direction':directions[number] if directions[number]<256 else None,'input':cases[number],'output':original,'instructions':steps,'match':True})
    (out/'verification.json').write_text(json.dumps({'cases':reports,'scope':'Prepared E778 supplied velocity/fraction inputs; already-arrived startsE746. Stops before cell/arrival callbacks. No skipped calls, frames or rendering.'},indent=2)+'\n')
    print(f'{len(cases)} original/native numeric ball steps match: {out}')


if __name__=='__main__':main()

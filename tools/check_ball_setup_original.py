#!/usr/bin/env python3
"""Compare composed native direction/projection with originalE73E preparation."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--compiler',default='c++');a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    root=Path(__file__).resolve().parents[1];out=a.output_dir.resolve()
    if out.is_relative_to(root):p.error('Private outputs must be outside checkout')
    out.mkdir(parents=True,exist_ok=False)
    m=ObservedCartridge(rom[16:262160],bytes(2048),None,set())
    words=[m[0xFB4C+2*i]+256*m[0xFB4D+2*i] for i in range(64)]
    driver=out/'setup-driver.cpp';driver.write_text('''#include "captain/shot_content.hpp"
#include "captain/ball_velocity.hpp"
#include "captain/player_movement.hpp"
#include <fstream>
#include <iostream>
int main(int argc,char**argv){if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);auto pack=captain::load_shot_content(f);if(!pack.movement)return 2;captain::BallVelocityContent table{};for(auto&v:table.quarter_wave){unsigned n;if(!(std::cin>>n)||n>65535)return 2;v=n;}unsigned x,y,c,d;while(std::cin>>x>>y>>c>>d){auto direction=captain::calculate_player_direction({static_cast<unsigned short>(x),static_cast<unsigned short>(y),static_cast<unsigned char>(d),0},c,pack.movement->geometry);auto v=captain::calculate_ball_velocity(direction,table);std::cout<<unsigned(direction)<<' '<<v.x<<' '<<v.y<<'\\n';}}
''')
    binary=out/'setup-driver';subprocess.run([a.compiler,'-std=c++17','-I'+str(root/'engine/include'),str(driver),str(root/'build/libcaptain-core.a'),'-o',str(binary)],capture_output=True,check=True,timeout=30)
    cases=[]
    for x,y,dest in [(120,100,221),(160,96,233),(180,159,234),(52,84,239),(204,172,0),(128,124,5),(128,124,233)]:
        cell=((x-48)//8)*12+((y-80)//8);cases.append((x*256+64,y*256+192,cell,dest))
    native=subprocess.run([str(binary),str(a.pack.resolve())],input=' '.join(map(str,words))+'\n'+'\n'.join(' '.join(map(str,c)) for c in cases),text=True,capture_output=True,check=True,timeout=30).stdout.splitlines()
    if len(native)!=len(cases):raise RuntimeError('Result count mismatch')
    reports=[]
    for n,((x,y,cell,dest),line) in enumerate(zip(cases,native)):
        ram=bytearray(2048);ram[0x634:0x638]=x.to_bytes(2,'little')+y.to_bytes(2,'little');ram[0x5FE]=cell;ram[0x638]=dest;ram[0x600]=3;ram[0x5FF]=7
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xE73E;cpu.sp=0xFD;cpu.p=cpu.UNUSED
        for step in range(10000):
            if cpu.pc==0xE773:break
            cpu.step()
        else:raise RuntimeError('Setup instruction bound')
        original=(m[0x62C],m[0x639]+256*m[0x63A],m[0x63B]+256*m[0x63C])
        if tuple(map(int,line.split()))!=original:raise RuntimeError(f'case{n}: native{line} original{original}')
        (out/f'{n}-entry.bin').write_bytes(ram);(out/f'{n}-final.bin').write_bytes(m.ram[:2048])
        reports.append({'case':n,'input':cases[n],'direction_vx_vy':original,'count_after':m[0x600],'elapsed_after':m[0x5FF],'instructions':step,'match':True})
    (out/'verification.json').write_text(json.dumps({'cases':reports,'scope':'Native direction+projection composed; originalE73E throughE773 no skippedcalls/frames. Prefix/elapsed resets observed only, not native phase integration.'},indent=2)+'\n')
    print(f'{len(cases)} composed native direction/velocity setup cases match: {out}')


if __name__=='__main__':main()

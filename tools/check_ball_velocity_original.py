#!/usr/bin/env python3
"""Exhaustive256-direction native/ROM ball projection comparison; private inputs."""
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
    p.add_argument('rom',type=Path);p.add_argument('output_dir',type=Path)
    p.add_argument('--compiler',default='c++')
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    root=Path(__file__).resolve().parents[1];out=a.output_dir.resolve()
    if out.is_relative_to(root):p.error('Private outputs must be outside checkout')
    out.mkdir(parents=True,exist_ok=False)
    m=ObservedCartridge(rom[16:262160],bytes(2048),None,set());cpu=MPU(memory=m)
    words=[m[0xFB4C+2*i]+256*m[0xFB4D+2*i] for i in range(64)]
    def call(pc,angle):
        cpu.pc,cpu.sp,cpu.a,cpu.p=pc,0xFD,angle,cpu.UNUSED
        m[0x1FE],m[0x1FF]=255,1
        for _ in range(100):
            if cpu.pc==0x200:return cpu.x+256*cpu.y
            if cpu.pc<0x8000:raise RuntimeError('Unexpected projection CPU address')
            cpu.step()
        raise RuntimeError('Projection CPU instruction bound')
    driver=out/'native-projection-driver.cpp'
    driver.write_text('''#include "captain/ball_velocity.hpp"
#include <iostream>
int main(){captain::BallVelocityContent c{};for(auto& v:c.quarter_wave){unsigned n;if(!(std::cin>>n)||n>65535)return 2;v=n;}
for(unsigned d=0;d<256;++d){auto v=captain::calculate_ball_velocity(d,c);std::cout<<v.x<<' '<<v.y<<'\\n';}}
''')
    binary=out/'native-projection-driver'
    subprocess.run([a.compiler,'-std=c++17','-I'+str(root/'engine/include'),str(driver),str(root/'build/libcaptain-core.a'),'-o',str(binary)],check=True,capture_output=True,timeout=30)
    actual=subprocess.run([str(binary)],input=' '.join(map(str,words)),capture_output=True,text=True,check=True,timeout=30).stdout.splitlines()
    if len(actual)!=256:raise RuntimeError('Expected256native directions')
    results=[]
    for angle,line in enumerate(actual):
        native=tuple(map(int,line.split()));original=(call(0xCE4A,angle),call(0xCE4D,angle))
        if native!=original:raise RuntimeError(f'direction{angle}: native{native}, original{original}')
        results.append({'direction':angle,'x':original[0],'y':original[1],'match':True})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'All256direction bytes; originalCE4A/CE4D instructions execute with no skipped calls or frames. ROM-derived table remains private. No flight/clock/frame claim.'},indent=2)+'\n')
    print(f'All256original/native ball velocity directions match: {out}')


if __name__=='__main__':main()

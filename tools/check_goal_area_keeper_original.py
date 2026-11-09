#!/usr/bin/env python3
"""Compare goal-area keeper selection with controlled original CPU cases.

Requires the user's private approved ROM and writes all generated content and
reports outside the checkout. Presentation and AI are not simulated.
"""
import argparse
import hashlib
import itertools
import json
import os
import subprocess
import sys
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


DRIVER = r'''#include "captain/defender_selection.hpp"
#include <array>
#include <iostream>
int main(){
 std::array<unsigned char,9> cells{};unsigned n;
 for(auto& cell:cells){if(!(std::cin>>n)||n>239)return 2;cell=static_cast<unsigned char>(n);}
 unsigned side,x,y,random,count,stun;
 while(std::cin>>side>>x>>y>>random>>count>>stun)
  std::cout<<captain::select_goal_area_keeper({static_cast<unsigned char>(side),static_cast<unsigned char>(x),
   static_cast<unsigned char>(y),static_cast<unsigned char>(random),static_cast<unsigned char>(count),stun!=0},cells)<<"\n";
}
'''


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    parser.add_argument('output',type=Path)
    parser.add_argument('--archive',type=Path,default=Path('build/libcaptain_core.a'))
    parser.add_argument('--compiler',default='c++')
    args=parser.parse_args()
    root=Path(__file__).resolve().parents[1]
    output=args.output.resolve()
    if output.is_relative_to(root):parser.error('Private evidence must remain outside the checkout')
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('Expected approved Japanese ROM')
    output.mkdir(parents=True,exist_ok=False)
    memory=ObservedCartridge(rom[16:262160],bytes(2048),None,set());memory.registers[6:8]=[26,27]
    cells=[memory[address] for address in range(0x84ef,0x84f8)]
    driver=output/'driver.cpp';driver.write_text(DRIVER,encoding='utf-8')
    executable=output/('driver.exe' if os.name=='nt' else 'driver')
    subprocess.run([args.compiler,'-std=c++17','-I'+str(root/'engine/include'),str(driver),
                    str(args.archive.resolve()),'-o',str(executable)],check=True,capture_output=True,text=True,timeout=120)
    cases=[]
    choices=itertools.product((0,11),(48,56,64,72,79,176,183,188,192,200,207),
        (96,104,112,120,124,128,136,144,152,159),(0,50,51,52,84,85,86,255),(0,1),(0,1,4,5))
    for side,x,y,random,stun,count in choices:
        if side==11 and count>4:continue
        ram=bytearray(2048);ram[0x5fb]=side;ram[0x635]=x;ram[0x637]=y;ram[0xe2]=random
        keeper=11 if side==0 else 0;ram[0x300+12*keeper+10]=stun;ram[0x600]=count
        ids=([12,13,14,15,16] if side==0 else [1,2,3,4,5]);ram[0x601:0x606]=bytes(ids)
        memory=ObservedCartridge(rom[16:262160],bytes(ram),None,set());memory.registers[6:8]=[26,27]
        memory[0x1fe],memory[0x1ff]=255,1
        cpu=MPU(memory=memory);cpu.pc=0x848f;cpu.sp=0xfd;cpu.p=cpu.UNUSED
        for _ in range(10000):
            if cpu.pc==0x200:break
            cpu.step()
        else:raise RuntimeError('Original goalkeeper selection exceeded its instruction bound')
        actual=list(memory.ram[0x601:0x601+memory[0x600]])
        inserted=len(actual)==count+1
        expected=([keeper]+ids[:count]) if inserted else ids[:count]
        if actual!=expected:raise ValueError(f'Original case {len(cases)} returned an unexpected prefix')
        cases.append((side,x,y,random,count,stun,inserted))
    source=' '.join(map(str,cells))+'\n'+'\n'.join(' '.join(map(str,c[:6])) for c in cases)+'\n'
    run=subprocess.run([str(executable)],input=source,text=True,capture_output=True,check=True,timeout=120)
    rows=run.stdout.splitlines()
    if len(rows)!=len(cases):raise ValueError(f'Native returned {len(rows)} rows for {len(cases)} cases')
    for index,(row,case) in enumerate(zip(rows,cases)):
        if row not in ('0','1') or bool(int(row))!=case[6]:
            raise ValueError(f'case {index}, boundary goal_area.keeper, selected: native {row}, original {int(case[6])}; input {case[:6]}')
    (output/'report.json').write_text(json.dumps({'cases':len(cases),'mismatches':0,
        'scope':'Controlled original CPU routine versus native; no frame, presentation or AI simulation.'},indent=2)+'\n')
    print(f'Matched {len(cases)} original goalkeeper-selection cases; evidence {output}')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,subprocess.SubprocessError) as error:
        print(f'Goal-area keeper comparison stopped: {error}',file=sys.stderr)
        sys.exit(1)

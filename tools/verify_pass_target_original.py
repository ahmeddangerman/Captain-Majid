#!/usr/bin/env python3
"""Compare bounded recipient-to-cell resolution against original CPU routine $E059."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from reference_cartridge import ReferenceCartridge
from reference_identity import PAYLOAD_SHA256


def iter_cases():
    for x in range(256):
        for y in range(256):
            yield 7,42,x,y
    for recipient in (*range(1,11),*range(12,22)):
        for x in (0,47,48,52,127,128,203,207,208,255):
            for y in (0,79,80,84,127,128,171,175,176,255):
                yield recipient,42,x,y
    for previous in range(256):
        yield 255,previous,0,0


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    parser.add_argument('--cxx',default='c++')
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if rom[:4]!=b'NES\x1a' or len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('ROM payload does not match the verified Japanese reference')
    from py65.devices.mpu6502 import MPU
    memory=ReferenceCartridge(rom[16:262160]);cpu=MPU(memory=memory)
    cases=list(iter_cases());expected=[]
    for recipient,previous,x,y in cases:
        memory[0x5fc],memory[0x638]=recipient,previous
        if recipient!=255:
            record=0x300+12*recipient
            memory[record+5],memory[record+6],memory[record+7],memory[record+8]=x^255,x,y^255,y
        memory[0x1fe],memory[0x1ff]=255,1
        cpu.pc,cpu.sp,cpu.p=0xe059,0xfd,cpu.UNUSED
        cpu.a=cpu.x=cpu.y=0
        for _ in range(500):
            if cpu.pc==0x200:break
            cpu.step()
        else:raise RuntimeError(f'Original target resolver did not return for case {len(expected)}')
        expected.append(memory[0x638])
    root=Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-pass-target-') as temp:
        binary=Path(temp)/'compare'
        subprocess.run([args.cxx,'-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        '-I'+str(root/'engine/include'),str(root/'engine/tests/pass_target_trace_driver.cpp'),
                        str(root/'engine/src/pass_target.cpp'),str(root/'engine/src/defender_selection.cpp'),
                        '-o',str(binary)],check=True,timeout=60)
        data=str(len(cases))+'\n'+'\n'.join(' '.join(map(str,case)) for case in cases)+'\n'
        result=subprocess.run([str(binary)],input=data,text=True,capture_output=True,check=True,timeout=60)
    actual=[int(line) for line in result.stdout.splitlines()]
    if len(actual)!=len(expected):raise RuntimeError(f'Native output count {len(actual)} != {len(expected)}')
    for index,(want,got) in enumerate(zip(expected,actual)):
        if want!=got:raise AssertionError(f'Case {index}, input {cases[index]}: original {want}, native {got}')
    print(f'Pass target cell matched {len(cases)} controlled original-CPU cases; field IDs and sentinel255 only.')


if __name__=='__main__':
    main()

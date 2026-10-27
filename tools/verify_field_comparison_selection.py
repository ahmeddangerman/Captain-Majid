#!/usr/bin/env python3
"""Compare original $834C..$837F selection, comparison and classification."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path
from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256
from verify_field_defender_preparation import word


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('ROM payload mismatch')
    prg=rom[16:262160]
    mem=ReferenceCartridge(prg)
    pointer=word(mem,0x8D95)
    content=mem[0x83D7:0x83DC]+mem[0x83DD:0x83E2]+mem[0x83E1:0x84E1]
    content+=mem[pointer:pointer+256]+mem[0x83F1:0x83F5]
    cases=[(a,d,slot) for a in range(5) for d in range(5) for slot in range(22)]
    expected=[]
    for a,d,slot in cases:
        mem=ReferenceCartridge(prg);cpu=MPU(memory=mem)
        mem[0x43B],mem[0x43D],mem[0x442]=a,d,slot
        power=200+slot*64
        mem[0x32:0x34]=[240,0]
        mem[0x61C:0x61E]=[power&255,power>>8]
        mem[0xE2],mem[0xE3]=(slot*31+a*17)&255,(d*53+slot*11)&255
        mem[0x3A],mem[0x43E]=(slot%2)*128,3
        mem[0x621]=4 if slot in (0,11) else 0
        cpu.pc,cpu.p=0x834C,cpu.UNUSED
        for _ in range(64):
            if cpu.pc==0x8373:
                if cpu.a!=1 or mem.bank_writes:
                    raise AssertionError('Unexpected selector/mapper state')
                offset=mem[0x3B]
                selection=(cpu.x,mem[pointer+offset],mem[pointer+offset+1],
                           int(bool(cpu.p&cpu.NEGATIVE)),*mem[0x83F1:0x83F5])
                break
            cpu.step()
        else:raise RuntimeError(f'Selection did not finish: ${cpu.pc:04X}')
        if selection[1]:
            special=False
            for _ in range(5000):
                if cpu.pc==0x837F:
                    expected.append((*selection,1,word(mem,0x32),mem[0x71],mem[0x619],
                                     cpu.a,cpu.y,mem[0x43E],int(special)))
                    break
                special=special or cpu.pc==0x8D3A
                cpu.step()
            else:raise RuntimeError(f'Comparison did not finish: ${cpu.pc:04X}')
        else:expected.append((*selection,0,0,0,0,0,0,0,0))
    root=Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-field-select-') as temp:
        binary=Path(temp)/'native'
        subprocess.run(['c++','-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        '-I'+str(root/'engine/include'),str(root/'engine/tests/field_selection_trace_driver.cpp'),
                        str(root/'engine/src/field_comparison_content.cpp'),str(root/'engine/src/defender_contact.cpp'),str(root/'engine/src/action_strength.cpp'),'-o',str(binary)],check=True,timeout=60)
        data=' '.join(map(str,content))+'\n'+str(len(cases))+'\n'
        data+='\n'.join(' '.join(map(str,c)) for c in cases)+'\n'
        run=subprocess.run([str(binary)],input=data,text=True,capture_output=True,check=True,timeout=30)
    actual=[tuple(map(int,line.split())) for line in run.stdout.splitlines()]
    if len(actual)!=len(expected):raise AssertionError('Output count mismatch')
    for i,(want,got) in enumerate(zip(expected,actual)):
        if want!=got:raise AssertionError(f'{cases[i]}: ROM={want}, native={got}')
    print(f'Field comparison selection: {len(cases)} original fixtures matched including {sum(row[8] for row in expected)} original caller arithmetic/classification paths; zero-range entries are selection-only.')


if __name__=='__main__':main()

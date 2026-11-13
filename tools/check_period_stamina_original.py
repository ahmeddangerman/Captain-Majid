#!/usr/bin/env python3
"""Compare the original interval stamina routine with the native implementation.

Fixtures use a captured roster but deliberately vary period, player type and stamina.
They do not simulate the interval presentation, input, audio, NMI or the next kickoff.
"""
import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','pack','session','executable','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();rom=args.rom.read_bytes();raw=args.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('Expected approved Japanese ROM')
    if len(raw)!=2048:parser.error('Expected 2048-byte captured interval RAM')
    if args.output.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private fixtures must stay outside the checkout')
    rng=random.Random(20261113);records=bytearray();prg=rom[16:262160]
    for period in (0,1,2,3):
        for case in range(128):
            memory=ObservedCartridge(prg,raw,None,set());memory.registers[6:8]=[26,27]
            memory[0x22],memory[0x23],memory[0x24],memory[0x25]=0,7,26,27;memory.select=7
            memory[0x27]=period;memory[0x1fe],memory[0x1ff]=255,1
            if case%2:memory[0x300+12*3]=32
            types=bytes(memory[0x300+12*id] for id in range(11));before=bytearray()
            for id in range(11):
                stamina=rng.randrange(1201);address=0x301+12*id
                memory[address],memory[address+1]=stamina&255,stamina>>8
                before.extend((stamina&255,stamina>>8))
            cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xd022,0xfd,cpu.UNUSED
            for _ in range(100000):
                if cpu.pc==0x200:break
                if cpu.pc<0x8000:raise RuntimeError('Original routine left PRG')
                cpu.step()
            else:raise RuntimeError('Original routine exceeded instruction bound')
            after=bytes(memory[0x301+12*id+part] for id in range(11) for part in (0,1))
            records.extend(bytes((period,))+types+before+after)
    args.output.write_bytes(records)
    subprocess.run([str(args.executable.resolve()),'--period-stamina',str(args.pack.resolve()),
                    str(args.session.resolve()),str(args.output.resolve())],check=True)


if __name__=='__main__':main()

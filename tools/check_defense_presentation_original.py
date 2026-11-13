#!/usr/bin/env python3
"""Compare original defender-presentation selection for all random bytes."""
import argparse
import hashlib
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','executable','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected approved Japanese ROM')
    if args.output.resolve().is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Private fixtures must stay outside checkout')
    memory=ReferenceCartridge(rom[16:262160]);memory.registers[6:8]=[26,27];cases=bytearray()
    for count in range(1,5):
        for random in range(256):
            memory[0xe2]=random;memory[0x600]=count
            cpu=MPU(memory=memory);cpu.pc,cpu.p=0x8062,cpu.UNUSED
            for _ in range(100):
                if cpu.pc==0x8074:break
                cpu.step()
            else:raise RuntimeError('Original defender selection exceeded instruction bound')
            cases.extend((random,count,memory[0x617]))
    args.output.write_bytes(cases)
    subprocess.run([str(args.executable.resolve()),'--defense-slot',str(args.output.resolve())],check=True)


if __name__=='__main__':main()

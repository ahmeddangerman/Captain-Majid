#!/usr/bin/env python3
"""Compare first-match lineup and stamina with isolated original CPU routines.

The starting RAM and levels are explicit private inputs. This does not run the
original presentation or establish an entire match bootstrap.
"""
import argparse
import hashlib
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','captured','pack','exe','session'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();raw=bytearray(a.ram.read_bytes());expected=a.captured.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256 or len(raw)!=2048 or len(expected)!=2048:
        p.error('Expected verified Japanese ROM and private 2048-byte RAM captures')
    if raw[0x2a]!=0 or any(raw[0x303+12*i] for i in range(11)):
        p.error('Expected the captured initial campaign and level-zero roster')
    pack=a.pack.read_bytes()
    if pack[:8] not in (b'CMJSHOTY',b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb',b'CMJSHOTc'):p.error('Expected a first-match Y or Z pack')
    for i in range(11):raw[0x300+12*i:0x303+12*i]=bytes(3)
    memory=ObservedCartridge(rom[16:262160],raw,None,set())
    def run(pc,banks,x=0):
        memory.registers[6:8]=banks;memory[0x24],memory[0x25]=banks
        memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
        memory[0x1fe],memory[0x1ff]=0x28,0x86
        cpu=MPU(memory=memory);cpu.pc=pc;cpu.sp=0xfd;cpu.p=cpu.UNUSED;cpu.x=x
        for _ in range(50000):
            if cpu.pc==0x8629:return
            if cpu.pc<0x8000:raise RuntimeError(f'Unexpected CPU boundary {cpu.pc:04x}')
            memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
        raise RuntimeError('Original roster routine exceeded instruction bound')
    run(0xa8b7,[0,2]);run(0xdb62,[0,1])
    for slot in range(22):
        off=0x300+12*slot
        if bytes(memory.ram[off:off+4])!=expected[off:off+4]:
            raise RuntimeError(f'Original roster differs from captured startup at player {slot}')
    if pack[:8] in (b'CMJSHOTb',b'CMJSHOTc'):
        stats=ReferenceCartridge(rom[16:262160]);stats.registers[6:8]=[28,29]
        rows=1+max(stats[0x9662+12*((kind-35)&255)+5] for kind in range(1,121))
        trailer=121+2+48*rows+(7 if pack[:8]==b'CMJSHOTc' else 0);lineup=pack[-21-trailer:-10-trailer]
    elif pack[:8]==b'CMJSHOTa':lineup=pack[-21:-10]
    elif pack[:8]==b'CMJSHOTZ':lineup=pack[-14:-3]
    else:lineup=pack[-11:]
    if lineup!=bytes(memory.ram[0x300+12*i] for i in range(11)):
        raise RuntimeError('Y pack lineup differs from original roster writes')
    result=subprocess.run([str(a.exe.resolve()),'--check-first-match-roster',str(a.session.resolve()),str(a.pack.resolve())],
                          capture_output=True,text=True,check=True)
    print(result.stdout.strip())
    print('Original lineup and stamina routines matched the emulator capture for all 22 roster records.')


if __name__=='__main__':main()

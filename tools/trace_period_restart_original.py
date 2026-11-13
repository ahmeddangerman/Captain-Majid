#!/usr/bin/env python3
"""Trace one controlled interval into the next original kickoff.

The host acknowledges waits, clears unimplemented presentation queues, supplies
menu inputs and stubs PPU/mapper writes. This is not live presentation or NMI.
All original RAM checkpoints are written outside the checkout.
"""
import argparse
import hashlib
import json
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


class PeriodCartridge(ObservedCartridge):
    def __getitem__(self,address):
        if address==0x2002:return 128
        return super().__getitem__(address)
    def __setitem__(self,address,value):
        if address in (0xe000,0xa000) or 0x2000<=address<=0x2007:return
        return super().__setitem__(address,value)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','output-dir'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();rom=args.rom.read_bytes();raw=args.ram.read_bytes();out=args.output_dir.resolve()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected approved Japanese ROM')
    if len(raw)!=2048:parser.error('Expected 2048-byte period-end RAM')
    if out.is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Original RAM must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False)
    memory=PeriodCartridge(rom[16:262160],raw,None,set());memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xda98,0x50,cpu.UNUSED
    boundaries={(30,0xd022),(30,0xd092),(30,0xdaaa),(30,0xdae9),(30,0xdaf5),
                (30,0xdb0b),(30,0xdb0e),(30,0xdb1e),(31,0xe0df),(31,0xe145)}
    seen=[];frames=0;menu_polls=0;entered_dispatch=False
    for step in range(5000000):
        bank=memory.bank_at(cpu.pc)
        if bank==30 and cpu.pc==0xc400:entered_dispatch=True
        if entered_dispatch and (bank,cpu.pc) in boundaries:
            name=f'period-restart-{bank:02d}-{cpu.pc:04x}.bin'
            if name not in seen:(out/name).write_bytes(memory.ram[:2048]);seen.append(name)
            if (bank,cpu.pc)==(31,0xe145):break
        if cpu.pc==0xcaae or (bank==0 and cpu.pc==0x9f04):
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=160
            if bank==0 and cpu.pc==0x9f04:memory[0x4d]=memory[0x4e]=0;memory[0x628]=memory[0x629]=0
            if memory[0x498]:memory[0x498]-=1
            frames+=1
            if frames>2000:raise RuntimeError('Presentation acknowledgement bound reached')
        if entered_dispatch and bank==0 and cpu.pc==0x9bed and memory[0x27]==1:
            menu_polls+=1;memory[0x1e]=4 if menu_polls%2 else 128
        if cpu.pc<0x8000:raise RuntimeError('Original execution left PRG')
        memory.instruction=(bank,cpu.pc);cpu.step()
    else:raise RuntimeError('Original instruction bound reached')
    if len(seen)!=len(boundaries) or menu_polls!=2:raise RuntimeError('Unexpected interval route')
    (out/'trace.json').write_text(json.dumps({'steps':step,'host_waits':frames,'menu_polls':menu_polls,
        'boundaries':seen,'scope':'Controlled CPU trace; presentation and NMI are not emulated.'},indent=2)+'\n')
    print(f'Captured {len(seen)} interval and restart boundaries after {frames} host waits')


if __name__=='__main__':main()

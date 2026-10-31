#!/usr/bin/env python3
"""Observe one sampled chooser input after the original cooperative yield."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_special_poll(prg,ram,pressed,cursor,option):
    if len(ram)!=2048 or not 0<=pressed<=255 or cursor not in (0,1) or not 0<=option<=127:
        raise ValueError('Expected chooser sample and one-option state')
    m=ObservedCartridge(prg,ram,None,set());m.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,28),(4,29),(5,1),(6,0x4c),(7,28),(8,29),
                          (0x191,0x85),(0x192,0xeb),(0x24,28),(0x25,29)):
        m[address]=value
    m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[28,29]
    # CF8F returns to D785; D77A returns to D3F4 (6502 return addresses are minus1).
    m[0x14d:0x151]=[0x84,0xd7,0xf3,0xd3]
    m[0x1c]=m[0x1e]=pressed;m[0x622]=cursor;m[0x623]=1;m[0x430]=1;m[0x431]=option
    cpu=MPU(memory=m);cpu.pc,cpu.sp,cpu.p=0xcfa9,0x4c,cpu.UNUSED
    for step in range(5000):
        bank=m.bank_at(cpu.pc)
        if bank==30 and cpu.pc in (0xcfa4,0xd3f4):
            return {'boundary':'wait' if cpu.pc==0xcfa4 else 'decided',
                    'instructions':step,'cursor':m[0x622], 'y':m[0x2fc],
                    'result':cpu.a if cpu.pc==0xd3f4 else None},bytes(m.ram[:2048])
        if cpu.pc<0x8000:raise RuntimeError(f'Chooser input left PRG at {cpu.pc:04x}')
        m.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Chooser input instruction bound')

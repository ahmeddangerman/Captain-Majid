#!/usr/bin/env python3
"""Observe original special redraw followed by the saved-input menu tail."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_special_tail(prg,ram,pressed,cursor,option):
    if len(ram)!=2048 or ram[0x22]&64 or not pressed&192 or cursor not in (0,1) or not 0<=option<=127:
        raise ValueError('Expected completed special chooser and valid captured RAM')
    m=ObservedCartridge(prg,ram,None,set());m.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,28),(4,29),(5,1),(6,0x50),(7,28),(8,29),
                          (0x191,0x85),(0x192,0xeb),(0x24,28),(0x25,29)):
        m[address]=value
    m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[28,29]
    m[0x151:0x155]=[0x67,0xd5,0x8e,0x8b];m[0x1c]=m[0x1e]=pressed
    cpu=MPU(memory=m);cpu.pc,cpu.sp,cpu.p=0xd3f4,0x50,cpu.UNUSED
    cpu.a=(option if cursor else 0) if pressed&128 else 128
    after_redraw=False;requests=[];frames=0
    for step in range(500000):
        bank=m.bank_at(cpu.pc)
        if bank==30 and cpu.pc==0xd41f:after_redraw=True
        if after_redraw and bank==30 and cpu.pc==0xd3a6:
            return {'boundary':'poll_wait','instructions':step,'frames':frames,'requests':requests},bytes(m.ram[:2048])
        if after_redraw:
            if bank==30 and cpu.pc==0xd3a3:requests.append({'kind':0,'id':cpu.a,'slot':0})
            elif cpu.pc==0xe93d:requests.append({'kind':3,'id':cpu.a,'slot':cpu.x})
        if cpu.pc<0x8000:raise RuntimeError(f'Special tail left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            m.instruction=None;m[0x1b]|=128;m[0x515]=0;m[0x1c]=m[0x1e]=pressed
            if m[0x498]:m[0x498]-=1
            frames+=1
        m.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError(f'Special tail instruction bound at {cpu.pc:04x}')

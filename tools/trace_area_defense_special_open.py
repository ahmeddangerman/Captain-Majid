#!/usr/bin/env python3
"""Observe original confirmed area defense through its special chooser opening."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_special_open(prg, ram, pressed):
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x5fb]!=11 or ram[0x621]!=2 or not 1<=ram[0x600]<=4 or ram[0x61e]>=ram[0x600] or not pressed&128:
        raise ValueError('Expected confirmed route2 human defensive choice')
    m=ObservedCartridge(prg,ram,None,set());m.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        m[address]=value
    m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27]
    m[0x151:0x155]=[0x67,0xd5,0x8e,0x8b]
    m[0x1c],m[0x1e]=pressed,pressed
    cpu=MPU(memory=m);cpu.pc,cpu.sp,cpu.p=0xd3d4,0x50,cpu.UNUSED
    cpu.y=m[0x61e];cpu.x=m[0x60b+cpu.y];cpu.a=m[0x601+cpu.y]
    m[0x43d],m[0x442]=cpu.x,cpu.a
    requests=[];frames=0
    for step in range(500000):
        bank=m.bank_at(cpu.pc)
        if bank==30 and cpu.pc==0xcfa4:
            if m[0x430]!=1 or m[0x622]!=0 or m[0x623]!=1:
                raise RuntimeError('Expected one-option chooser at cursor zero')
            return {'boundary':'special_open','instructions':step,'frames':frames,'requests':requests,
                    'option':m[0x431], 'tile':m[0x2ff], 'x':m[0x2fd], 'attribute':m[0x2fe],
                    'old_y':m[0x2fc]},bytes(m.ram[:2048])
        if bank==30 and cpu.pc==0xd3ee:
            requests.append({'kind':0,'id':cpu.a,'slot':0})
        if cpu.pc<0x8000:raise RuntimeError('Special chooser opening left original PRG')
        if cpu.pc==0xcaae:
            m.instruction=None;m[0x1b]|=128;m[0x515]=0;m[0x1c],m[0x1e]=pressed,pressed
            if m[0x498]:m[0x498]-=1
            frames+=1
        m.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Special chooser opening instruction bound')

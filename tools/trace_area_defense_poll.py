#!/usr/bin/env python3
"""Observe one sampled defensive UI poll, stopping before preparation or yield."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_defense_poll(prg,ram,pressed,*,preparation=False):
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x5fb]!=11 or ram[0x621]!=2 or not 1<=ram[0x600]<=4 or ram[0x61e]>ram[0x600]:
        raise ValueError('Expected active route2 human defensive state')
    if preparation and (ram[0x61e]>=ram[0x600] or ram[0x60b+ram[0x61e]]==255 or not pressed&128):
        raise ValueError('Expected confirmed active defensive choice')
    m=ObservedCartridge(prg,ram,None,set());m.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        m[address]=value
    m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27]
    # Explicit host parent returns: D573 -> D568 bank restore -> area caller8B8F.
    # The resumed sample uses SP50 rather than retaining the observer's CPU object.
    m[0x151:0x155]=[0x67,0xd5,0x8e,0x8b]
    m[0x1c],m[0x1e]=pressed,pressed
    cpu=MPU(memory=m);cpu.pc,cpu.sp,cpu.p=0xd3d4 if preparation else 0xd3ab,0x50,cpu.UNUSED
    if preparation:
        cpu.y=m[0x61e];cpu.x=m[0x60b+cpu.y];cpu.a=m[0x601+cpu.y]
        m[0x43d],m[0x442]=cpu.x,cpu.a
    requests=[];frames=0
    for step in range(500000):
        bank=m.bank_at(cpu.pc)
        if (bank==30 and (cpu.pc==0xd3a6 or (cpu.pc==0xd3d4 and not preparation))) or (bank==26 and cpu.pc==0x8b8f):
            return {'boundary':'complete' if bank==26 else 'preparation_pending' if cpu.pc==0xd3d4 else 'poll_wait',
                    'instructions':step,'frames':frames,'requests':requests},bytes(m.ram[:2048])
        if cpu.pc==0xe93d:requests.append({'kind':3,'id':cpu.a,'slot':cpu.x})
        elif bank==30 and cpu.pc==0xd3a3:requests.append({'kind':0,'id':cpu.a,'slot':0})
        elif bank==30 and cpu.pc==0xcc46:requests.append({'kind':4,'id':0,'slot':0})
        if cpu.pc<0x8000:raise RuntimeError('Defensive poll left original PRG')
        if cpu.pc==0xcaae:
            m.instruction=None;m[0x1b]|=128;m[0x515]=0;m[0x1c],m[0x1e]=pressed,pressed
            if m[0x498]:m[0x498]-=1
            frames+=1
        m.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Defensive poll instruction bound')

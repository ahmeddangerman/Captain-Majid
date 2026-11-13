#!/usr/bin/env python3
"""Observe a field-contact defense prompt; frame acknowledgements omit hardware."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_field_defense_prompt(prg,raw):
    if len(raw)!=2048 or raw[0x22]&64 or raw[0x5fb]!=11 or not 1<=raw[0x600]<=4:
        raise ValueError('Expected an opposing field-contact state')
    memory=ObservedCartridge(prg,raw,None,set());memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0xe4be,0x50,cpu.UNUSED;frames=0
    for step in range(500000):
        bank=memory.bank_at(cpu.pc)
        if bank==30 and cpu.pc==0xd3a6:return {'frames':frames,'instructions':step},bytes(memory.ram[:2048])
        if cpu.pc<0x8000:raise RuntimeError('Original defense prompt left PRG')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=0
            if memory[0x498]:memory[0x498]-=1
            frames+=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Original defense prompt exceeded instruction bound')

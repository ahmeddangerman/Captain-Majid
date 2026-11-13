#!/usr/bin/env python3
"""Observe a controlled goal-area shot to goalkeeper entry, without live PPU/NMI."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_area_shot(prg,raw,first,second):
    if len(raw)!=2048 or raw[0x22]&64 or raw[0x5fb]!=11 or raw[0x621]!=1:
        raise ValueError('Expected an opposing goal-area defensive menu result')
    memory=ObservedCartridge(prg,raw,None,set());memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x8b8f,0x50,cpu.UNUSED;captured={};counts={};frames=0
    targets={(26,0x829d),(26,0x82a0),(26,0x83f5),(26,0x83f8),(31,0xe773),(31,0xe7cc),(30,0xdd1c)}
    for _ in range(1000000):
        bank=memory.bank_at(cpu.pc)
        if bank==26 and cpu.pc==0x8298:memory[0xe2],memory[0xe3]=first,second
        if (bank,cpu.pc) in targets:
            key=f'{cpu.pc:04x}';counts[key]=counts.get(key,0)+1
            captured[f'{key}-{counts[key]}']=bytes(memory.ram[:2048])
            if cpu.pc==0xdd1c:return captured
        if cpu.pc<0x8000:raise RuntimeError('Original area shot left PRG')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=128
            if memory[0x498]:memory[0x498]-=1
            frames+=1
            if frames>1000:raise RuntimeError('Original area shot exceeded frame acknowledgement bound')
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Original area shot exceeded instruction bound')

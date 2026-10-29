#!/usr/bin/env python3
"""Observe prepared route2 keeper-area menu entry before attacking or defensive choices."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_keeper_area_menu(prg, ram, defense_setup=False, defense_prompt=False):
    if defense_prompt:defense_setup=True
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x621]!=2:
        raise ValueError('Expected route2 keeper-area prepared RAM')
    memory=ObservedCartridge(prg,ram,None,{0x517,0x532,0x8e,0x469,0x44e,0x62d,0x628,0x61e})
    memory.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                          (0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
        memory[address]=value
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23];memory.registers[6:8]=[26,27]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x8b73,0x50,cpu.UNUSED
    frames=0;requests=[];prompt_started=False
    for step in range(500000):
        bank=memory.bank_at(cpu.pc)
        if (bank==30 and (cpu.pc==0xd5e5 or (cpu.pc==0xd36e and not defense_setup) or (cpu.pc==0xd390 and defense_setup and not defense_prompt) or (cpu.pc==0xd3a6 and defense_prompt))) or (defense_setup and bank==26 and cpu.pc==0x8b8f):
            return {'boundary':'human_menu' if cpu.pc==0xd5e5 else 'defense_prompt' if cpu.pc==0xd3a6 else 'defense_initial' if cpu.pc==0xd390 else 'choices_done' if cpu.pc==0x8b8f else 'defensive_choice',
                    'instructions':step,'frames':frames,'writes':memory.writes,'presentation_requests':requests},bytes(memory.ram[:2048])
        if bank==30 and cpu.pc==0xd390 and defense_prompt:prompt_started=True
        if prompt_started:
            if bank==30 and cpu.pc in (0xd4f0,0xd3a3):requests.append({'kind':0,'id':cpu.a,'slot':0})
            elif cpu.pc==0xe93d:requests.append({'kind':3,'id':cpu.a,'slot':cpu.x})
        if bank==26 and cpu.pc==0x8b86:requests.append({'kind':1,'id':cpu.a})
        if bank==30:
            if cpu.pc in (0xd586,0xd5af):requests.append({'kind':1,'id':cpu.a})
            elif cpu.pc==0xd5b2:requests.append({'kind':4,'id':0})
            elif cpu.pc in (0xd5d4,0xd5dd):requests.append({'kind':0,'id':cpu.a})
        if cpu.pc<0x8000:raise RuntimeError('Keeper-area menu left original PRG')
        if cpu.pc==0xcaae:
            memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0
            memory[0x1c],memory[0x1e]=0,0
            if memory[0x498]:memory[0x498]-=1
            frames+=1
        memory.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError('Keeper-area menu instruction bound')

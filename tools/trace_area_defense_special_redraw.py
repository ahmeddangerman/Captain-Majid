#!/usr/bin/env python3
"""Observe special choice resolution through original defensive redraw."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_special_redraw(prg,ram,pressed,cursor,option):
    if len(ram)!=2048 or ram[0x22]&64 or not pressed&192 or cursor not in (0,1) or not 0<=option<=127:
        raise ValueError('Expected a completed special chooser and valid captured RAM')
    m=ObservedCartridge(prg,ram,None,set());m.ram[:0x1a]=bytes(0x1a)
    for address,value in ((0,5),(1,255),(2,0x90),(3,28),(4,29),(5,1),(6,0x50),(7,28),(8,29),
                          (0x191,0x85),(0x192,0xeb),(0x24,28),(0x25,29)):
        m[address]=value
    m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[28,29]
    m[0x151:0x155]=[0x67,0xd5,0x8e,0x8b];m[0x1c]=m[0x1e]=pressed
    cpu=MPU(memory=m);cpu.pc,cpu.sp,cpu.p=0xd3f4,0x50,cpu.UNUSED
    cpu.a=(option if cursor else 0) if pressed&128 else 128
    requests=[];frames=0;decision=None;redrawing=False
    for step in range(500000):
        bank=m.bank_at(cpu.pc)
        if bank==30 and cpu.pc in (0xd403,0xd40c) and decision is None:
            canceled=not bool(pressed&128)
            decision={'status':'accepted' if cpu.pc==0xd403 else 'canceled' if canceled else 'denied',
                      'subtype':0 if canceled else m[0x43e],
                      'cost':0 if canceled else m[0x43f]|m[0x440]<<8,
                      'effect':0 if canceled else m[0x445]}
        if bank==30 and cpu.pc==0xd40c:redrawing=True
        if bank==30 and cpu.pc==0xd41f:
            if decision is None or not redrawing:raise RuntimeError('Missing original resolution or redraw')
            return {'boundary':'redrawn','instructions':step,'frames':frames,'decision':decision,
                    'requests':requests},bytes(m.ram[:2048])
        if cpu.pc==0xcbb0 and not redrawing:requests.append({'kind':1,'id':cpu.a,'slot':0})
        if redrawing:
            if cpu.pc==0xcc46:requests.append({'kind':4,'id':0,'slot':0})
            elif bank==30 and cpu.pc in (0xd4f0,0xd41c):requests.append({'kind':0,'id':cpu.a,'slot':0})
            elif cpu.pc==0xe93d:requests.append({'kind':3,'id':cpu.a,'slot':cpu.x})
        if cpu.pc<0x8000:raise RuntimeError(f'Special redraw left PRG at {cpu.pc:04x}')
        if cpu.pc==0xcaae:
            m.instruction=None;m[0x1b]|=128;m[0x515]=0;m[0x1c]=m[0x1e]=pressed
            if m[0x498]:m[0x498]-=1
            frames+=1
        m.instruction=(bank,cpu.pc);cpu.step()
    raise RuntimeError(f'Special redraw instruction bound at {cpu.pc:04x}')

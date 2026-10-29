#!/usr/bin/env python3
"""Bounded originalDCFD throughDD08 shot distance/destination observation."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_approach(prg, ram):
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x5FB] not in (0,11):
        raise ValueError('Expected supported explicit shot approach RAM')
    m=ObservedCartridge(prg,ram,None,set());m.registers[6:8]=[26,27]
    m[0x24],m[0x25]=26,27;m[0x23]=m[0x22]|7;m.select=m[0x23]
    cpu=MPU(memory=m);cpu.pc=0xDCFD;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    for step in range(2000):
        if cpu.pc==0xDD08:
            return {'entry':0xDCFD,'exit':cpu.pc,'instructions':step,'distance':m[0x62B],
                    'destination':m[0x638],'counter':m[0x61A],'loose_ball':bool(cpu.p&1),
                    'scope':'Constructed entry; every called original instruction executes. No scripts, frames, audio or rendering.'},bytes(m.ram[:2048])
        if cpu.pc<0x8000:raise RuntimeError('Unexpected shot approach execution outside PRG')
        cpu.step()
    raise RuntimeError('Shot approach instruction bound reached')

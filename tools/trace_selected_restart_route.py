#!/usr/bin/env python3
"""Bounded original9366 routing before field/goal-area restart setup."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge


def trace_restart(prg,ram):
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x612]!=4 or ram[0x5FB] not in (0,11):
        raise ValueError('Expected captured selected-result4 RAM')
    memory=ObservedCartridge(prg,ram,None,set())
    memory.registers[6:8]=[26,27]
    memory[0x24],memory[0x25]=26,27
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc=0x9366;cpu.sp=0xFD;cpu.p=cpu.UNUSED
    for step in range(1000):
        if cpu.pc in (0x9379,0x94CF):
            return {'entry':0x9366,'exit':cpu.pc,'instructions':step,
                'route':'field_restart' if cpu.pc==0x9379 else 'goal_area_restart',
                'scope':'Constructed CPU entry; no skipped calls, scripts, frames or presentation.'},bytes(memory.ram[:2048])
        if cpu.pc<0x8000:raise RuntimeError('Unexpected restart execution outside PRG')
        cpu.step()
    raise RuntimeError('Restart route instruction bound reached')

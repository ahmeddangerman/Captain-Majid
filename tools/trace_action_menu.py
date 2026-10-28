#!/usr/bin/env python3
"""Bounded private observations of human menu and prepared ordinary-shot boundaries."""
import argparse
import hashlib
import json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def trace_menu(prg, ram, mode='open', pressed=0, max_instructions=2_000_000, max_frames=10000):
    if len(ram) != 2048 or ram[0x22] & 64 or ram[0x5FB] != 0 or ram[0x441] not in range(1, 11):
        raise ValueError('Expected supported human field-carrier RAM checkpoint')
    if mode not in ('open', 'poll', 'basic-shot') or not 0 <= pressed <= 255:
        raise ValueError('Unsupported observation mode/pressed mask')
    if ram[0x600] > 5 or max_instructions < 1 or max_frames < 1:
        raise ValueError('Unsupported observation bounds/selected count')
    memory = ObservedCartridge(prg, ram, None, set())
    memory.ram[:0x1A] = bytes(0x1A)
    for address, value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),
                           (0x191,0x85),(0x192,0xEB),(0x24,26),(0x25,27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]; memory.registers[6:8] = [26,27]
    entry = (0xE4BE if ram[0x600] else 0xE38A) if mode == 'open' else 0xD5EA if mode == 'poll' else 0x8F72
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = entry, 0x50, cpu.UNUSED
    memory[0x1C], memory[0x1E] = pressed, pressed
    if mode == 'basic-shot':
        # Explicit prepared ordinary UI choice; this is not a special-move/menu emulation.
        memory[0x43B], memory[0x43C], memory[0x44E] = 0,0,0
        memory[0x151], memory[0x152] = 255,1
    frames, requests = [], []
    for step in range(max_instructions):
        if mode == 'basic-shot' and cpu.pc == 0x200:
            boundary = 'basic_attack_prepared'
        else:
            if cpu.pc < 0x8000: raise RuntimeError(f'Unexpected PC${cpu.pc:04X}')
            bank = memory.bank_at(cpu.pc); boundary = None
            if bank == 30:
                if cpu.pc == 0xD5E5: boundary = 'human_action_menu'
                elif mode == 'poll' and cpu.pc == 0xD67C: boundary = 'attack_choice_pending'
                elif cpu.pc in (0xD586,0xD5AF): requests.append({'kind':1,'id':cpu.a})
                elif cpu.pc == 0xD5B2: requests.append({'kind':4,'id':0})
                elif cpu.pc in (0xD5D4,0xD5DD): requests.append({'kind':0,'id':cpu.a})
                elif cpu.pc == 0xD676: requests.append({'kind':3,'id':cpu.a})
            elif bank == 31 and cpu.pc in (0xE38C,0xE4C3): requests.append({'kind':1,'id':cpu.a})
        if boundary:
            return {'format':'captain-action-menu-observation-v1','mode':mode,'entry':entry,'exit':cpu.pc,
                'boundary':boundary,'original_instructions':step,'internal_ram_sha256':hashlib.sha256(ram).hexdigest(),
                'scope':'Constructed CPU/task observation; no skipped calls, NMI/audio/PPU/hardware timing.',
                'host_setup':'Foreground5/SP50, background1/EB86/SP90; banks26/27 and scheduler slots replaced.',
                'host_policy':f'CAAE: acknowledge frame/display/one graphics descriptor; held/pressed fixed{pressed}.',
                'prepared_choice': 'action0/subtype0/variant0 and synthetic0200 return' if mode=='basic-shot' else None,
                'host_frames':frames,'presentation_requests':requests,'action':memory[0x43B],
                'cursor':memory[0x61E],'flash':memory[0x61F]},bytes(memory.ram[:2048])
        if cpu.pc == 0xCAAE:
            if len(frames) >= max_frames: raise RuntimeError('Host frame acknowledgement bound reached')
            memory.instruction = None
            memory[0x1B] |= 128; memory[0x515] = 0
            memory[0x1C], memory[0x1E] = pressed,pressed
            if memory[0x498]: memory[0x498] -= 1
            frames.append(step)
        memory.instruction = (bank,cpu.pc)
        cpu.step()
    raise RuntimeError(f'Original instruction bound reached at${cpu.pc:04X}')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('rom',type=Path);p.add_argument('ram',type=Path);p.add_argument('report',type=Path);p.add_argument('output_ram',type=Path)
    p.add_argument('--mode',choices=('open','poll','basic-shot'),default='open');p.add_argument('--pressed',type=int,default=0)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        p.error('ROM does not match verified Japanese reference')
    report,final=trace_menu(rom[16:262160],a.ram.read_bytes(),a.mode,a.pressed)
    with a.output_ram.open('xb') as out:out.write(final)
    with a.report.open('x') as out:json.dump(report,out,indent=2)
    print(f"Observed {report['boundary']} at${report['exit']:04X}.")


if __name__=='__main__':main()

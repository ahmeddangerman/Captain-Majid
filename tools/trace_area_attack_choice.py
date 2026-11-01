#!/usr/bin/env python3
"""Observe the original opponent action choice after area defense selection.

Private CPU research only. This does not implement native AI or execute an action.
"""
import argparse,hashlib,json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from verify_action_strength import PAYLOAD_SHA256


def observe_area_attack_choice(prg,ram):
    if len(ram)!=2048 or ram[0x22]&64 or ram[0x5fb]!=11 or ram[0x621]!=2:
        raise ValueError('Expected prepared opponent possession in own-area route2')
    count=ram[0x600]
    if count>4 or any(ram[0x60b+i]>6 for i in range(count)):
        raise ValueError('Expected completed field defensive choices')
    watched={0x43b,0x43c,0x43f,0x440,0x444,0x445,0x61c,0x61d}
    memory=ObservedCartridge(prg,ram,None,watched)
    memory.registers[6:8]=[26,27];memory[0x24],memory[0x25]=26,27
    memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x8b8f,0x50,cpu.UNUSED
    for step in range(20000):
        if memory.bank_at(cpu.pc)==26 and cpu.pc==0x897d:
            return {'instructions':step,'action':memory[0x43b],'subtype':memory[0x43c],
                    'writes':memory.writes},bytes(memory.ram[:2048])
        if cpu.pc<0x8000 or cpu.pc in (0xcb0f,0xcaae):
            raise RuntimeError(f'AI choice reached unsupported scheduler/return at {cpu.pc:04x}')
        memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
    raise RuntimeError(f'AI choice instruction bound at {cpu.pc:04x}')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last'])
    base[0x60b:0x60f]=bytes([4]*4);base[0x606:0x60a]=bytes(4);base[0x61e]=4
    cases=[]
    for second in (0,127,255):
        for first in range(256):
            raw=bytearray(base);raw[0xe2],raw[0xe3]=first,second
            report,final=observe_area_attack_choice(prg,raw)
            (out/f'r{first}-s{second}-choice.bin').write_bytes(final)
            cases.append({'random_first':first,'random_second':second,**report})
    summary={str(action):sum(c['action']==action for c in cases)for action in sorted({c['action']for c in cases})}
    (out/'observations.json').write_text(json.dumps({'cases':cases,'action_counts':summary,
        'scope':'768 controlled random pairs on one captured area fixture with constructed completed ordinary defenses. Original CPU executes8B8F through897D after opponent choice; no scheduler/presentation calls skipped. No native AI comparison or action execution. Other profiles, formations and random scheduling remain unverified.'},indent=2)+'\n')
    print(f'{len(cases)} original area attack choices observed: {summary}; private outputs: {out}')


if __name__=='__main__':main()

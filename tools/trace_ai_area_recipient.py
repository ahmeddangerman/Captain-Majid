#!/usr/bin/env python3
"""Bounded original recipient choice; stop before its special-move lookup."""
import argparse,hashlib,json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from verify_action_strength import PAYLOAD_SHA256

def observe_recipient(prg,ram,profile_type):
 if len(ram)!=2048 or not 35<=profile_type<=203 or not 12<=ram[0x441]<=21:
  raise ValueError('Expected direct opponent profile and field carrier')
 m=ObservedCartridge(prg,ram,None,set());m.registers[6:8]=[28,29]
 m[0x24],m[0x25]=28,29;m[0x23]=m[0x22]|7;m.select=m[0x23]
 pointer=0x9662+12*(profile_type-35);m[0x3a],m[0x3b]=pointer&255,pointer>>8
 m[0x1fe],m[0x1ff]=255,1;c=MPU(memory=m);c.pc,c.sp,c.p=0x8933,0xfd,c.UNUSED
 for step in range(4000):
  if c.pc==0x8a3f or c.pc==0x200:
   return {'boundary':'move_lookup' if c.pc==0x8a3f else 'fallback','recipient':m[0x5fc] if c.pc==0x8a3f else None,'action':m[0x43b],
           'cell':m[0x638] if c.pc==0x8a3f else None,'random_first':m[0xe2],'instructions':step}
  if c.pc<0x8000 or c.pc in (0xcb0f,0xcaae):raise RuntimeError(f'Unsupported recipient boundary {c.pc:04x}')
  c.step()
 raise RuntimeError('Recipient instruction bound')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','keeper_ram','output'):p.add_argument(name,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
 out=a.output.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private report must stay outside checkout')
 prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last']);cases=[]
 for kind in range(35,204):
  for pattern in ('left','right','spread'):
   for first in range(8):
    for second in (0,127,255):
     raw=bytearray(base);raw[0x441],raw[0x621],raw[0x635],raw[0xe2],raw[0xe3]=12,2,120,first,second
     for player in range(12,22):raw[0x306+12*player]=80 if pattern=='left' else 160 if pattern=='right' else 60+10*(player-12)
     report=observe_recipient(prg,raw,kind);cases.append({'type':kind,'pattern':pattern,'first':first,'second':second,**report})
 counts={b:sum(c['boundary']==b for c in cases)for b in ('move_lookup','fallback')}
 with out.open('x')as f:json.dump({'cases':cases,'counts':counts,'scope':'Constructed opponent positions on captured RAM. Original8933 through recipient selection, stopping before8A3F special-move lookup or return. No frame/scheduler calls skipped; no native parity or action execution.'},f,indent=2)
 print(f'{len(cases)} original recipient decisions observed: {counts}; private report {out}')
if __name__=='__main__':main()

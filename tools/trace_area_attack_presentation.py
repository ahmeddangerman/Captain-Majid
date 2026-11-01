#!/usr/bin/env python3
"""Observe area action presentation to defender setup with host acknowledgements."""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge

def trace_area_attack_presentation(prg,ram):
 if len(ram)!=2048 or ram[0x22]&64 or ram[0x5fb]!=11 or ram[0x621]!=2 or not 1<=ram[0x600]<=4:
  raise ValueError('Expected prepared opponent area attack and nonempty defenders')
 m=ObservedCartridge(prg,ram,None,{0x532});m.ram[:0x1a]=bytes(0x1a)
 for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):m[address]=value
 m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27]
 c=MPU(memory=m);c.pc,c.sp,c.p=0x8980,0x50,c.UNUSED;frames=0;requests=[]
 for step in range(500000):
  bank=m.bank_at(c.pc)
  if bank==26 and c.pc==0x8998:
   return {'instructions':step,'frames':frames,'requests':requests,'motion_writes':m.writes},bytes(m.ram[:2048])
  if bank==26 and c.pc==0x8982:requests.append({'kind':1,'id':c.a,'slot':0})
  if c.pc<0x8000:raise RuntimeError('Area action presentation left PRG')
  if c.pc==0xcaae:
   m[0x1b]|=128;m[0x515]=0
   if m[0x498]:m[0x498]-=1
   frames+=1
  m.instruction=(bank,c.pc);c.step()
 raise RuntimeError('Area action presentation instruction bound')

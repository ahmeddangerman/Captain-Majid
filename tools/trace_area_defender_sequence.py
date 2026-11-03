#!/usr/bin/env python3
"""Observe original area result presentation and defender-loop boundaries.

Only explicit host frame acknowledgements are supplied. No original routines are
skipped and random bytes stay frozen. RAM observations belong outside Git.
"""
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge

def trace_area_sequence(prg, raw, *, until='presentation', cursor=0):
 if len(raw)!=2048 or raw[0x22]&64 or raw[0x5fb]!=11 or not 1<=raw[0x600]<=4 or not 0<=cursor<raw[0x600]:
  raise ValueError('Expected supported original opponent area defender input')
 if until not in ('clock','presentation','next'):raise ValueError('Unsupported area sequence endpoint')
 m=ObservedCartridge(prg,raw,None,set());m.ram[:0x1a]=bytes(0x1a)
 for address,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):
  m[address]=value
 m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27];m[0x616]=cursor
 c=MPU(memory=m);c.pc,c.sp,c.p=0x89da,0x50,c.UNUSED;frames=0;requests=[];clock=None;comparison=None
 for step in range(500000):
  bank=m.bank_at(c.pc);boundary=None
  if bank==26:
   if c.pc==0x8a28:comparison={'power':m[0x32]|m[0x33]<<8,'cost':m[0x43f]|m[0x440]<<8,'effect':m[0x445],'strength':m[0x71],'loss':m[0x619],'score':c.a,'result':c.y,'subtype':m[0x43e],'weakened':int(bool(m[0x3a]&128))}
   if c.pc==0x8a3f:
    clock=bytes(m.ram[:2048])
    if until=='clock':boundary='clock_done'
   if c.pc==0x8a48 and until=='presentation':boundary='presented'
   if c.pc==0x89da and m[0x616]!=cursor:boundary='next_defender'
   if c.pc==0x8a5d:boundary='prefix_done'
   if c.pc==0x8a60:boundary='terminal_result'
   if c.pc in (0x8a40,0x8a45):requests.append({'kind':1,'id':c.a,'slot':0})
   if c.pc==0x89d2:requests.append({'kind':5,'id':c.a,'slot':0})
  if bank==30 and c.pc in (0xd214,0xda98):boundary='added_time' if c.pc==0xd214 else 'period_end'
  if boundary:return {'boundary':boundary,'instructions':step,'frames':frames,'requests':requests,'comparison':comparison},bytes(m.ram[:2048]),clock
  if c.pc<0x8000:raise RuntimeError('Original area sequence left PRG')
  if c.pc==0xcaae:
   m[0x1b]|=128;m[0x515]=0
   if m[0x498]:m[0x498]-=1
   frames+=1
   if frames>2000:raise RuntimeError('Original area sequence frame bound')
  c.step()
 raise RuntimeError('Original area sequence instruction bound')

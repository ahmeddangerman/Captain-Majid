#!/usr/bin/env python3
"""Compare the saved area-prefix clock continuation with original CPU observations."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from check_area_defender_sequence_original import saved
from trace_area_attack_choice import observe_area_attack_choice
from trace_area_attack_presentation import trace_area_attack_presentation
from trace_area_defender_sequence import trace_area_sequence
from verify_action_strength import PAYLOAD_SHA256

def observe(prg,raw):
 m=ObservedCartridge(prg,raw,None,set());m.ram[:0x1a]=bytes(0x1a)
 for addr,value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):m[addr]=value
 m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27]
 c=MPU(memory=m);c.pc,c.sp,c.p=0x8a5d,0x50,c.UNUSED;frames=0
 for _ in range(200000):
  bank=m.bank_at(c.pc)
  if bank==26 and c.pc==0x8a60:return bytes(m.ram[:2048]),0
  if bank==30 and c.pc in (0xd214,0xda98):return bytes(m.ram[:2048]),1 if c.pc==0xd214 else 2
  if c.pc<0x8000:raise RuntimeError('Original clock left PRG')
  if c.pc==0xcaae:
   m[0x1b]|=128;m[0x515]=0
   if m[0x498]:m[0x498]-=1
   frames+=1
   if frames>60:raise RuntimeError('Original clock frame bound')
  c.step()
 raise RuntimeError('Original clock instruction bound')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','keeper_ram','probe','output_dir'):p.add_argument(name,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private data must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
 seed=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active']);seed[0x61e]=1;seed[0x60b:0x60f]=bytes((6,255,255,255));seed[0x606:0x60a]=bytes(4)
 _,prepared=observe_area_attack_choice(prg,seed,prepare_attack=True);_,announced=trace_area_attack_presentation(prg,prepared,announce=True)
 _,base,_=trace_area_sequence(prg,announced,until='next');cost=prepared[0x43f:0x441]
 reference=ObservedCartridge(prg,base,None,set());reference.registers[6:8]=[26,27];results=[];boundaries=set()
 for action in (1,4,6):
  elapsed=reference[0x908e+action]
  for remaining in (0,1,3,30,449):
   for extra in (0,5,100):
    for random in (0,255):
     raw=bytearray(base);raw[0x43b]=action;raw[0x43c]=0;raw[0x5f7:0x5f9]=remaining.to_bytes(2,'little');raw[0x5f9]=extra;raw[0xe2]=random;raw[0x5ff]=255
     raw[0x5fc]=13;raw[0x638]=(((raw[0x306+12*13]-48)>>3)*12+((raw[0x308+12*13]-80)>>3))
     raw[0x449],raw[0x44a]=128,2
     for keeper in (0,11):
      off=0x300+12*keeper;raw[off+5:off+8]=bytes((2,7,3));raw[off+10]=5
     label=f'a{action}-t{remaining}-e{extra}-r{random}';source=out/f'{label}-entry.cmjsession';target=out/f'{label}-preview.cmjsession';source.write_bytes(saved(raw,88,cost))
     run=subprocess.run([str(a.probe.resolve()),str(source),str(elapsed),str(target)],capture_output=True,text=True,check=True,timeout=10)
     original,boundary=observe(prg,raw);wanted=saved(original,90+boundary,cost);actual=target.read_bytes()
     if int(run.stdout)!=boundary or actual!=wanted:raise RuntimeError(f'{label}: boundary {run.stdout.strip()} != {boundary}, offsets {[(i,x,y)for i,(x,y)in enumerate(zip(actual,wanted))if x!=y]}')
     results.append({'action':action,'remaining':remaining,'extra':extra,'random':random,'boundary':boundary});boundaries.add(boundary)
 if boundaries!={0,1,2}:raise RuntimeError(f'Missing clock boundary: {boundaries}')
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Original8A5D through8A60 or added-time/period-end; frozen randomness and explicit host frame acknowledgements. Saved clock continuation only; no possession dispatch.'},indent=2)+'\n')
 print(f'{len(results)} saved area prefix clock continuations match original, including normal, added-time and period-end boundaries')
if __name__=='__main__':main()

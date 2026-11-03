#!/usr/bin/env python3
"""Compare area result dispatch movement to original through8A72."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from check_area_defender_sequence_original import saved
from check_area_prefix_clock_original import observe as observe_prefix
from trace_area_attack_choice import observe_area_attack_choice
from trace_area_attack_presentation import trace_area_attack_presentation
from trace_area_defender_sequence import trace_area_sequence
from verify_action_strength import PAYLOAD_SHA256

def observe(prg,raw):
 m=ObservedCartridge(prg,raw,None,set());m.ram[:0x1a]=bytes(0x1a)
 for a,v in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),(6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),(0x24,26),(0x25,27)):m[a]=v
 m[0x23]=m[0x22]|7;m.select=m[0x23];m.registers[6:8]=[26,27]
 c=MPU(memory=m);c.pc,c.sp,c.p=0x8a6f,0x50,c.UNUSED;frames=0
 for _ in range(2000000):
  if m.bank_at(c.pc)==26 and c.pc==0x8a72:return bytes(m.ram[:2048])
  if c.pc<0x8000:raise RuntimeError('Original movement left PRG')
  if c.pc==0xcaae:
   m[0x1b]|=128;m[0x515]=0
   if m[0x498]:m[0x498]-=1
   frames+=1
   if frames>2000:raise RuntimeError('Original movement frame bound')
  c.step()
 raise RuntimeError(f'Original movement instruction bound at bank{m.bank_at(c.pc)}:{c.pc:04x}, frames{frames}')

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','keeper_ram','pack','probe','output_dir'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private data must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];seed=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active'])
 seed[0x61e]=1;seed[0x60b:0x60f]=bytes((6,255,255,255));seed[0x606:0x60a]=bytes(4)
 _,prepared=observe_area_attack_choice(prg,seed,prepare_attack=True);_,entry=trace_area_attack_presentation(prg,prepared,announce=True)
 _,prefix,_=trace_area_sequence(prg,entry,until='next');cost=prepared[0x43f:0x441];prefix=bytearray(prefix);prefix[0x5f7:0x5f9]=(449).to_bytes(2,'little');prefix[0x5f9]=0
 clocked,boundary=observe_prefix(prg,prefix)
 if boundary:raise RuntimeError('Unexpected initial clock boundary')
 results=[]
 for elapsed in (0,1,4,15,255):
  for first,second in ((0,0),(0,255),(255,0),(255,255)):
   raw=bytearray(clocked);raw[0x5ff]=elapsed;raw[0xe2],raw[0xe3]=first,second
   name=f'e{elapsed}-r{first}-{second}';source=out/f'{name}.cmjsession';target=out/f'{name}.cmjfield';source.write_bytes(saved(raw,90,cost))
   run=subprocess.run([str(a.probe.resolve()),str(a.pack.resolve()),str(source),str(target)],capture_output=True,text=True,timeout=30)
   if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
   final=observe(prg,raw);expected=saved(final,90,cost)[11:250];actual=target.read_bytes()
   if run.stdout.strip()!='0' or actual!=expected:raise RuntimeError(f'{name}: field differences {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y][:25]}')
   results.append({'elapsed':elapsed,'first':first,'second':second})
 for power in (0,8):
  raw=bytearray(entry);actor=raw[0x601];raw[0x300+12*actor:0x304+12*actor]=bytes((23,100,0,15))
  raw[0x60b],raw[0x606],raw[0x43b],raw[0x43c]=2,0,4,0
  raw[0x61c:0x61e]=power.to_bytes(2,'little');raw[0xe2:0xe4]=bytes((255,255));raw[0x5f7:0x5f9]=(449).to_bytes(2,'little');raw[0x5f9]=0
  report,terminal,_=trace_area_sequence(prg,raw,until='next')
  if report['boundary']!='terminal_result' or report['comparison']['result'] not in (2,3):raise RuntimeError('Expected terminal contact')
  comparison=report['comparison'];route=comparison['result']
  for elapsed in (0,1,4,15,255):
   for first,second in ((0,0),(0,255),(255,0),(255,255)):
    raw=bytearray(terminal);raw[0x5ff]=elapsed;raw[0xe2],raw[0xe3]=first,second
    name=f'c{route}-e{elapsed}-r{first}-{second}';source=out/f'{name}.cmjsession';target=out/f'{name}.cmjfield';source.write_bytes(saved(raw,89,cost,comparison))
    run=subprocess.run([str(a.probe.resolve()),str(a.pack.resolve()),str(source),str(target)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
    final=observe(prg,raw);expected=saved(final,89,cost,comparison)[11:250];actual=target.read_bytes()
    if run.stdout.strip()!=str(route) or actual!=expected:raise RuntimeError(f'{name}: field differences {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y][:25]}')
    results.append({'route':route,'elapsed':elapsed,'first':first,'second':second})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Constructed nonterminal and terminal-ready states; original8A6F to8A72 including C606 field advancement. Frozen random bytes at input and explicit host frame acknowledgements. No later dispatch or possession transition.'},indent=2)+'\n')
 print(f'{len(results)} area dispatch movement previews match original across nonterminal and terminal entries')
if __name__=='__main__':main()

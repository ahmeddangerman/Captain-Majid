#!/usr/bin/env python3
"""Compare final area dispatch and selected recovery against original CPU traces."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_area_defense_poll_original import defense_entries
from check_area_defender_sequence_original import saved
from check_area_prefix_clock_original import observe as observe_prefix
from trace_ball_recovery_phase import trace_recovery
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
 for player_id in (*range(1,11),*range(12,22)):
  seed[0x300+12*player_id:0x304+12*player_id]=bytes((23,100,0,15))
 _,prepared=observe_area_attack_choice(prg,seed,prepare_attack=True);_,entry=trace_area_attack_presentation(prg,prepared,announce=True)
 _,prefix,_=trace_area_sequence(prg,entry,until='next');cost=prepared[0x43f:0x441];prefix=bytearray(prefix);prefix[0x5f7:0x5f9]=(449).to_bytes(2,'little');prefix[0x5f9]=0
 clocked,boundary=observe_prefix(prg,prefix)
 if boundary:raise RuntimeError('Unexpected initial clock boundary')
 results=[]
 for elapsed in (0,1,4,15,255):
  for first,second in ((0,0),(0,255),(255,0),(255,255)):
   raw=bytearray(clocked);raw[0x5ff]=elapsed;raw[0xe2],raw[0xe3]=first,second
   name=f'e{elapsed}-r{first}-{second}';source=out/f'{name}.cmjsession';target=out/f'{name}-dispatched.cmjsession';source.write_bytes(saved(raw,90,cost))
   run=subprocess.run([str(a.probe.resolve()),str(a.pack.resolve()),str(source),'--dispatch-area-result','--capture',str(target)],capture_output=True,text=True,timeout=30)
   if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
   final=observe(prg,raw);expected=bytearray(saved(final,90,cost)[11:250]);actual=target.read_bytes()
   route=re.search(r'route (\d+)',run.stdout)
   if not route or route.group(1)!='0' or actual[258]!=93 or actual[11:250]!=expected:
    raise RuntimeError(f'{name}: wrong route/phase or movement field; route={route.group(1) if route else None}, phase={actual[258] if len(actual)>258 else None}, differences {[(i,x,y)for i,(x,y)in enumerate(zip(actual[11:250],expected))if x!=y][:25]}')
   results.append({'elapsed':elapsed,'first':first,'second':second})
 for power in (0,8):
  for defense_action in (2,4):
   raw=bytearray(entry);actor=raw[0x601];raw[0x300+12*actor:0x304+12*actor]=bytes((23,100,0,15))
   raw[0x60b],raw[0x606],raw[0x43b],raw[0x43c]=defense_action,0,4,0
   raw[0x61c:0x61e]=power.to_bytes(2,'little');raw[0xe2:0xe4]=bytes((255,255));raw[0x5f7:0x5f9]=(449).to_bytes(2,'little');raw[0x5f9]=0
   report,terminal,_=trace_area_sequence(prg,raw,until='next')
   if report['boundary']!='terminal_result' or report['comparison']['result'] not in (2,3):raise RuntimeError('Expected terminal contact')
   comparison=report['comparison'];result=comparison['result'];route=3 if result==3 and defense_action==2 else 2
   for elapsed in (0,1,4,15,255):
    for first,second in ((0,0),(0,255),(255,0),(255,255)):
     raw=bytearray(terminal);raw[0x5ff]=elapsed;raw[0xe2],raw[0xe3]=first,second
     name=f'c{result}-d{defense_action}-w{power}-e{elapsed}-r{first}-{second}';source=out/f'{name}.cmjsession';target=out/f'{name}-dispatched.cmjsession';source.write_bytes(saved(raw,89,cost,comparison))
     run=subprocess.run([str(a.probe.resolve()),str(a.pack.resolve()),str(source),'--dispatch-area-result','--capture',str(target)],capture_output=True,text=True,timeout=30)
     if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
     final=observe(prg,raw);expected=bytearray(saved(final,89,cost,comparison)[11:250]);actual=target.read_bytes()
     if route==3:
      expected[9]=0;expected[10]=raw[0x601+raw[0x616]]
     expected_phase=1 if route==3 else 94
     actual_route=re.search(r'route (\d+)',run.stdout)
     if not actual_route or actual_route.group(1)!=str(route) or actual[258]!=expected_phase or actual[11:250]!=expected:
      raise RuntimeError(f'{name}: wrong route/phase or movement/possession state; route={actual_route.group(1) if actual_route else None}, phase={actual[258] if len(actual)>258 else None}, differences {[(i,x,y)for i,(x,y)in enumerate(zip(actual[11:250],expected))if x!=y][:25]}')
     recovery_boundary=None
     if route==2:
      recovered=out/f'{name}-recovered.cmjsession'
      recovery_run=subprocess.run([str(a.probe.resolve()),str(a.pack.resolve()),str(target),'--recover-area-result','--capture',str(recovered)],capture_output=True,text=True,timeout=30)
      if recovery_run.returncode:raise RuntimeError(f'{name} recovery: {recovery_run.stderr}')
      recovery_report,recovered_raw=trace_recovery(prg,final,selected_recovery=True)
      recovery_boundary=recovery_report['boundary']
      expected_recovery=bytearray(saved(recovered_raw,89,cost,comparison)[11:250]);actual_recovery=recovered.read_bytes()
      if recovery_boundary=='recovered_owner_handoff':
       if actual_recovery[258]!=95 or actual_recovery[11:250]!=expected_recovery:
        raise RuntimeError(f'{name}: recovered-owner field differs from original8C31: {[(i,x,y)for i,(x,y)in enumerate(zip(actual_recovery[11:250],expected_recovery))if x!=y][:25]}')
      elif recovery_boundary=='boundary_restart':
       if actual_recovery[258]!=5 or actual_recovery[20:25]!=expected_recovery[9:14]:
        raise RuntimeError(f'{name}: area boundary-restart ball handoff differs from original')
      else:raise RuntimeError(f'{name}: unsupported original recovery boundary {recovery_boundary}')
     results.append({'result':result,'defense_action':defense_action,'route':route,'recovery_boundary':recovery_boundary,'elapsed':elapsed,'first':first,'second':second})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Original8A6F through C606 movement and saved result dispatch; terminal action2 direct capture versus action4 recovery route. Route2 continuations also match selected recovery at original8C31 or boundary restart. Frozen input randomness and explicit host frame acknowledgements.'},indent=2)+'\n')
 print(f'{len(results)} final area dispatch states match original; {sum(r.get("route")==2 for r in results)} selected recovery continuations also match')
if __name__=='__main__':main()

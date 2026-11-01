#!/usr/bin/env python3
"""Compare saved first area defender contact to original prepared CPU execution."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from check_area_defender_announcement_original import checkpoint
from trace_area_attack_choice import observe_area_attack_choice
from trace_area_attack_presentation import trace_area_attack_presentation
from check_area_contact_original import observe_contact
from verify_action_strength import PAYLOAD_SHA256

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram','output_dir'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()));results=[];codes=set()
 binary='build/captain-phase-session';pack=str(a.pack.resolve())
 for count in (1,4):
  for variant in (1,2):
   seed=bytearray(seeds['active' if count==1 else 'last']);seed[0x61e]=count;seed[0x44e]=variant
   seed[0x60b:0x60f]=bytes([4]*count+[255]*(4-count));seed[0x606:0x60a]=bytes(4)
   _,prepared=observe_area_attack_choice(prg,seed,prepare_attack=True)
   _,announced=trace_area_attack_presentation(prg,prepared,announce=True)
   attack_cost=prepared[0x43f:0x441]
   for defense,subtype in ((2,0),(2,1),(4,0)):
    for kind in (23,32):
     for stamina in (0,100,65535):
      for attack in (1,4,6):
       for power in (0,16,65535):
        for first,second in ((0,0),(255,255)):
         raw=bytearray(announced);actor=raw[0x601];off=0x300+12*actor
         raw[off:off+4]=bytes((kind,stamina&255,stamina>>8,15));raw[0x43b]=attack
         raw[0x60b],raw[0x606],raw[0x616]=defense,subtype,0
         raw[0x61c:0x61e]=power.to_bytes(2,'little');raw[0xe2],raw[0xe3]=first,second
         raw[0x43c]=0
         # Explicit target metadata for constructed action1 prepared inputs.
         raw[0x5fc]=13;raw[0x638]=(((raw[0x306+12*13]-48)>>3)*12+((raw[0x308+12*13]-80)>>3))
         name=f'n{count}-v{variant}-d{defense}s{subtype}-t{kind}-e{stamina}-a{attack}-p{power}-r{first}'
         source=out/f'{name}.cmjsession';data=bytearray(checkpoint(raw,81));data[396:398]=attack_cost;source.write_bytes(data)
         target=out/f'{name}-contact.cmjsession'
         run=subprocess.run([binary,pack,str(source),'--resolve-area-defender','--capture',str(target)],capture_output=True,text=True,timeout=30)
         if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
         m,c=observe_contact(prg,raw);m[0x612]=c.y
         expected=bytearray(checkpoint(bytes(m.ram[:2048]),82));expected[396:398]=attack_cost
         expected[398:412]=bytes((1,0,defense,m[0x43e],m[0x445],m[0x32],m[0x33],m[0x43f],m[0x440],m[0x71],m[0x619],c.a,c.y,int(bool(m[0x3a]&128))))
         actual=target.read_bytes()
         if actual!=expected:raise RuntimeError(f'{name}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]} lengths{len(actual)}/{len(expected)}')
         if len(results)<4:
          replay=out/f'{name}-replay.cmjsession'
          repeat=subprocess.run([binary,pack,str(target),'--resolve-area-defender','--capture',str(replay)],capture_output=True,text=True,timeout=30)
          if not repeat.returncode or replay.exists():raise RuntimeError('Replayed contact charged stamina twice')
          interactive=out/f'{name}-interactive.cmjsession'
          repeat=subprocess.run([binary,pack,str(source),'--interactive'],input=f'resolve-area-defender\nsave {interactive}\nquit\n',capture_output=True,text=True,timeout=30)
          if repeat.returncode or 'Error:' in repeat.stdout or interactive.read_bytes()!=actual:raise RuntimeError('Interactive area contact differs')
         codes.add(c.y);results.append({'count':count,'variant':variant,'defense':defense,'subtype':subtype,'type':kind,'stamina':stamina,'attack':attack,'power':power,'random':first,'result':c.y})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Constructed prepared announcement inputs. Original89DA..8A28 executes all instructions; result write8148 represented by captured Y. Full native saved state, energy and comparison checked. Only first defender; result timing/dispatch not run.'},indent=2)+'\n')
 if codes!=set(range(4)):raise RuntimeError(f'Incomplete result coverage {codes}')
 print(f'{len(results)} complete saved area defender contacts match original; four results, replay and interactive capture checked')
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare complete supported area defender prefixes and prepared result displays."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from check_area_defender_announcement_original import checkpoint
from check_area_contact_original import observe_contact
from trace_area_attack_choice import observe_area_attack_choice
from trace_area_attack_presentation import trace_area_attack_presentation
from trace_area_defender_sequence import trace_area_sequence
from verify_action_strength import PAYLOAD_SHA256


def saved(raw,phase,cost,comparison=None,cursor=None):
 data=bytearray(checkpoint(raw,phase));data[396:398]=cost
 index=raw[0x616] if cursor is None else cursor
 data[412]=index
 if comparison is not None:
  c=comparison
  data[398:412]=bytes((1,index,raw[0x60b+index],c['subtype'],c['effect'],c['power']&255,c['power']>>8,c['cost']&255,c['cost']>>8,c['strength'],c['loss'],c['score'],c['result'],c['weakened']))
 return bytes(data)


def run_step(binary,pack,source,target,operation):
 run=subprocess.run([binary,pack,str(source),operation,'--capture',str(target)],capture_output=True,text=True,timeout=30)
 if run.returncode:raise RuntimeError(f'{source.name} {operation}: {run.stderr}')
 requests=[{'kind':int(k),'id':int(v),'slot':int(slot)}for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
 return target.read_bytes(),requests


def matches(actual,expected,label):
 if actual!=expected:
  diffs=[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]
  raise RuntimeError(f'{label}: {diffs[:20]}, lengths{len(actual)}/{len(expected)}')


def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese reference')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False)
 prg=rom[16:262160];seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()))
 binary='build/captain-phase-session';pack=str(a.pack.resolve());results=[];codes=set();slots=set();contacts=skips=0
 patterns=((4,4,4,4),(2,4,2,4),(5,6,5,6),(6,4,5,2),(2,6,4,5),(5,5,4,6))
 for count in (1,4):
  for variant in (1,2):
   seed=bytearray(seeds['active' if count==1 else 'last']);seed[0x600],seed[0x61e],seed[0x44e]=count,count,variant
   ids=[2,5,7,10][:count];seed[0x601:0x606]=bytes(ids+[0]*(5-count))
   seed[0x60b:0x60f]=bytes([4]*count+[255]*(4-count));seed[0x606:0x60a]=bytes(4)
   _,prepared=observe_area_attack_choice(prg,seed,prepare_attack=True)
   _,base=trace_area_attack_presentation(prg,prepared,announce=True);cost=prepared[0x43f:0x441]
   for pattern_id,pattern in enumerate(patterns):
    for attack in (1,4,6):
     for power in (0,8,16,40,65535):
      for random in (0,255):
       raw=bytearray(base);raw[0x43b],raw[0x43c],raw[0x612],raw[0x616]=attack,0,0,0
       raw[0x60b:0x60f]=bytes(list(pattern[:count])+[255]*(4-count))
       raw[0x606:0x60a]=bytes([int(action==2 and pattern_id%2==0)for action in pattern[:count]]+[0]*(4-count))
       for i,id in enumerate(ids):
        raw[0x300+12*id:0x304+12*id]=bytes((23 if i%2==0 else 32,0 if pattern_id==4 else 100,0,15))
       raw[0x61c:0x61e]=power.to_bytes(2,'little');raw[0xe2:0xe4]=bytes((random,random))
       raw[0x5f7:0x5f9]=(449).to_bytes(2,'little');raw[0x5f9]=0;raw[0x517]=0 if random==0 else 64
       raw[0x5fc]=13;raw[0x638]=(((raw[0x306+12*13]-48)>>3)*12+((raw[0x308+12*13]-80)>>3))
       name=f'n{count}-v{variant}-p{pattern_id}-a{attack}-w{power}-r{random}'
       source=out/f'{name}-entry.cmjsession';source.write_bytes(saved(raw,81,cost,cursor=0));commands=[]
       for cursor in range(count):
        action=raw[0x60b+cursor];slots.add(cursor)
        target=out/f'{name}-s{cursor}-next.cmjsession'
        if action in (5,6):
         report,after,_=trace_area_sequence(prg,raw,until='next',cursor=cursor)
         phase=87 if report['boundary']=='next_defender' else 88
         if report['boundary']not in ('next_defender','prefix_done'):raise RuntimeError('Unexpected skip boundary')
         actual,requests=run_step(binary,pack,source,target,'--skip-area-defender');commands.append('skip-area-defender')
         matches(actual,saved(after,phase,cost),name+' skip')
         if requests!=report['requests']:raise RuntimeError('Skip wait requests differ')
         skips+=1
        else:
         contact_path=out/f'{name}-s{cursor}-contact.cmjsession'
         actual,_=run_step(binary,pack,source,contact_path,'--resolve-area-defender');commands.append('resolve-area-defender')
         m,c=observe_contact(prg,raw);m[0x612]=c.y
         comparison={'power':m[0x32]|m[0x33]<<8,'cost':m[0x43f]|m[0x440]<<8,'effect':m[0x445],'strength':m[0x71],'loss':m[0x619],'score':c.a,'result':c.y,'subtype':m[0x43e],'weakened':int(bool(m[0x3a]&128))}
         matches(actual,saved(bytes(m.ram[:2048]),82,cost,comparison),name+' contact');codes.add(c.y)
         clock_path=out/f'{name}-s{cursor}-clock.cmjsession'
         clock_report,clock_raw,_=trace_area_sequence(prg,raw,until='clock',cursor=cursor)
         if clock_report['boundary']!='clock_done':raise RuntimeError('Unexpected clock pause in normal-time sequence')
         actual,_=run_step(binary,pack,contact_path,clock_path,'--advance-area-contact-clock');commands.append('advance-area-contact-clock')
         matches(actual,saved(clock_raw,83,cost,comparison),name+' clock')
         presented=out/f'{name}-s{cursor}-presented.cmjsession'
         report,presented_raw,_=trace_area_sequence(prg,raw,cursor=cursor)
         if report['boundary']!='presented':raise RuntimeError('Missing result presentation')
         actual,requests=run_step(binary,pack,clock_path,presented,'--present-area-contact-result');commands.append('present-area-contact-result')
         matches(actual,saved(presented_raw,86,cost,comparison),name+' presented')
         if requests!=report['requests']:raise RuntimeError('Result script requests differ')
         report,after,_=trace_area_sequence(prg,raw,until='next',cursor=cursor)
         phase={'next_defender':87,'prefix_done':88,'terminal_result':89}[report['boundary']]
         actual,requests=run_step(binary,pack,presented,target,'--advance-area-defender');commands.append('advance-area-defender')
         matches(actual,saved(after,phase,cost,comparison if phase==89 else None),name+' advance')
         if requests!=report['requests'][2:]:raise RuntimeError('Next defender wait differs')
         contacts+=1
        if phase in (88,89):
         # A completed prefix or terminal result cannot charge another defender.
         for operation in ('--resolve-area-defender','--skip-area-defender','--advance-area-defender','--present-area-contact-result'):
          failed=out/f'{name}-rejected.cmjsession'
          run=subprocess.run([binary,pack,str(target),operation,'--capture',str(failed)],capture_output=True,text=True,timeout=30)
          if not run.returncode or failed.exists():raise RuntimeError('Completed sequence replay accepted')
         break
        source=target;raw=bytearray(after)
       else:raise RuntimeError('Prefix did not terminate')
       if len(results)<4:
        interactive=out/f'{name}-interactive.cmjsession'
        run=subprocess.run([binary,pack,str(out/f'{name}-entry.cmjsession'),'--interactive'],input='\n'.join([*commands,f'save {interactive}','quit','']),capture_output=True,text=True,timeout=30)
        if run.returncode or 'Error:' in run.stdout:raise RuntimeError('Interactive chain failed')
        matches(interactive.read_bytes(),actual,'interactive chain')
       results.append({'count':count,'variant':variant,'pattern':pattern_id,'attack':attack,'power':power,'random':random,'exit':phase,'last_cursor':after[0x616]})
 if codes!=set(range(4))or slots!=set(range(4)):raise RuntimeError(f'Incomplete result/slot coverage {codes}/{slots}')
 (out/'verification.json').write_text(json.dumps({'cases':results,'contacts':contacts,'skips':skips,'scope':'Full represented native prefix, contact, clock and prepared result/loop states compared to actual original instructions. Explicit frozen-random frame acknowledgements. Stops at89DA next defender,8A5D prefix-end before final clock, or8A60 terminal before possession dispatch. No native renderer or complete match.'},indent=2)+'\n')
 print(f'{len(results)} complete area defender sequences match original; {contacts} contacts, {skips} skipped entries, all four results and selected slots covered')
if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare prepared opponent defender announcements before contact calculation."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries,poll_snapshot
from trace_area_attack_choice import observe_area_attack_choice
from trace_area_attack_presentation import trace_area_attack_presentation
from verify_action_strength import PAYLOAD_SHA256
def checkpoint(raw,phase):
 data=bytearray(poll_snapshot(raw,phase));action=raw[0x43b]
 data[393:396]=bytes((1,raw[0x5fc] if action==1 else 0,raw[0x638] if action==1 else 0));data[396:398]=raw[0x43f:0x441]
 return bytes(data)
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for n in ('rom','pack','keeper_ram','output_dir'):p.add_argument(n,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()));results=[]
 for count in (1,4):
  for actions in ([4]*4,[6]*4,[2,4,6,2],[5,2,6,4]):
   for variant in (1,2):
    for random in (0,128):
     raw=bytearray(seeds['active' if count==1 else 'last']);ids=[2,5,7,10][:count]
     raw[0x601:0x606]=bytes(ids+[0]*(5-count));raw[0x60b:0x60f]=bytes(actions[:count]+[255]*(4-count));raw[0x606:0x60a]=bytes([int(x==2)for x in actions[:count]]+[0]*(4-count));raw[0x61e]=count;raw[0x44e],raw[0xe2]=variant,random
     _,prepared=observe_area_attack_choice(prg,raw,prepare_attack=True);_,entry=trace_area_attack_presentation(prg,prepared);report,after=trace_area_attack_presentation(prg,prepared,announce=True)
     name=f'n{count}-a{actions[0]}-{actions[-1]}-v{variant}-r{random}';source=out/f'{name}-entry.cmjsession';source.write_bytes(checkpoint(entry,80));target=out/f'{name}-announced.cmjsession'
     run=subprocess.run(['build/captain-phase-session',str(a.pack.resolve()),str(source),'--announce-prepared-area-defenders',str(after[0x517]),'--capture',str(target)],capture_output=True,text=True,timeout=30)
     if run.returncode:raise RuntimeError(run.stderr)
     actual,expected=target.read_bytes(),checkpoint(after,81)
     if actual!=expected:raise RuntimeError(f'{name}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
     requests=[{'kind':int(k),'id':int(v),'slot':int(s)}for k,v,s in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
     if requests!=report['requests'][1:]:raise RuntimeError(f'{name}: request mismatch {requests} != {report["requests"][1:]}')
     results.append({'count':count,'actions':actions[:count],'variant':variant,'random':random,'facing':after[0x517],'frames':report['frames']})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Prepared original script14 followed by defender announcements to89D5. Full represented state and post-intro script/wait requests compared. Final facing supplied explicitly from original; not independently computed. Host frame acknowledgements, no native renderer, clock scheduling or defender outcomes.'},indent=2)+'\n')
 print(f'{len(results)} prepared defender announcements match original; facing captured explicitly')
if __name__=='__main__':main()

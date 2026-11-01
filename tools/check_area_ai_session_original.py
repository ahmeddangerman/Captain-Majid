#!/usr/bin/env python3
"""Compare saved opponent area choices with the original completed-menu caller."""
import argparse,hashlib,json,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries,poll_snapshot
from trace_area_attack_choice import observe_area_attack_choice
from verify_action_strength import PAYLOAD_SHA256
def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
 p.add_argument('--prepare',action='store_true')
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];base=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['last'])
 base[0x61e]=4;base[0x60b:0x60f]=bytes([4]*4);base[0x606:0x60a]=bytes(4);results=[]
 for kind in (35,50,70,117):
  for variant in (1,2):
   for pattern in range(3):
    for first in (*range(8),128,255):
     for second in (0,255):
      raw=bytearray(base);actor=raw[0x441];raw[0x300+12*actor:0x303+12*actor]=bytes((kind,0,0));raw[0x44e],raw[0xe2],raw[0xe3]=variant,first,second
      for i in range(10):raw[0x306+12*(12+i)],raw[0x308+12*(12+i)]=(80 if pattern==0 else 160 if pattern==1 else 60+10*i),80+8*i
      name=f't{kind}-v{variant}-p{pattern}-r{first}-s{second}';source=out/f'{name}.cmjsession';source.write_bytes(poll_snapshot(raw,72))
      report,final=observe_area_attack_choice(prg,raw);target=out/f'{name}-native.cmjsession'
      run=subprocess.run(['build/captain-phase-session',str(a.pack.resolve()),str(source),'--choose-keeper-area-action','--capture',str(target)],capture_output=True,text=True,timeout=30)
      if run.returncode:raise RuntimeError(run.stderr)
      expected=bytearray(poll_snapshot(final,78));expected[393:396]=bytes((1,final[0x5fc] if report['action']==1 else 0,final[0x638] if report['action']==1 else 0))
      actual=target.read_bytes()
      if actual!=expected:raise RuntimeError(f'{name}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]} lengths {len(actual)}/{len(expected)}')
      if a.prepare:
       prepared=out/f'{name}-prepared.cmjsession'
       run=subprocess.run(['build/captain-phase-session',str(a.pack.resolve()),str(target),'--prepare-opponent-area-attack','--capture',str(prepared)],capture_output=True,text=True,timeout=30)
       if run.returncode:raise RuntimeError(run.stderr)
       preparation,raw_prepared=observe_area_attack_choice(prg,raw,prepare_attack=True)
       expected=bytearray(poll_snapshot(raw_prepared,79));expected[393:396]=bytes((1,raw_prepared[0x5fc] if report['action']==1 else 0,raw_prepared[0x638] if report['action']==1 else 0));expected[396:398]=raw_prepared[0x43f:0x441]
       actual=prepared.read_bytes()
       if actual!=expected:raise RuntimeError(f'{name} prepared: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
      results.append({'type':kind,'variant':variant,'pattern':pattern,'first':first,'second':second,'action':report['action']})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Constructed completed defensive prefixes and direct opponent profiles. Full represented save compared at897D after original CPU choice. Target metadata captures new recipient/cell only. No attack preparation, renderer or complete match.'},indent=2)+'\n')
 print(f'{len(results)} complete saved area AI choices match original')
if __name__=='__main__':main()

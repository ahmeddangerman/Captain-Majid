#!/usr/bin/env python3
"""Controlled original keeper-area setup: routes, variants, scan order and caps."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from check_keeper_entry_original import observe
from check_keeper_saves_original import observe_keeper_save
from check_kickoff_save_handoffs_original import snapshot
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];base=bytearray(a.keeper_ram.read_bytes())
    base[0x612],base[0x616],base[0x43d],base[0x43e]=2,0,0,0
    base,_,_=observe_keeper_save(prg,base)
    _,base=trace_recovery(prg,base)
    report,base=trace_recovery(prg,base,continue_handoff=True)
    if report['boundary']!='keeper_area_interaction':raise RuntimeError('Expected keeper-area source')
    binary=str(Path('build/captain-phase-session').resolve());results=[]
    for side in (0,11):
        for x in (56,204):
            for variant in (0,5):
                for first,second in ((0,0),(255,0),(10,50)):
                    ram=bytearray(base)
                    ram[0x5fb],ram[0x441],ram[0x5fd]=side,side+1,1
                    ram[0x635],ram[0x637],ram[0x5fe]=x,124,((x-48)//8)*12+5
                    ram[0xe2],ram[0xe3],ram[0x44e],ram[0x61a]=first,second,variant,70
                    for slot in (*range(1,11),*range(12,22)):
                        record=0x300+12*slot
                        ram[record+6],ram[record+8],ram[record+10]=x,124,int(slot%3==0)
                    name=f's{side}-x{x}-v{variant}-r{first}-{second}'
                    source=out/f'{name}-entry.cmjsession';source.write_bytes(snapshot(ram,7))
                    final,_,steps=observe(prg,ram,0x8b4a,{0x8b73})
                    target=out/f'{name}-native.cmjsession'
                    run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--prepare-keeper-area','--capture',str(target)],capture_output=True,text=True,timeout=30)
                    if run.returncode:raise RuntimeError(run.stderr)
                    actual,expected=target.read_bytes(),snapshot(final,65)
                    if actual!=expected:raise RuntimeError(f'{name} mismatch {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
                    results.append({'side':side,'x':x,'variant':variant,'random':[first,second],
                                    'route':final[0x621],'selected':list(final[0x601:0x601+final[0x600]]),'instructions':steps})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Constructed controlled keeper-area inputs based on captured recovery state; original8B4A..8B73 executes every instruction before interaction presentation. Both possession sides/areas, preserved/generated variants and defender scan order/caps. No menu or live match claim.'},indent=2)+'\n')
    print(f'{len(results)} controlled keeper-area comparisons match: {out}')

if __name__=='__main__':main()

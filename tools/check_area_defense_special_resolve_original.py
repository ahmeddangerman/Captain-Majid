#!/usr/bin/env python3
"""Compare saved special decisions to original move preparation and affordability."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries
from check_area_defense_special_poll_original import checkpoint
from trace_area_defense_special_open import trace_special_open
from trace_area_defense_special_resolve import trace_special_resolution
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    raw=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active'])
    raw[0x30c],raw[0x300+12*10],raw[0x60b],raw[0x442],raw[0x44e]=23,24,2,1,1
    _,opened=trace_special_open(prg,raw,128)
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());cases=[]
    # The profile implementation deliberately excludes certain alternate encodings.
    for player_type in (23,24,48,70):
        for variant in (1,2):
            for level in (0,15):
                for stamina in (0,181,4096):
                    if player_type>=35 and stamina&128 and stamina>>8<35:continue
                    for cursor,pressed in ((0,128),(1,128),(1,192)):
                        cases.append((player_type,variant,level,stamina,cursor,pressed))
    # Cancel bypasses profile loading, including otherwise unsupported raw types.
    cases.extend((255,variant,0,128,cursor,pressed) for variant in (1,2)
                 for cursor in (0,1) for pressed in (64,65,72,79))
    results=[]
    for index,(player_type,variant,level,stamina,cursor,pressed) in enumerate(cases):
        state=bytearray(opened);state[0x30c]=player_type;state[0x30d]=stamina&255;state[0x30e]=stamina>>8
        state[0x30f],state[0x44e],state[0x1e],state[0x622]=level,variant,pressed,cursor
        state[0x43d],state[0x442],state[0x60b]=2,1,2
        source=out/f'{index}-entry.cmjsession';entry=bytearray(checkpoint(state,75,1,cursor,pressed)[:387]);entry[10]=ord('r');source.write_bytes(entry)
        report,final=trace_special_resolution(prg,state,pressed,cursor,1)
        target=out/f'{index}-resolved.cmjsession'
        run=subprocess.run([binary,pack,str(source),'--resolve-keeper-area-special','--capture',str(target)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'case{index} {cases[index]}: {run.stderr}')
        expected=bytearray(checkpoint(final,75,1,cursor,pressed)[:387]);expected[10]=ord('u');expected[258]=76
        status={'accepted':0,'denied':1,'canceled':2}[report['boundary']]
        expected+=bytes((1,status,report['subtype'],report['cost']&255,report['cost']>>8,report['effect']))
        expected+=bytes(3)
        actual=target.read_bytes()
        if actual!=expected:raise RuntimeError(f'case{index} {cases[index]}: {[(i,x,y)for i,(x,y) in enumerate(zip(actual,expected))if x!=y]} length {len(actual)}/{len(expected)}')
        requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
        if requests!=report['requests']:raise RuntimeError(f'case{index}: {requests} != {report["requests"]}')
        results.append({'type':player_type,'variant':variant,'level':level,'stamina':stamina,'cursor':cursor,'pressed':pressed,**report})
    # The interactive command must resume exactly the same saved decision.
    source=out/'0-entry.cmjsession';target=out/'interactive.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--interactive'],
        input=f'resolve-keeper-area-special\nsave {target}\nquit\n',capture_output=True,text=True,timeout=30)
    if run.returncode or 'Error:' in run.stdout or not target.exists() or target.read_bytes()!=(out/'0-resolved.cmjsession').read_bytes():
        raise RuntimeError('Interactive special resolution differs from flag operation')
    target=out/'conflicting-operations.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--resolve-keeper-area-special','--recover','--capture',str(target)],capture_output=True,text=True,timeout=30)
    if not run.returncode or target.exists() or 'Choose one initial operation' not in run.stderr:
        raise RuntimeError('Conflicting initial operations were not rejected before capture')
    def reject(name,data,operation=('--resolve-keeper-area-special',)):
        source=out/f'{name}-entry.cmjsession';source.write_bytes(data);target=out/f'{name}-output.cmjsession'
        run=subprocess.run([binary,pack,str(source),*operation,'--capture',str(target)],capture_output=True,timeout=30)
        if not run.returncode or target.exists():raise RuntimeError(f'Invalid {name} accepted')
    resolved=bytearray((out/'0-resolved.cmjsession').read_bytes())
    reject('replay',resolved)
    malformed=bytearray(resolved);malformed[388]=3;reject('bad-status',malformed)
    reject('truncated',resolved[:-1])
    legacy=bytearray(resolved[:387]);legacy[10]=ord('r');reject('legacy-phase',legacy)
    raw=bytearray(opened);raw[0x30c]=48;raw[0x30d]=128;raw[0x30e]=0;raw[0x30f]=0
    raw[0x44e],raw[0x1e],raw[0x622],raw[0x43d],raw[0x442],raw[0x60b]=1,128,1,2,1,2
    invalid_profile=bytearray(checkpoint(raw,75,1,1,128)[:387]);invalid_profile[10]=ord('r')
    reject('unsupported-profile',invalid_profile)
    # Use ordinary profile to isolate unsupported move index from profile decoding.
    raw[0x30c]=23;raw[0x30d]=0;raw[0x30e]=0
    unsupported_move=bytearray(checkpoint(raw,75,127,1,128)[:387]);unsupported_move[10]=ord('r')
    reject('unsupported-move',unsupported_move)
    summary={key:sum(case['boundary']==key for case in results) for key in ('accepted','denied','canceled')}
    (out/'verification.json').write_text(json.dumps({'cases':results,'summary':summary,'scope':'Captured route2 state with constructed field profiles, stamina, variants, selection/cancel and saved inputs. Original D3F4 runs to D403 or D40C, including actual warning script when denied and explicit cooperative frame acknowledgements. Compared complete represented state, saved decision, and ordered warning requests. No debit, registration, cleanup or actual rendering/NMI/audio.'},indent=2)+'\n')
    print(f'{len(cases)} special decision comparisons match: {summary}: {out}')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare special decision registration and redraw to the approved original ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries, poll_snapshot
from check_area_defense_special_poll_original import checkpoint
from trace_area_defense_special_resolve import trace_special_resolution
from trace_area_defense_special_redraw import trace_special_redraw
from trace_area_defense_poll import trace_defense_poll
from verify_action_strength import PAYLOAD_SHA256


def snapshot(ram,phase,cursor,pressed,option,decision):
    data=bytearray(checkpoint(ram,phase,option,cursor,pressed)[:387])
    data[10]=ord('w');data[258]=phase
    status={'accepted':0,'denied':1,'canceled':2}[decision['status']]
    cost=decision['cost'];data+=bytes((1,status,decision['subtype'],cost&255,cost>>8,decision['effect']))
    data+=bytes(5)
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes();packed=a.pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    if packed[:8]==b'CMJSHOTI':packed=b'CMJSHOTH'+packed[8:21274]
    if packed[:8]==b'CMJSHOTH':packed=b'CMJSHOTG'+packed[8:20717]
    if packed[:8]==b'CMJSHOTG':packed=b'CMJSHOTF'+packed[8:20224]
    if packed[:8]!=b'CMJSHOTF':p.error('Expected private version15F pack')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()))
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());results=[]
    # Each fixture keeps a captured area state while constructing the represented
    # choice arrays. FF slots represent defenders not chosen yet.
    fixtures=[('single',1,0,[2,255,255,255],[0,0,0,0]),
              ('current_special',1,0,[2,255,255,255],[1,0,0,0]),
              ('early',4,0,[2,255,255,255],[1,0,0,0]),
              ('middle',4,1,[2,2,255,255],[1,0,0,0]),
              ('last',4,3,[2,4,6,2],[1,0,0,1])]
    choices=[('ordinary',0,128,0,1),('special',1,128,200,1),
             ('denied',1,128,0,1),('canceled',1,64,0,1),
             ('both_buttons',1,192,200,1),('option_zero',1,128,0,0)]
    for name,count,slot,actions,subtypes in fixtures:
        for choice,cursor,pressed,stamina,option in choices:
            raw=bytearray(seeds['active' if count==1 else 'last']);actor=slot+1
            raw[0x600],raw[0x61e]=count,slot
            raw[0x601:0x606]=bytes(list(range(1,count+1))+[0]*(5-count))
            raw[0x60b:0x60f]=bytes(actions);raw[0x606:0x60a]=bytes(subtypes)
            record=0x300+12*actor;raw[record],raw[record+1],raw[record+2],raw[record+3]=23,stamina&255,stamina>>8,0
            raw[0x43d],raw[0x442],raw[0x44e],raw[0x1e]=2,actor,1,pressed
            resolved,pre=trace_special_resolution(prg,raw,pressed,cursor,option)
            report,final=trace_special_redraw(prg,raw,pressed,cursor,option)
            decision={'status':resolved['boundary'],'subtype':resolved['subtype'],'cost':resolved['cost'],'effect':resolved['effect']}
            if decision!=report['decision']:raise RuntimeError('Decision observer disagreement')
            source=out/f'{name}-{choice}-entry.cmjsession';source.write_bytes(snapshot(pre,76,cursor,pressed,option,decision))
            target=out/f'{name}-{choice}-redraw.cmjsession'
            run=subprocess.run([binary,pack,str(source),'--redraw-keeper-area-special','--capture',str(target)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'{name}/{choice}: {run.stderr}')
            expected=snapshot(final,77,cursor,pressed,option,decision);actual=target.read_bytes()
            if actual!=expected:raise RuntimeError(f'{name}/{choice}: {[(i,x,y)for i,(x,y) in enumerate(zip(actual,expected))if x!=y]} length {len(actual)}/{len(expected)}')
            requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
            expected_requests=[request for request in report['requests'] if request['kind']!=1]
            if requests!=expected_requests:raise RuntimeError(f'{name}/{choice}: {requests} != {expected_requests}')
            results.append({'fixture':name,'choice':choice,'decision':decision['status'],'requests':len(requests),'cursor':final[0x61e]})
    # Older content can redraw subtype0 only; special icons need F. Do not save on failure.
    old=bytearray(packed[:-1]);old[7]=ord('E');old_pack=out/'prior-e.cmjpack';old_pack.write_bytes(old)
    for source in (out/'single-special-entry.cmjsession',out/'middle-canceled-entry.cmjsession'):
        target=out/f'{source.stem}-missing-icon.cmjsession'
        run=subprocess.run([binary,str(old_pack),str(source),'--redraw-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
        if not run.returncode or target.exists():raise RuntimeError('Redraw accepted missing special icon')
    source=out/'single-ordinary-entry.cmjsession';target=out/'ordinary-with-old-content.cmjsession'
    run=subprocess.run([binary,str(old_pack),str(source),'--redraw-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
    if run.returncode or target.read_bytes()!=(out/'single-ordinary-redraw.cmjsession').read_bytes():raise RuntimeError('Prior content cannot redraw subtype0')
    # A previously chosen special subtype also survives later polls. A fresh
    # captured fixture is explicitly placed at the existing poll boundary.
    poll_results=[]
    # The four direction entries for own-area defense start after the three
    # title bytes and the first two direction rows in the content10 section.
    directions=packed[20077:20081]
    unchanged=next(i for i,action in enumerate(directions) if action==2)
    changed=next(i for i,action in enumerate(directions) if action not in (2,255))
    for name,pressed in (('blink',0),('back',64),('unchanged',1<<unchanged),('changed',1<<changed)):
        raw=bytearray(seeds['last']);raw[0x61e]=1;raw[0x61f]=raw[0x620]=0
        raw[0x60b:0x60f]=bytes((4,2,255,255));raw[0x606:0x60a]=bytes((0,1,0,0))
        data=bytearray(poll_snapshot(raw,70));data[10]=ord('w')
        source=out/f'poll-{name}-entry.cmjsession';source.write_bytes(data)
        report,final=trace_defense_poll(prg,raw,pressed)
        if report['boundary']!='poll_wait':raise RuntimeError('Unexpected special subtype poll boundary')
        target=out/f'poll-{name}-result.cmjsession'
        run=subprocess.run([binary,pack,str(source),'--poll-keeper-area-defense',str(pressed),'--capture',str(target)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
        expected=bytearray(poll_snapshot(final,70));expected[10]=ord('w')
        if target.read_bytes()!=expected:raise RuntimeError(f'{name}: special subtype poll state mismatch')
        if name=='unchanged' and final[0x607]!=1:raise RuntimeError('Unchanged action lost special subtype')
        if name=='changed' and final[0x607]!=0:raise RuntimeError('Changed action retained old special subtype')
        requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
        if requests!=report['requests']:raise RuntimeError(f'{name}: special subtype poll requests mismatch')
        if name!='changed':
            rejected=out/f'poll-{name}-missing-icon.cmjsession'
            run=subprocess.run([binary,str(old_pack),str(source),'--poll-keeper-area-defense',str(pressed),'--capture',str(rejected)],capture_output=True,timeout=30)
            if not run.returncode or rejected.exists():raise RuntimeError(f'{name}: old pack accepted special icon')
        poll_results.append(name)
    source=out/'last-special-entry.cmjsession';target=out/'interactive.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'redraw-keeper-area-special\nsave {target}\nquit\n',capture_output=True,text=True,timeout=30)
    if run.returncode or 'Error:' in run.stdout or target.read_bytes()!=(out/'last-special-redraw.cmjsession').read_bytes():raise RuntimeError('Interactive redraw differs from flag operation')
    # Unsupported synthetic option indices must not read adjacent ROM data.
    invalid=bytearray((out/'single-special-entry.cmjsession').read_bytes())
    invalid[384]=invalid[389]=127
    source=out/'unsupported-option-entry.cmjsession';source.write_bytes(invalid)
    target=out/'unsupported-option-result.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--redraw-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('Unsupported special subtype accepted')
    # An older snapshot cannot carry newly supported nonzero choice subtypes.
    old=bytearray((out/'poll-blink-entry.cmjsession').read_bytes());old[10]=ord('s')
    source=out/'prior-version-special.cmjsession';source.write_bytes(old)
    target=out/'prior-version-special-result.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('Prior version accepted a special subtype')
    summary={key:sum(r['decision']==key for r in results) for key in ('accepted','denied','canceled')}
    (out/'verification.json').write_text(json.dumps({'cases':results,'summary':summary,'subtype_poll_cases':poll_results,
        'scope':'Constructed selected action/subtype prefixes on captured area state. Original D3F4 runs through decision and redraw to D41F before PLP/input tail; warning61 is outside compared redraw requests because phase76 already emitted it. Full represented state and ordered clear/count/slot icon/optional display22 requests compared. Host frame acknowledgements; no NMI/audio/rendering, post-redraw input tail or stamina debit.'},indent=2)+'\n')
    print(f'{len(results)} defensive special redraw cases match: {summary}: {out}')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare ordinary defensive registration and full-prefix return to private ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries, poll_snapshot
from trace_area_defense_poll import trace_defense_poll
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()))
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());results=[]

    def compare(name,entry,pressed,preparation=False,source=None):
        phase=71 if preparation else 70
        if source is None:
            source=out/f'{name}-entry.cmjsession';source.write_bytes(poll_snapshot(entry,phase))
        report,final=trace_defense_poll(prg,entry,pressed,preparation=preparation)
        expected_phase={'complete':72,'poll_wait':70,'preparation_pending':71}[report['boundary']]
        target=out/f'{name}-native.cmjsession'
        operation=['--prepare-keeper-area-defense'] if preparation else ['--poll-keeper-area-defense',str(pressed)]
        run=subprocess.run([binary,pack,str(source),*operation,'--capture',str(target)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
        actual,expected=target.read_bytes(),poll_snapshot(final,expected_phase)
        if actual!=expected:raise RuntimeError(f'{name}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
        requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
        if requests!=report['requests']:raise RuntimeError(f'{name}: requests {requests} != {report["requests"]}')
        results.append({'name':name,'phase':expected_phase,'requests':len(requests)})
        return final,target

    for slot in range(4):
        for player in range(1,11):
            ids=[i for i in range(1,11) if i!=player][:3];ids.insert(slot,player)
            for action in range(3,7):
                for pressed in (128,144,192,208):
                    entry=bytearray(seeds['last']);entry[0x61e]=slot
                    entry[0x601:0x605]=bytes(ids);entry[0x60b:0x60f]=bytes([4]*4)
                    entry[0x60b+slot]=action;entry[0x442]=player;entry[0x1e]=pressed
                    compare(f's{slot}-p{player}-a{action}-b{pressed}',entry,pressed,True)

    # Last-slot waits have count1..4; A exits, B returns, idle retains UI state.
    for count in range(1,5):
        entry=bytearray(seeds['last']);entry[0x600]=entry[0x61e]=count
        entry[0x601:0x606]=bytes([*range(1,count+1),*([0]*(5-count))])
        entry[0x60b:0x60f]=bytes([*([4]*count),*([255]*(4-count))])
        for pressed in range(256):
            if pressed&15 and not pressed&192:continue
            compare(f'full{count}-b{pressed}',entry,pressed)

    # Chained sequence consumes native save outputs, not fresh expected fixtures.
    entry=bytearray(seeds['last']);entry[0x61e]=0;entry[0x60b:0x60f]=bytes([4,5,6,3])
    source=None
    for slot in range(4):
        entry,source=compare(f'chain{slot}-confirm',entry,128,source=source)
        entry,source=compare(f'chain{slot}-register',entry,128,True,source)
    entry,source=compare('chain-complete',entry,128,source=source)

    # Interactive uses the same operation and saved result as the flag interface.
    initial=out/'s3-p10-a6-b208-entry.cmjsession'
    expected=out/'s3-p10-a6-b208-native.cmjsession'
    interactive=out/'interactive.cmjsession'
    run=subprocess.run([binary,pack,str(initial),'--interactive'],
        input=f'prepare-keeper-area-defense\nsave {interactive}\nquit\n',capture_output=True,text=True,timeout=30)
    if run.returncode or 'Error:' in run.stdout or interactive.read_bytes()!=expected.read_bytes():
        raise RuntimeError('Interactive registration differs from flag operation')

    # Unsupported paths must leave no capture behind; no special-menu bypass.
    rejected=[]
    for name,action,slot,pressed in (('special0',0,0,128),('special1',1,0,128),('special2',2,0,128),('full-direction',4,3,129)):
        entry=bytearray(seeds['last']);entry[0x61e]=slot;entry[0x60b+slot]=action
        entry[0x442]=entry[0x601+slot];entry[0x1e]=pressed
        source=out/f'{name}-entry.cmjsession';source.write_bytes(poll_snapshot(entry,71))
        target=out/f'{name}-rejected.cmjsession'
        run=subprocess.run([binary,pack,str(source),'--prepare-keeper-area-defense','--capture',str(target)],capture_output=True,timeout=30)
        if not run.returncode or target.exists():raise RuntimeError(f'{name}: unsupported registration accepted')
        rejected.append(name)
    (out/'verification.json').write_text(json.dumps({'cases':results,'rejected':rejected,
        'scope':'Constructed ordinary field actions3..6 across4 slots/10 player IDs/4 pressed samples, full-prefix count1..4 polls and one chained four-defender sequence. Complete represented snapshots and ordered requests compared. Starts D3D4 before stat loader or D3AB after sampled yield; stops D3A6 or bank26:8B8F. Host presentation/frame acknowledgements; no special options, stamina charge, AI, actual renderer/NMI/audio or area-action execution.'},indent=2)+'\n')
    print(f'{len(results)} ordinary defensive registration/return comparisons match: {out}')


if __name__=='__main__':main()

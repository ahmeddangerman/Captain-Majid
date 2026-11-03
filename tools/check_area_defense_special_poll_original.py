#!/usr/bin/env python3
"""Compare every sampled special chooser input to the approved original ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries,poll_snapshot
from trace_area_defense_special_open import trace_special_open
from trace_area_defense_special_poll import trace_special_poll
from verify_action_strength import PAYLOAD_SHA256


def checkpoint(raw,phase,option,cursor,pressed=0):
    # Preserve the q prefix and append r's cursor/exit-sample bytes explicitly.
    data=bytearray(poll_snapshot(raw,phase)[:383]);data[10]=ord('z')
    data+=bytes((1,option,cursor,pressed))+bytes(25)
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes();packed=a.pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    if packed[:8]==b'CMJSHOTJ':packed=b'CMJSHOTI'+packed[8:21286]
    if packed[:8]==b'CMJSHOTI':packed=b'CMJSHOTH'+packed[8:21274]
    if packed[:8]==b'CMJSHOTH':packed=b'CMJSHOTG'+packed[8:20717]
    if packed[:8]==b'CMJSHOTG':packed=b'CMJSHOTF'+packed[8:20224]
    if packed[:8]==b'CMJSHOTF':packed=b'CMJSHOTE'+packed[8:-1]
    if packed[:8]!=b'CMJSHOTE':p.error('Expected private version14E pack')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    count=int.from_bytes(packed[20088:20090],'little');pos=20090;rule=None
    for player_type in range(count):
        present=packed[pos];pos+=1
        if present:
            subtype,conditional,required=packed[pos:pos+3];pos+=3
            if rule is None:rule=(player_type,subtype,required if conditional else None)
    if rule is None or pos+3!=len(packed):raise RuntimeError('Missing private special rule or chooser geometry')
    raw=bytearray(dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active'])
    actor=1;raw[0x30c],raw[0x60b],raw[0x442],raw[0x44e]=rule[0],2,actor,1
    if rule[2] is not None:raw[0x300+12*10]=rule[2]
    report,opened=trace_special_open(prg,raw,128)
    if report['option']!=rule[1]:raise RuntimeError('Original opening did not expose expected option')
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());results=[]
    for phase,cursor in ((73,0),(74,0),(74,1)):
        for option in (0,1,127):
            source=out/f'phase{phase}-c{cursor}-option{option}.cmjsession'
            source.write_bytes(checkpoint(opened,phase,option,cursor))
            for pressed in range(256):
                report,final=trace_special_poll(prg,opened,pressed,cursor,option)
                decided=report['boundary']=='decided';expected_phase=75 if decided else 74
                expected=checkpoint(final,expected_phase,option,report['cursor'],pressed if decided else 0)
                target=out/f'p{phase}-c{cursor}-o{option}-b{pressed}.cmjsession'
                run=subprocess.run([binary,pack,str(source),'--poll-keeper-area-special',str(pressed),'--capture',str(target)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'{source.name} b{pressed}: {run.stderr}')
                actual=target.read_bytes()
                if actual!=expected:raise RuntimeError(f'{source.name} b{pressed}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
                y=re.search(r'Special cursor y (\d+)',run.stdout)
                if not y or int(y[1])!=report['y']:raise RuntimeError(f'{source.name} b{pressed}: cursor position differs')
                if decided:
                    native_result=(option if actual[385] else 0) if actual[386]&128 else 128
                    if native_result!=report['result']:raise RuntimeError('Saved outcome does not determine original result')
                results.append({'phase':phase,'cursor':cursor,'option':option,'pressed':pressed,'result':report['result']})
    source=out/'phase74-c1-option1.cmjsession';target=out/'interactive.cmjsession'
    flag=out/'p74-c1-o1-b128.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'poll-keeper-area-special 128\nsave {target}\nquit\n',text=True,capture_output=True,timeout=30)
    if run.returncode or 'Error:' in run.stdout or not target.exists() or target.read_bytes()!=flag.read_bytes():raise RuntimeError('Interactive input differs from flag')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'All256 sampled bytes from phase73 and phase74 cursors0/1, synthetic options0/1/127 on captured area state. Original CFA9 runs to CFA4 wait or D3F4 before preparation; full represented state, cursor Y, and selected/cancel result match. Host resumes after explicit cooperative yield; no rendering/NMI/audio, cost loading, stamina charge, or move execution.'},indent=2)+'\n')
    print(f'{len(results)} special chooser sampled inputs match: {out}')


if __name__=='__main__':main()

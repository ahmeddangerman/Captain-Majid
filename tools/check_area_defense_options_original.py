#!/usr/bin/env python3
"""Compare raw-type defensive special availability and ordinary fallback to ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries, poll_snapshot
from trace_area_defense_poll import trace_defense_poll
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes();packed=a.pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    if packed[:8]!=b'CMJSHOTC':p.error('Expected private version12C pack')
    # Canonical content11 prefix size, followed by the new bounded rule section.
    start=20088;count=int.from_bytes(packed[start:start+2],'little');pos=start+2;rules=[];offsets=[]
    if not 1<=count<=256:raise RuntimeError('Invalid private type count')
    for _ in range(count):
        offsets.append(pos);present=packed[pos];pos+=1;rule=None
        if present:
            subtype,conditional,required=packed[pos:pos+3];pos+=3
            rule=(subtype,required if conditional else None)
        rules.append(rule)
    if pos!=len(packed):raise RuntimeError('Unexpected private rule section size')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    base=dict(defense_entries(prg,a.keeper_ram.read_bytes()))['active']
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());results=[];registered=0;special_rejections=0
    for player_type in range(count):
        actor=1+player_type%10
        for partner in (False,True):
            for variant in (1,2):
                for action in (0,1,2):
                    name=f't{player_type}-p{int(partner)}-v{variant}-a{action}'
                    raw=bytearray(base);raw[0x601]=raw[0x442]=actor
                    for player in range(1,11):raw[0x300+12*player]=0
                    raw[0x300+12*actor]=player_type
                    rule=rules[player_type]
                    if partner and rule and rule[1] is not None:
                        teammate=10 if actor!=10 else 1;raw[0x300+12*teammate]=rule[1]
                    raw[0x60b],raw[0x44e],raw[0x1e]=action,variant,128
                    source=out/f'{name}-entry.cmjsession';source.write_bytes(poll_snapshot(raw,71))
                    report,_=trace_defense_poll(prg,raw,128,preparation=True,options_only=True)
                    target=out/f'{name}-inspect.cmjsession'
                    run=subprocess.run([binary,pack,str(source),'--inspect-keeper-area-defense','--capture',str(target)],capture_output=True,text=True,timeout=30)
                    if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
                    match=re.search(r'Special defensive option: (none|\d+)',run.stdout)
                    option=None if match and match[1]=='none' else int(match[1]) if match else 'missing'
                    if option!=report['option'] or target.read_bytes()!=source.read_bytes():raise RuntimeError(f'{name}: availability/state mismatch')
                    target=out/f'{name}-registered.cmjsession'
                    run=subprocess.run([binary,pack,str(source),'--prepare-keeper-area-defense','--capture',str(target)],capture_output=True,text=True,timeout=30)
                    if option is not None:
                        if not run.returncode or target.exists():raise RuntimeError(f'{name}: special choice silently registered')
                        special_rejections+=1
                    else:
                        if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
                        registered+=1;tail,final=trace_defense_poll(prg,raw,128,preparation=True)
                        if tail['boundary']!='poll_wait' or target.read_bytes()!=poll_snapshot(final,70):raise RuntimeError(f'{name}: ordinary registration differs')
                        requests=[{'kind':int(k),'id':int(v),'slot':int(slot)}for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
                        if requests!=tail['requests']:raise RuntimeError(f'{name}: registration requests differ')
                    results.append({'type':player_type,'partner':partner,'variant':variant,'action':action,'option':option})

    # Malformed new rules and missing older content must fail without writing a save.
    source=out/'t0-p0-v1-a2-entry.cmjsession';bad_cases=[]
    bad=bytearray(packed);bad[start:start+2]=bytes(2);bad_cases.append(bad)
    bad=bytearray(packed);bad[start:start+2]=(257).to_bytes(2,'little');bad_cases.append(bad)
    bad=bytearray(packed);bad[offsets[0]]=2;bad_cases.append(bad)
    conditional=next(offsets[i] for i,r in enumerate(rules) if r and r[1] is not None)
    unconditional=next(offsets[i] for i,r in enumerate(rules) if r and r[1] is None)
    for offset,value in ((conditional+1,128),(conditional+2,2),(unconditional+3,1)):
        bad=bytearray(packed);bad[offset]=value;bad_cases.append(bad)
    prior=bytearray(packed[:start]);prior[7]=ord('B')
    bad_cases.extend((packed[:-1],packed+b'\0',prior))
    for index,bad in enumerate(bad_cases):
        bad_pack=out/f'bad{index}.cmjpack';bad_pack.write_bytes(bad);target=out/f'bad{index}.cmjsession'
        run=subprocess.run([binary,str(bad_pack),str(source),'--inspect-keeper-area-defense','--capture',str(target)],capture_output=True,timeout=30)
        if not run.returncode or target.exists():raise RuntimeError(f'Invalid/missing option content accepted {index}')
    # Type beyond the imported pointer table must not index ROM code or guess.
    raw=bytearray(base);raw[0x601]=raw[0x442]=1;raw[0x30c]=count;raw[0x60b]=2;raw[0x1e]=128
    source=out/'unknown-type.cmjsession';source.write_bytes(poll_snapshot(raw,71));target=out/'unknown-result.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--inspect-keeper-area-defense','--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('Unknown raw option type accepted')
    (out/'verification.json').write_text(json.dumps({'cases':results,'ordinary_registrations':registered,'special_rejections':special_rejections,
        'invalid_packs':len(bad_cases),'scope':'Every imported raw type with partner absent/present, variants1/2 and actions0/1/2. Actor cycles through IDs1..10. Original D3D4 executes stat bank loader to D3E6; ordinary cases also continue to D3A6 and compare full represented state/requests. Available specials reject preparation; no selection or stamina charge. Controlled RAM on captured menu, explicit parent returns and host frame/display acknowledgements; not live match/presentation.'},indent=2)+'\n')
    print(f'{len(results)} original availability checks, {registered} ordinary registrations, {special_rejections} special-choice pauses match: {out}')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare native special chooser opening with the approved original ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries, poll_snapshot
from trace_area_defense_special_open import trace_special_open
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes();packed=a.pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    if packed[:8]!=b'CMJSHOTD':p.error('Expected private version13D pack')
    start=20088;count=int.from_bytes(packed[start:start+2],'little');pos=start+2;rules=[]
    if not 1<=count<=256:raise RuntimeError('Invalid option rule count')
    for player_type in range(count):
        present=packed[pos];pos+=1
        if present:
            subtype,conditional,required=packed[pos:pos+3];pos+=3
            rules.append((player_type,subtype,required if conditional else None))
    if pos+2!=len(packed):raise RuntimeError('Unexpected private chooser geometry section')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    seeds=dict(defense_entries(prg,a.keeper_ram.read_bytes()))
    binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve());results=[]
    for player_type,subtype,required in rules:
        for actor in (1,5,10):
            for count in (1,4):
                for slot in range(count):
                    for pressed in (128,129,192,255):
                        name=f't{player_type}-a{actor}-n{count}-s{slot}-b{pressed}'
                        raw=bytearray(seeds['active' if count==1 else 'last'])
                        ids=[i for i in range(1,11) if i!=actor][:count-1];ids.insert(slot,actor)
                        raw[0x601:0x606]=bytes(ids+[0]*(5-count))
                        raw[0x61e]=slot;raw[0x60b:0x60f]=bytes([4]*count+[255]*(4-count));raw[0x60b+slot]=2
                        for player in range(1,11):raw[0x300+12*player]=0
                        raw[0x300+12*actor]=player_type
                        if required is not None:
                            teammate=10 if actor!=10 else 1;raw[0x300+12*teammate]=required
                        raw[0x442],raw[0x44e],raw[0x1e]=actor,1,pressed
                        source=out/f'{name}-entry.cmjsession';data=bytearray(poll_snapshot(raw,71)[:383]);data[10]=ord('p');source.write_bytes(data)
                        report,final=trace_special_open(prg,raw,pressed)
                        if report['option']!=subtype:raise RuntimeError(f'{name}: unexpected original option')
                        target=out/f'{name}-opened.cmjsession'
                        run=subprocess.run([binary,pack,str(source),'--open-keeper-area-special','--capture',str(target)],capture_output=True,text=True,timeout=30)
                        if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
                        expected=bytearray(poll_snapshot(final,73)[:383]);expected[10]=ord('q');expected+=bytes([1,report['option']])
                        actual=target.read_bytes()
                        if actual!=expected:raise RuntimeError(f'{name}: state mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]} (lengths {len(actual)}/{len(expected)})')
                        requests=[{'kind':int(k),'id':int(v),'slot':int(s)} for k,v,s in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
                        if requests!=report['requests']:raise RuntimeError(f'{name}: display request mismatch')
                        geometry=re.search(r'Defensive cursor \| tile (\d+) \| x (\d+) \| attribute (\d+)',run.stdout)
                        if not geometry or tuple(map(int,geometry.groups()))!=(report['tile'],report['x'],report['attribute']):
                            raise RuntimeError(f'{name}: cursor geometry mismatch')
                        results.append({'type':player_type,'actor':actor,'count':count,'slot':slot,'pressed':pressed,'frames':report['frames']})
    first=results[0];name=f"t{first['type']}-a{first['actor']}-n{first['count']}-s{first['slot']}-b{first['pressed']}"
    source=out/f'{name}-entry.cmjsession';opened=out/f'{name}-opened.cmjsession'
    interactive=out/'interactive.cmjsession'
    run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'open-keeper-area-special\nsave {interactive}\nquit\n',capture_output=True,text=True,timeout=30)
    if run.returncode or 'Error:' in run.stdout or not interactive.exists() or interactive.read_bytes()!=opened.read_bytes():
        raise RuntimeError('Interactive chooser opening differs from flag operation')
    previous=bytearray(packed[:-2]);previous[7]=ord('C')
    for index,bad in enumerate((packed[:-1],packed+b'\0',bytes(previous))):
        badpack=out/f'bad{index}.cmjpack';badpack.write_bytes(bad);target=out/f'bad{index}.cmjsession'
        run=subprocess.run([binary,str(badpack),str(source),'--open-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
        if not run.returncode or target.exists():raise RuntimeError(f'Bad/missing cursor content accepted {index}')
    target=out/'replay.cmjsession'
    run=subprocess.run([binary,pack,str(opened),'--open-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('Repeated chooser opening accepted')
    legacy=bytearray(opened.read_bytes()[:383]);legacy[10]=ord('p')
    legacy_source=out/'bad-legacy.cmjsession';legacy_source.write_bytes(legacy);target=out/'bad-legacy-output.cmjsession'
    run=subprocess.run([binary,pack,str(legacy_source),'--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('New chooser phase accepted as old snapshot')
    type_count=int.from_bytes(packed[start:start+2],'little')
    no_rule=next(i for i in range(type_count) if all(r[0]!=i for r in rules))
    raw=bytearray(seeds['active']);raw[0x30c]=no_rule;raw[0x60b]=2;raw[0x442]=1;raw[0x1e]=128
    data=bytearray(poll_snapshot(raw,71)[:383]);data[10]=ord('p')
    no_option=out/'no-option.cmjsession';no_option.write_bytes(data);target=out/'no-option-output.cmjsession'
    run=subprocess.run([binary,pack,str(no_option),'--open-keeper-area-special','--capture',str(target)],capture_output=True,timeout=30)
    if not run.returncode or target.exists():raise RuntimeError('Chooser opened with no available option')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Controlled available options on captured area state, varied acting field ID, defender count/slot, conditional teammate, and original confirm sample. Original CPU executes D3D4 through CFA4 before first chooser yield, with explicit host parent returns and frame acknowledgements. Complete represented session and ordered display/cursor requests compared; original $02FC prior Y remains unrepresented. No chooser input, stamina handling or actual rendering.'},indent=2)+'\n')
    print(f'{len(results)} special chooser openings match: {out}')


if __name__=='__main__':main()

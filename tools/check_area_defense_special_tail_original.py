#!/usr/bin/env python3
"""Compare special-choice redraw continuation with the approved original ROM."""
import argparse,hashlib,json,re,subprocess
from pathlib import Path
from check_area_defense_poll_original import defense_entries,poll_snapshot
from check_area_defense_special_redraw_original import snapshot
from trace_area_defense_special_redraw import trace_special_redraw
from trace_area_defense_special_tail import trace_special_tail
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):parser.add_argument(name,type=Path)
    args=parser.parse_args();rom=args.rom.read_bytes();packed=args.pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected approved Japanese ROM')
    if packed[:8]!=b'CMJSHOTF':parser.error('Expected private version15F pack')
    out=args.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
    seeds=dict(defense_entries(prg,args.keeper_ram.read_bytes()))
    binary=str(Path('build/captain-phase-session').resolve());pack=str(args.pack.resolve());results=[];unsupported=[]
    fixtures=[('single',1,0,[2,255,255,255],[1,0,0,0]),
              ('middle',4,1,[2,2,255,255],[1,1,0,0]),
              ('last',4,3,[2,4,6,2],[1,0,0,1])]
    modes=[('ordinary',0,0,range(128,256)),('special',1,200,range(128,256)),
           ('denied',1,0,range(128,256)),('cancel',1,0,range(64,128))]
    for name,count,slot,actions,subtypes in fixtures:
        for mode,cursor,stamina,samples in modes:
            for pressed in samples:
                raw=bytearray(seeds['active' if count==1 else 'last']);actor=slot+1
                raw[0x600],raw[0x61e]=count,slot;raw[0x601:0x606]=bytes(list(range(1,count+1))+[0]*(5-count))
                raw[0x60b:0x60f]=bytes(actions);raw[0x606:0x60a]=bytes(subtypes)
                record=0x300+12*actor;raw[record],raw[record+1],raw[record+2],raw[record+3]=23,stamina&255,stamina>>8,0
                raw[0x43d],raw[0x442],raw[0x44e],raw[0x1e]=2,actor,1,pressed
                redraw,pre=trace_special_redraw(prg,raw,pressed,cursor,1)
                decision=redraw['decision'];case=f'{name}-{mode}-b{pressed}'
                # Original direction processing at a full prefix uses scratch
                # slots not included in the native saved model.
                unrepresented=decision['status']!='canceled' and pre[0x61e]==count and not pressed&64 and bool(pressed&15)
                source=out/f'{case}-entry.cmjsession';source.write_bytes(snapshot(pre,77,cursor,pressed,1,decision))
                target=out/f'{case}-native.cmjsession'
                run=subprocess.run([binary,pack,str(source),'--continue-keeper-area-special','--capture',str(target)],capture_output=True,text=True,timeout=30)
                if unrepresented:
                    if not run.returncode or target.exists():raise RuntimeError(f'{case}: full-prefix scratch direction accepted')
                    unsupported.append(case);continue
                if run.returncode:raise RuntimeError(f'{case}: {run.stderr}')
                report,final=trace_special_tail(prg,raw,pressed,cursor,1)
                expected=bytearray(poll_snapshot(final,70));expected[10]=129
                actual=target.read_bytes()
                if actual!=expected:raise RuntimeError(f'{case}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]} lengths {len(actual)}/{len(expected)}')
                requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
                if requests!=report['requests']:raise RuntimeError(f'{case}: {requests} != {report["requests"]}')
                results.append({'fixture':name,'choice':mode,'pressed':pressed,'decision':decision['status'],'cursor':final[0x61e],'requests':len(requests)})
    (out/'verification.json').write_text(json.dumps({'cases':results,'unsupported_full_direction':len(unsupported),
        'scope':'Constructed choice prefixes on captured area state, all A/B samples with varied directions, accepted/denied/canceled outcomes and single/multiple selected defenders. Original D3F4 runs through warning/redraw then D41F to D3A6; only post-D41F requests compared. Full represented saved state compared. Full-prefix unrepresented direction scratch rejects. Host frame acknowledgements; no live rendering/NMI/audio or match execution.'},indent=2)+'\n')
    print(f'{len(results)} saved-input tails match; {len(unsupported)} unrepresented full-prefix directions reject: {out}')


if __name__=='__main__':main()

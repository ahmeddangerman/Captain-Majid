#!/usr/bin/env python3
"""Compare initial human defensive-choice state in route2 keeper-area play."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_keeper_entry_original import observe
from check_keeper_saves_original import observe_keeper_save
from check_kickoff_save_handoffs_original import snapshot
from trace_ball_recovery_phase import trace_recovery
from trace_keeper_area_menu import trace_keeper_area_menu
from verify_action_strength import PAYLOAD_SHA256


def defense_snapshot(ram,phase):
    data=bytearray(snapshot(ram,phase));data[370:372]=bytes([1,ram[0x61a]])
    if phase in (67,69):
        # Old subtypes cannot be read while action==FF; direction selection clears
        # the subtype before use. Canonicalize these semantically unused bytes.
        data[372:382]=bytes([1,*ram[0x60b:0x60f],0,0,0,0,ram[0x620]])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    p.add_argument('--prompt',action='store_true')
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];base=bytearray(a.keeper_ram.read_bytes())
    base[0x612],base[0x616],base[0x43d],base[0x43e]=2,0,0,0
    base,_,_=observe_keeper_save(prg,base);_,base=trace_recovery(prg,base);_,base=trace_recovery(prg,base,continue_handoff=True)
    binary=str(Path('build/captain-phase-session').resolve());results=[]
    for count in (0,1,4):
        for blink in (0,7,255):
            for flags in (0,128,211):
                raw=bytearray(base);raw[0x5fb],raw[0x441],raw[0x5fd]=11,12,1
                raw[0x635],raw[0x637],raw[0x5fe]=204,124,233
                for slot in (*range(1,11),*range(12,22)):
                    raw[0x306+12*slot],raw[0x308+12*slot],raw[0x30a+12*slot]=200 if slot<=count else 120,124,0
                raw,_,_=observe(prg,raw,0x8b4a,{0x8b73});raw=bytearray(raw)
                raw[0x61e],raw[0x61f],raw[0x620]=211,flags,blink
                raw[0x606:0x60a]=bytes([37,38,39,40])
                entry_report,entry=trace_keeper_area_menu(prg,raw)
                if entry_report['boundary']!='defensive_choice' or entry[0x600]!=count:raise RuntimeError('Expected defensive entry prefix')
                report,after=trace_keeper_area_menu(prg,raw,defense_setup=True)
                phase=67 if report['boundary']=='defense_initial' else 68
                name=f'n{count}-blink{blink}-flags{flags}';source=out/f'{name}-entry.cmjsession';source.write_bytes(defense_snapshot(entry,66))
                target=out/f'{name}-native.cmjsession'
                run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--begin-keeper-area-defense',str(entry[0x620]),'--capture',str(target)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(run.stderr)
                actual,expected=target.read_bytes(),defense_snapshot(after,phase)
                if actual!=expected:raise RuntimeError(f'{name} mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                prompt_checked=False
                if a.prompt and phase==67:
                    prompt_report,prompt_after=trace_keeper_area_menu(prg,raw,defense_prompt=True)
                    if prompt_report['boundary']!='defense_prompt':raise RuntimeError('Expected defensive prompt')
                    prompt_target=out/f'{name}-prompt.cmjsession'
                    prompt_run=subprocess.run([binary,str(a.pack.resolve()),str(target),'--open-keeper-area-defense','--capture',str(prompt_target)],capture_output=True,text=True,timeout=30)
                    if prompt_run.returncode:raise RuntimeError(prompt_run.stderr)
                    if prompt_target.read_bytes()!=defense_snapshot(prompt_after,69):raise RuntimeError('Defensive prompt state mismatch')
                    expected_requests=prompt_report['presentation_requests'][len(report['presentation_requests']):]
                    actual_requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',prompt_run.stdout)]
                    if actual_requests!=expected_requests:raise RuntimeError(f'Defensive prompt requests {actual_requests} != {expected_requests}')
                    if subprocess.run([binary,str(a.pack.resolve()),str(prompt_target),'--open-keeper-area-defense'],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Prompt replay accepted')
                    prompt_checked=True
                results.append({'count':count,'blink':blink,'flags':flags,'phase':phase,'instructions':report['instructions'],'prompt_checked':prompt_checked})
    if a.prompt:
        packed=a.pack.read_bytes()
        if packed[:8] not in (b'CMJSHOTA',b'CMJSHOTB',b'CMJSHOTC',b'CMJSHOTD'):raise RuntimeError('Prompt verification needs version10 or later pack')
        if packed[:8]in (b'CMJSHOTC',b'CMJSHOTD'):packed=b'CMJSHOTB'+packed[8:20088]
        tail_size=15 if packed[:8]==b'CMJSHOTA' else 22
        old=bytearray(packed[:-tail_size]);old[7]=ord('9')
        invalid=bytearray(packed);invalid[-1 if tail_size==15 else -8]=7
        for index,bad in enumerate((packed[:-1],packed+b'\0',bytes(invalid),bytes(old))):
            bad_pack=out/f'rejected{index}.cmjpack';bad_pack.write_bytes(bad)
            bad_output=out/f'rejected{index}.cmjsession'
            rejected=subprocess.run([binary,str(bad_pack),str(target),'--open-keeper-area-defense','--capture',str(bad_output)],capture_output=True,timeout=30)
            if rejected.returncode==0 or bad_output.exists():raise RuntimeError('Bad/missing private menu content accepted')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Original full route2 menu observed to defensive entry and replayed to D390 initial state or 8B8F empty return; numeric state/represented bytes compared. Old subtypes behind FF actions canonicalized zero as unread until cleared. Blink captured explicitly; host frame acknowledgements, no actual NMI/PPU/audio, optional prompt compares slot-aware presentation requests and complete state to D3A6. Version10 content required; bad/truncated/trailing/legacy content rejects. No defense input yet.'},indent=2)+'\n')
    print(f'{len(results)} defensive initialization comparisons match: {out}')

if __name__=='__main__':main()

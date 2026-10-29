#!/usr/bin/env python3
"""Compare route2 prepared keeper-area menu state before human or AI choice."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_keeper_entry_original import observe
from check_keeper_saves_original import observe_keeper_save
from check_kickoff_save_handoffs_original import snapshot
from trace_ball_recovery_phase import trace_recovery
from trace_keeper_area_menu import trace_keeper_area_menu
from verify_action_strength import PAYLOAD_SHA256


def area_snapshot(ram, phase):
    data=bytearray(snapshot(ram,phase));data[370:372]=bytes([1,ram[0x61a]])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];base=bytearray(a.keeper_ram.read_bytes())
    base[0x612],base[0x616],base[0x43d],base[0x43e]=2,0,0,0
    base,_,_=observe_keeper_save(prg,base);_,base=trace_recovery(prg,base);_,base=trace_recovery(prg,base,continue_handoff=True)
    binary=str(Path('build/captain-phase-session').resolve());results=[]
    for side,x in ((0,56),(11,204)):
        for near in (False,True):
            for first in (0,255):
                for stun,penalty in ((0,0),(0,24),(1,24)):
                    raw=bytearray(base);raw[0x5fb],raw[0x441],raw[0x635],raw[0x5fe]=side,side+1,x,((x-48)//8)*12+5
                    raw[0x5fd],raw[0xe2],raw[0xe3]=1,first,0
                    for slot in (*range(1,11),*range(12,22)):
                        raw[0x306+12*slot],raw[0x308+12*slot],raw[0x30a+12*slot]=(x+8 if x<128 else x-8) if near else 120,124,int(slot%3==0)
                    raw,_,_=observe(prg,raw,0x8b4a,{0x8b73});raw=bytearray(raw)
                    keeper=11 if side==0 else 0
                    raw[0x307+12*keeper],raw[0x30a+12*keeper]=penalty,stun
                    for address,value in ((0x517,17),(0x532,7),(0x8e,99),(0x469,77),(0x615,85),(0x62d,77),(0x628,35),(0x61e,211)):raw[address]=value
                    report,after=trace_keeper_area_menu(prg,raw)
                    phase=18 if report['boundary']=='human_menu' else 66
                    name=f's{side}-n{int(near)}-r{first}-stun{stun}-p{penalty}'
                    source=out/f'{name}-entry.cmjsession';source.write_bytes(area_snapshot(raw,65))
                    target=out/f'{name}-native.cmjsession'
                    run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--open-prepared-keeper-area-menu',str(after[0x517]),'--capture',str(target)],capture_output=True,text=True,timeout=30)
                    if run.returncode:raise RuntimeError(run.stderr)
                    actual,expected=target.read_bytes(),area_snapshot(after,phase)
                    if actual!=expected:raise RuntimeError(f'{name} mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                    requests=[{'kind':int(kind),'id':int(identity)} for kind,identity in re.findall(r'Presentation request (\d+) \| id (\d+)',run.stdout)]
                    if requests!=report['presentation_requests']:raise RuntimeError(f'{name} request ordering mismatch {requests} != {report["presentation_requests"]}')
                    results.append({'side':side,'near':near,'random':first,'stun':stun,'penalty':penalty,'phase':phase,
                                    'captured_facing':after[0x517],'selected':after[0x600],'instructions':report['instructions']})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Controlled route2 area-menu inputs, original8B73 to D5E5 human menu or D36E AI choice entry. All represented bytes and typed request order compared. Final animation facing supplied explicitly from original observation; its computation is not implemented. Host frame acknowledgements without actual PPU/NMI/audio; no AI choice or menu confirmation.'},indent=2)+'\n')
    print(f'{len(results)} prepared keeper-area menu comparisons match: {out}')

if __name__=='__main__':main()

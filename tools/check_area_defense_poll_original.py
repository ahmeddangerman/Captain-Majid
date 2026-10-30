#!/usr/bin/env python3
"""Compare sampled defensive directions/blinking/back/confirmation to the ROM."""
import argparse, hashlib, json, re, subprocess
from pathlib import Path
from check_keeper_entry_original import observe
from check_keeper_saves_original import observe_keeper_save
from check_kickoff_save_handoffs_original import snapshot
from trace_ball_recovery_phase import trace_recovery
from trace_keeper_area_menu import trace_keeper_area_menu
from trace_area_defense_poll import trace_defense_poll
from verify_action_strength import PAYLOAD_SHA256


def poll_snapshot(ram,phase):
    data=bytearray(snapshot(ram,phase));data[370:372]=bytes([1,ram[0x61a]])
    actions=list(ram[0x60b:0x60f]);subtypes=[ram[0x606+i] if actions[i]!=255 else 0 for i in range(4)]
    data[372:382]=bytes([1,*actions,*subtypes,ram[0x620]])
    data[382]=ram[0x1e] if phase==71 else 0
    return bytes(data)


def defense_entries(prg,keeper_ram):
    base=bytearray(keeper_ram)
    base[0x612],base[0x616],base[0x43d],base[0x43e]=2,0,0,0
    base,_,_=observe_keeper_save(prg,base);_,base=trace_recovery(prg,base);_,base=trace_recovery(prg,base,continue_handoff=True)
    seeds=[('inactive',1,0,[255]*4,0,0),('active',1,0,[4,255,255,255],128,7),
           ('back',4,1,[4,2,255,255],211,0),('last',4,3,[4,2,6,5],128,255)]
    for name,count,cursor,actions,flags,blink in seeds:
        raw=bytearray(base);raw[0x5fb],raw[0x441],raw[0x5fd]=11,12,1
        raw[0x635],raw[0x637],raw[0x5fe]=204,124,233
        for slot in (*range(1,11),*range(12,22)):
            raw[0x306+12*slot],raw[0x308+12*slot],raw[0x30a+12*slot]=200 if slot<=count else 120,124,0
        raw,_,_=observe(prg,raw,0x8b4a,{0x8b73})
        report,ready=trace_keeper_area_menu(prg,raw,defense_prompt=True)
        if report['boundary']!='defense_prompt':raise RuntimeError('Expected defensive prompt')
        entry=bytearray(ready);entry[0x61e],entry[0x61f],entry[0x620]=cursor,flags,blink
        entry[0x60b:0x60f]=bytes(actions);entry[0x606:0x60a]=bytes(4)
        yield name,entry


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_ram','output_dir'):p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160];base=bytearray(a.keeper_ram.read_bytes())
    binary=str(Path('build/captain-phase-session').resolve());results=[]
    for name,entry in defense_entries(prg,base):
        source=out/f'{name}-entry.cmjsession';source.write_bytes(poll_snapshot(entry,70))
        for pressed in range(256):
            report,final=trace_defense_poll(prg,entry,pressed)
            phase=71 if report['boundary']=='preparation_pending' else 70
            target=out/f'{name}-pressed{pressed}.cmjsession'
            run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--poll-keeper-area-defense',str(pressed),'--capture',str(target)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(run.stderr)
            actual,expected=target.read_bytes(),poll_snapshot(final,phase)
            if actual!=expected:raise RuntimeError(f'{name} pressed{pressed}: {[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
            requests=[{'kind':int(k),'id':int(v),'slot':int(slot)} for k,v,slot in re.findall(r'Presentation request (\d+) \| id (\d+) \| slot (\d+)',run.stdout)]
            if requests!=report['requests']:raise RuntimeError(f'{name} pressed{pressed}: request mismatch {requests} != {report["requests"]}')
            if phase==71:
                cur=final[0x61e]
                if final[0x43d]!=final[0x60b+cur] or final[0x442]!=final[0x601+cur]:raise RuntimeError('Pending choice projection mismatch')
            results.append({'seed':name,'pressed':pressed,'phase':phase,'requests':len(requests)})
    packed=a.pack.read_bytes()
    if packed[:8]==b'CMJSHOTC':packed=b'CMJSHOTB'+packed[8:20088]
    if packed[:8]!=b'CMJSHOTB':raise RuntimeError('Defensive input requires version11 pack')
    prior=bytearray(packed[:-7]);prior[7]=ord('A')
    legacy=bytearray(packed[:-22]);legacy[7]=ord('9')
    invalid=bytearray(packed);invalid[-8]=7
    for index,bad in enumerate((packed[:-1],packed+b'\0',bytes(invalid),bytes(prior),bytes(legacy))):
        bad_pack=out/f'rejected{index}.cmjpack';bad_pack.write_bytes(bad)
        bad_output=out/f'rejected{index}.cmjsession'
        rejected=subprocess.run([binary,str(bad_pack),str(source),'--poll-keeper-area-defense','0','--capture',str(bad_output)],capture_output=True,timeout=30)
        if rejected.returncode==0 or bad_output.exists():raise RuntimeError('Invalid/missing defensive icon content accepted')
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Four constructed defensive UI seeds on captured area state; all256 pressed bytes each. All represented state and slot-aware sprite/subject requests compared from originalD3AB sampled input to D3A6 or D3D4 before preparation. Confirmation pressed byte retained in pending snapshots. Pending043D derived from stored selected action; unread FF subtypes canonicalized. Host frame acknowledgements; no stat/energy/special-move preparation or actual rendering/NMI/audio.'},indent=2)+'\n')
    print(f'{len(results)} defensive input comparisons match: {out}')

if __name__=='__main__':main()

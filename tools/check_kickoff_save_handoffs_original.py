#!/usr/bin/env python3
"""Compare controlled keeper non-goal handoffs with retained kickoff positions.

Result2/3 and keeper actions are explicitly supplied post-comparison inputs.
This tests outcome handlers; it does not prove these results arise in a match.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from check_keeper_saves_original import observe_keeper_save, post_save_checkpoint
from check_keeper_entry_original import keeper_checkpoint, observe
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def snapshot(ram, phase):
    data=bytearray(keeper_checkpoint(ram,phase) if phase==56 else post_save_checkpoint(ram,phase))
    data[361:365]=bytes([1,ram[0x28],ram[0x29],ram[0x629]])
    data[365:368]=bytes([1,ram[0x308],ram[0x38c]])
    if phase==65:data[370:372]=bytes([1,ram[0x61a]])
    return bytes(data)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','keeper_dir','output_dir'):p.add_argument(name,type=Path)
    p.add_argument('--punch-recovery',action='store_true')
    p.add_argument('--rebound-recovery',action='store_true')
    p.add_argument('--continue-recovery',action='store_true')
    p.add_argument('--keeper-area',action='store_true')
    a=p.parse_args()
    if a.continue_recovery and not (a.punch_recovery or a.rebound_recovery):p.error('Continuation requires recovery')
    if a.keeper_area and not a.continue_recovery:p.error('Keeper-area setup requires recovery continuation')
    rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(Path('build/captain-phase-session').resolve());results=[]
    for left in range(4):
        for right in range(4):
            base=(a.keeper_dir/f'l{left}-r{right}-keeper-entry.bin').read_bytes()
            for result in (2,3):
                for action in range(4):
                    ram=bytearray(base)
                    ram[0x612],ram[0x616],ram[0x43d],ram[0x43e]=result,0,action,0
                    name=f'l{left}-r{right}-result{result}-action{action}'
                    source=out/f'{name}-input.cmjsession';source.write_bytes(snapshot(ram,56))
                    final,phase,steps=observe_keeper_save(rom[16:262160],ram)
                    target=out/f'{name}-native.cmjsession'
                    run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--resolve-keeper-save','--capture',str(target)],capture_output=True,text=True,timeout=30)
                    if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
                    actual,expected=target.read_bytes(),snapshot(final,phase)
                    if actual!=expected:raise RuntimeError(f'{name}: {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                    replay=subprocess.run([binary,str(a.pack.resolve()),str(target),'--resolve-keeper-save'],capture_output=True,timeout=30)
                    if replay.returncode==0:raise RuntimeError('Keeper reward/outcome replay accepted')
                    recovered=False
                    if a.punch_recovery and phase==58:
                        report,recovery=trace_recovery(rom[16:262160],final,skip_jitter=True)
                        if report['boundary']!='recovered_owner_handoff':raise RuntimeError('Expected recovered field player')
                        recovered_target=out/f'{name}-recovered.cmjsession'
                        run=subprocess.run([binary,str(a.pack.resolve()),str(target),'--recover-keeper-punch','--capture',str(recovered_target)],capture_output=True,text=True,timeout=30)
                        if run.returncode:raise RuntimeError(run.stderr)
                        actual_recovery=recovered_target.read_bytes();wanted_recovery=snapshot(recovery,6)
                        if actual_recovery!=wanted_recovery:raise RuntimeError(f'Punch recovery snapshot mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual_recovery,wanted_recovery)) if x!=y]}')
                        recovered=True
                    rebound_recovered=False
                    if a.rebound_recovery and phase==2:
                        report,recovery=trace_recovery(rom[16:262160],final)
                        if report['boundary']!='recovered_owner_handoff':raise RuntimeError('Rebound requires a separately supported boundary path')
                        recovered_target=out/f'{name}-rebound-recovered.cmjsession'
                        run=subprocess.run([binary,str(a.pack.resolve()),str(target),'--recover','--capture',str(recovered_target)],capture_output=True,text=True,timeout=30)
                        if run.returncode:raise RuntimeError(run.stderr)
                        actual_recovery=recovered_target.read_bytes();wanted_recovery=snapshot(recovery,6)
                        if actual_recovery!=wanted_recovery:raise RuntimeError(f'Rebound recovery mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual_recovery,wanted_recovery)) if x!=y]}')
                        rebound_recovered=True
                    tail_phase=None;control_prepared=False;area_prepared=False
                    if a.continue_recovery and (recovered or rebound_recovered):
                        report,tail=trace_recovery(rom[16:262160],recovery,continue_handoff=True)
                        routes={'player_control_handoff':1,'keeper_area_interaction':7}
                        if report['boundary'] not in routes:raise RuntimeError(f'Unsupported tail {report["boundary"]}')
                        tail_phase=routes[report['boundary']]
                        tail_target=out/f'{name}-continued.cmjsession'
                        run=subprocess.run([binary,str(a.pack.resolve()),str(recovered_target),'--continue-recovery','--capture',str(tail_target)],capture_output=True,text=True,timeout=30)
                        if run.returncode:raise RuntimeError(run.stderr)
                        if tail_target.read_bytes()!=snapshot(tail,tail_phase):raise RuntimeError('Recovery continuation mismatch')
                        if tail_phase==1:
                            report,ready=trace_recovery(rom[16:262160],tail,prepare_control=True)
                            ready_target=out/f'{name}-control-ready.cmjsession'
                            run=subprocess.run([binary,str(a.pack.resolve()),str(tail_target),'--setup-control','--capture',str(ready_target)],capture_output=True,text=True,timeout=30)
                            if run.returncode:raise RuntimeError(run.stderr)
                            if ready_target.read_bytes()!=snapshot(ready,9):raise RuntimeError('Recovered control setup mismatch')
                            control_prepared=True
                    if a.keeper_area and tail_phase==7:
                        area,_,area_steps=observe(rom[16:262160],tail,0x8b4a,{0x8b73})
                        area_target=out/f'{name}-area-prepared.cmjsession'
                        run=subprocess.run([binary,str(a.pack.resolve()),str(tail_target),'--prepare-keeper-area','--capture',str(area_target)],capture_output=True,text=True,timeout=30)
                        if run.returncode:raise RuntimeError(run.stderr)
                        actual_area=area_target.read_bytes();wanted_area=snapshot(area,65)
                        if actual_area!=wanted_area:raise RuntimeError(f'Keeper area setup mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual_area,wanted_area)) if x!=y]}')
                        area_prepared=True
                    results.append({'left':left,'right':right,'input_result':result,'action':action,'phase':phase,'instructions':steps,'punch_recovered':recovered,'rebound_recovered':rebound_recovered,'tail_phase':tail_phase,'control_prepared':control_prepared,'area_prepared':area_prepared})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Constructed result2/3 and action inputs after acknowledged keeper comparison/display9, based on original kickoff positions. All represented outcome bytes match original861C/8646 handlers. No goal classification proof, optional punch recovery includes original movement/recipient selection without a second jitter; optional rebound recovery includes its separate jitter path, optional prepared recovery continuation stops at keeper-area interaction or prepares ordinary control, using disclosed host frame acknowledgements; optional keeper-area numeric setup runs8B4A..8B73 before its menu; no actual display/NMI/audio/PPU.'},indent=2)+'\n')
    print(f'{len(results)} controlled keeper save handoffs match: {out}')

if __name__=='__main__':main()

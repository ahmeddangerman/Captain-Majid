#!/usr/bin/env python3
"""Compare idle kickoff field round with the approved original ROM."""
import argparse, hashlib, json, subprocess
from pathlib import Path
from trace_ball_recovery_phase import trace_recovery
from check_kickoff_control_original import checkpoint
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','pack','observations_dir','output_dir'): p.add_argument(name,type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256: p.error('Expected approved Japanese ROM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]): p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(Path('build/captain-phase-session').resolve());cases=[]
    for side in (0,11):
        for left in range(4):
            for right in range(4):
                ram=bytearray((a.observations_dir/f's{side}-l{left}-r{right}-after.bin').read_bytes());ram[0x614]=0
                poll,ram=trace_recovery(rom[16:262160],ram,control_input=(0,0))
                if poll['boundary']!='control_clock_pending':raise RuntimeError('Expected clock')
                tick,before=trace_recovery(rom[16:262160],ram,control_tick=True)
                if tick['boundary']!='field_round_pending':raise RuntimeError('Expected field round')
                observed,after=trace_recovery(rom[16:262160],before,world_inputs=[(0,0)]*22)
                if observed['boundary']!='player_control_ready':raise RuntimeError(f'Unexpected boundary {observed["boundary"]}')
                name=f's{side}-l{left}-r{right}';source=out/f'{name}-input.cmjsession';source.write_bytes(checkpoint(before,13))
                target=out/f'{name}-native.cmjsession'
                run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--advance-world','0','0','--capture',str(target)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'{name}: {run.stderr}')
                actual,expected=target.read_bytes(),checkpoint(after,9)
                if actual!=expected:raise RuntimeError(f'{name} mismatch {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                cases.append({'side':side,'left':left,'right':right,'instructions':observed['original_instructions']})
    (out/'verification.json').write_text(json.dumps({'cases':cases,'scope':'Prepared kickoff, ordinary clock and idle input at 22 field polls. Full represented bytes compared; display36 acknowledged, host frame acknowledgements disclosed, no real NMI/audio/PPU. Other field inputs and interruptions excluded.'},indent=2)+'\n')
    print(f'{len(cases)} idle kickoff field-round comparisons match: {out}')

if __name__=='__main__':main()

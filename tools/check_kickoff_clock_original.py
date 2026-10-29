#!/usr/bin/env python3
"""Compare ordinary kickoff control clock handoff with original CPU observations."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from trace_ball_recovery_phase import trace_recovery
from check_kickoff_control_original import checkpoint
from verify_action_strength import PAYLOAD_SHA256


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'observations_dir', 'output_dir'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    rom = a.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True, exist_ok=False)
    binary = str(Path('build/captain-phase-session').resolve())
    results=[]
    for side in (0,11):
        for left in range(4):
            for right in range(4):
                base=bytearray((a.observations_dir / f's{side}-l{left}-r{right}-after.bin').read_bytes())
                base[0x614]=0
                poll, before=trace_recovery(rom[16:262160],base,control_input=(0,0))
                if poll['boundary']!='control_clock_pending':
                    raise RuntimeError('Expected control clock boundary')
                report, after=trace_recovery(rom[16:262160],before,control_tick=True)
                if report['boundary']!='field_round_pending':
                    raise RuntimeError(f'Expected ordinary field round, got {report["boundary"]}')
                name=f's{side}-l{left}-r{right}'
                source=out / f'{name}-input.cmjsession';source.write_bytes(checkpoint(before,10))
                target=out / f'{name}-native.cmjsession'
                run=subprocess.run([binary,str(a.pack.resolve()),str(source),'--advance-control','--capture',str(target)],capture_output=True,text=True,timeout=30)
                if run.returncode:
                    raise RuntimeError(f'{name}: {run.stderr}')
                actual,expected=target.read_bytes(),checkpoint(after,13)
                if actual!=expected:
                    raise RuntimeError(f'{name} mismatch: {[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                results.append({'side':side,'left':left,'right':right,'instructions':report['original_instructions']})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Prepared kickoff, ordinary input and E1E7 clock/energy to field-round boundary. All represented bytes compared. Original display36 acknowledged, frame host acknowledgements disclosed. Added time, period end, goal-area and subsequent field round excluded.'},indent=2)+'\n')
    print(f'{len(results)} ordinary kickoff clock comparisons match: {out}')


if __name__=='__main__':
    main()

#!/usr/bin/env python3
"""Compare kickoff menu opening after a saved late field-round interruption."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

from check_kickoff_control_original import checkpoint
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
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
    prg, results = rom[16:262160], []
    binary = str(Path('build/captain-phase-session').resolve())
    for left in range(4):
        for right in range(4):
            for mode in ('direct', 'late'):
                ram = bytearray((a.observations_dir / f's0-l{left}-r{right}-after.bin').read_bytes())
                if mode == 'late':
                    ram[0x614] = 0
                    _, ram = trace_recovery(prg, ram, control_input=(0, 0))
                    _, ram = trace_recovery(prg, ram, control_tick=True)
                    samples = [(0, 0)] * 22
                    samples[6] = (64, 0)
                    report, before = trace_recovery(prg, ram, world_inputs=samples)
                else:
                    report, before = trace_recovery(prg, ram, control_input=(64, 0))
                if report['boundary'] != 'action_menu_pending':
                    raise RuntimeError('Expected original menu request')
                report, after = trace_menu(prg, before)
                if report['boundary'] != 'human_action_menu':
                    raise RuntimeError('Expected opened menu')
                name = f'l{left}-r{right}-{mode}'
                source = out / f'{name}-input.cmjsession'
                source.write_bytes(checkpoint(before, 11))
                target = out / f'{name}-native.cmjsession'
                run = subprocess.run([binary, str(a.pack.resolve()), str(source), '--open-action-menu',
                                      '--capture', str(target)], capture_output=True, text=True, timeout=30)
                if run.returncode:
                    raise RuntimeError(f'{name}: {run.stderr}')
                actual, expected = target.read_bytes(), checkpoint(after, 18)
                if actual != expected:
                    raise RuntimeError(f'{name} mismatch: '
                                       f'{[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                results.append({'left': left, 'right': right, 'mode': mode,
                                'instructions': report['original_instructions']})
    (out / 'verification.json').write_text(json.dumps({
        'cases': results,
        'scope': 'Prepared kickoff to direct/late menu request and original menu opening. '
                 'All represented bytes including keeper Y compared. Host frame acknowledgements '
                 'replace actual PPU/NMI/audio timing; original kickoff display is assumed completed. '
                 'Menu confirmation and later attacks are outside scope.'
    }, indent=2) + '\n')
    print(f'{len(results)} kickoff menu-opening comparisons match: {out}')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare chained kickoff menu choices and prepared ordinary shots to original CPU."""
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
    binary, pack = str(Path('build/captain-phase-session').resolve()), str(a.pack.resolve())
    for left in range(4):
        for right in range(4):
            for mode in ('direct', 'late'):
                base = bytearray((a.observations_dir / f's0-l{left}-r{right}-after.bin').read_bytes())
                if mode == 'late':
                    base[0x614] = 0
                    _, ram = trace_recovery(prg, base, control_input=(0, 0))
                    _, ram = trace_recovery(prg, ram, control_tick=True)
                    samples = [(0, 0)] * 22
                    samples[6] = (64, 0)
                    report, ram = trace_recovery(prg, ram, world_inputs=samples)
                else:
                    report, ram = trace_recovery(prg, base, control_input=(64, 0))
                if report['boundary'] != 'action_menu_pending':
                    raise RuntimeError('Expected menu request')
                for choice, inputs in [('combined', [129]), ('staged', [128, 1, 1, 0, 128])]:
                    name = f'l{left}-r{right}-{mode}-{choice}'
                    source = out / f'{name}-input.cmjsession'
                    source.write_bytes(checkpoint(ram, 11))
                    index = 0

                    def compare(after, phase, operation):
                        nonlocal source, index
                        index += 1
                        target = out / f'{name}-{index}.cmjsession'
                        run = subprocess.run([binary, pack, str(source), *operation, '--capture', str(target)],
                                             capture_output=True, text=True, timeout=30)
                        if run.returncode:
                            raise RuntimeError(f'{name} {operation}: {run.stderr}')
                        actual, expected = target.read_bytes(), checkpoint(after, phase)
                        if actual != expected:
                            raise RuntimeError(f'{name} {operation} mismatch: '
                                               f'{[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                        source = target

                    report, current = trace_menu(prg, ram)
                    compare(current, 18, ['--open-action-menu'])
                    for pressed in inputs:
                        report, current = trace_menu(prg, current, mode='poll', pressed=pressed)
                        phase = 19 if report['boundary'] == 'attack_choice_pending' else 18
                        compare(current, phase, ['--menu-input', str(pressed)])
                    if phase != 19 or current[0x43b] != 0:
                        raise RuntimeError('Expected confirmed ordinary shot')
                    owner = current[0x441]
                    record = 0x300 + 12 * owner
                    stamina_before = current[record+1] | current[record+2] << 8
                    report, current = trace_menu(prg, current, mode='basic-shot')
                    compare(current, 20, ['--prepare-basic-shot'])
                    replay = subprocess.run([binary, pack, str(source), '--prepare-basic-shot'],
                                            capture_output=True, timeout=30)
                    if replay.returncode == 0:
                        raise RuntimeError('Prepared shot accepted a second stamina charge')
                    results.append({'left': left, 'right': right, 'mode': mode, 'choice': choice,
                                    'matched_boundaries': index, 'stamina_before': stamina_before,
                                    'stamina_after':current[record+1] | current[record+2] << 8,
                                    'power':current[0x61c] | current[0x61d] << 8})
    (out / 'verification.json').write_text(json.dumps({
        'cases': results,
        'scope': 'Native outputs feed consecutive menu and shot operations; all represented bytes compared. '
                 'Original observations use cooperative host frame acknowledgements; kickoff display36 assumed '
                 'completed. Basic shot uses explicit ordinary subtype0/variant0 and synthetic0200 return in '
                 'the original observer. Special moves, later defender/flight stages and real NMI/PPU/audio excluded.'
    }, indent=2) + '\n')
    print(f'{len(results)} kickoff shot-choice sequences match: {out}')


if __name__ == '__main__':
    main()

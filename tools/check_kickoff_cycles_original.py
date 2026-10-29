#!/usr/bin/env python3
"""Compare chained kickoff control cycles with explicit original CPU observations."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import subprocess

from check_kickoff_control_original import checkpoint
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256

PHASES = {'control_poll_wait': 9, 'control_clock_pending': 10,
          'action_menu_pending': 11, 'ai_cell_decision': 12,
          'field_round_pending': 13, 'player_control_ready': 9}


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
    binary, pack = str(Path('build/captain-phase-session').resolve()), str(a.pack.resolve())
    results, counts = [], Counter()
    patterns = [('right', 1), ('left', 2), ('down', 4), ('up', 8),
                ('diagonal', 9), ('opposite', 3), ('cycle-defender', 1), ('late-menu', 0)]
    for side in (0, 11):
        for left in range(4):
            for right in range(4):
                base = (a.observations_dir / f's{side}-l{left}-r{right}-after.bin').read_bytes()
                for pattern, held in patterns:
                    name = f's{side}-l{left}-r{right}-{pattern}'
                    ram, phase, index = base, 9, 0
                    source = out / f'{name}-0.cmjsession'
                    source.write_bytes(checkpoint(ram, 9))

                    def compare(report, after, operation):
                        nonlocal source, ram, phase, index
                        boundary = report['boundary']
                        if boundary not in PHASES:
                            raise RuntimeError(f'{name}: uncovered original boundary {boundary}')
                        phase = PHASES[boundary]
                        index += 1
                        target = out / f'{name}-{index}.cmjsession'
                        run = subprocess.run([binary, pack, str(source), *operation, '--capture', str(target)],
                                             capture_output=True, text=True, timeout=30)
                        if run.returncode:
                            raise RuntimeError(f'{name} at {boundary}: {run.stderr}')
                        actual, expected = target.read_bytes(), checkpoint(after, phase)
                        if actual != expected:
                            raise RuntimeError(f'{name} step {index}: '
                                               f'{[(i,x,y) for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]}')
                        counts[boundary] += 1
                        source, ram = target, after

                    rounds = 0
                    for cycle in range(2):
                        for poll in range(12):
                            if phase != 9:
                                break
                            pressed = 128 if pattern == 'cycle-defender' and side == 11 else 0
                            report, after = trace_recovery(rom[16:262160], ram, control_input=(held, pressed))
                            compare(report, after, ['--poll-control', str(held), str(pressed)])
                        if phase != 10:
                            break
                        report, after = trace_recovery(rom[16:262160], ram, control_tick=True)
                        compare(report, after, ['--advance-control'])
                        samples = [(held, 0)] * 22
                        if pattern == 'cycle-defender' and side == 11:
                            samples = [(held, 128 if slot % 2 else 64) for slot in range(22)]
                        if pattern == 'late-menu' and side == 0:
                            samples[6] = (64, 0)
                        content = out / f'{name}-inputs-{cycle}.txt'
                        content.write_text(''.join(f'{h} {p}\n' for h, p in samples))
                        report, after = trace_recovery(rom[16:262160], ram, world_inputs=samples)
                        compare(report, after, ['--advance-world-file', str(content)])
                        rounds += 1
                        if phase != 9:
                            break
                    results.append({'side': side, 'left': left, 'right': right, 'pattern': pattern,
                                    'matched_boundaries': index, 'rounds': rounds, 'final_phase': phase})
    (out / 'verification.json').write_text(json.dumps({
        'cases': results, 'boundaries': dict(counts),
        'scope': 'Each native result feeds the next native command; no re-export between commands. '
                 'Original observations use explicit held/pressed inputs and host frame acknowledgements. '
                 'Starts after prepared kickoff/control setup. Stops at menu/AI boundaries or two rounds. '
                 'No real NMI, PPU, audio, complete match, or original kickoff display.'
    }, indent=2) + '\n')
    print(f'{len(results)} chained cases; {sum(counts.values())} boundaries match: {out}')


if __name__ == '__main__':
    main()

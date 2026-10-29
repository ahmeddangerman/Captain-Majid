#!/usr/bin/env python3
"""Compare prepared kickoff control polling with the approved original ROM."""
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
    cases = []
    boundaries = {'control_poll_wait':9, 'control_clock_pending':10,
                  'action_menu_pending':11, 'ai_cell_decision':12}
    inputs = [(0, 0), (1, 0), (2, 0), (4, 0), (8, 0), (64, 0),
              (0, 64), (0, 128), (0, 192), (9, 128)]
    for side in (0, 11):
        base = bytearray((a.observations_dir / f's{side}-l0-r0-after.bin').read_bytes())
        for timer, changed_cell in ((0, False), (0, True), (1, False), (10, False)):
            for held, pressed in inputs:
                ram = bytearray(base)
                ram[0x614] = timer
                if changed_cell and side == 11:
                    ram[0x617] = 255
                report, after = trace_recovery(rom[16:262160], ram, control_input=(held, pressed))
                if report['boundary'] not in boundaries:
                    raise RuntimeError(f'Unexpected boundary: {report["boundary"]}')
                phase = boundaries[report['boundary']]
                name = f's{side}-t{timer}-h{held}-p{pressed}-cell{int(changed_cell)}'
                source = out / f'{name}-input.cmjsession'
                source.write_bytes(checkpoint(ram, 9))
                target = out / f'{name}-native.cmjsession'
                run = subprocess.run([binary, str(a.pack.resolve()), str(source), '--poll-control',
                                      str(held), str(pressed), '--capture', str(target)],
                                     capture_output=True, text=True, timeout=30)
                if run.returncode:
                    raise RuntimeError(f'{name}: native failed: {run.stderr}')
                expected = checkpoint(after, phase)
                actual = target.read_bytes()
                if actual != expected:
                    raise RuntimeError(f'{name} mismatch: '
                                       f'{[(i, x, y) for i, (x, y) in enumerate(zip(actual, expected)) if x != y]}')
                cases.append({'side':side,'timer':timer,'held':held,'pressed':pressed, 'changed_cell':changed_cell,
                              'boundary':report['boundary'], 'instructions':report['original_instructions']})
    (out / 'verification.json').write_text(json.dumps({
        'cases':cases,
        'scope':'Original E14A polling with disclosed host frame acknowledgements. All represented bytes, including keeper Y, compared. Inputs start after prepared kickoff control setup; original kickoff display is assumed acknowledged. No actual PPU, NMI, audio or full match.'
    }, indent=2) + '\n')
    print(f'{len(cases)} kickoff control poll comparisons match: {out}')


if __name__ == '__main__':
    main()

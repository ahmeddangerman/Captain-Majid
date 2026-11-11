#!/usr/bin/env python3
"""Compare unopposed pass field movement after constructed original control states."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from check_empty_pass_original import observe
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args();rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside the checkout')
    prg = rom[16:262160];count = 0
    with tempfile.TemporaryDirectory(prefix='pass-field-handoff-', dir=args.private_temp) as temporary:
        before, after, raw_file = (Path(temporary) / name for name in ('before.cmjsession', 'after.cmjsession', 'raw.bin'))
        for fixture in range(4):
            ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes());ready[0x614] = 0
            _, poll = trace_recovery(prg, ready, control_input=(1, 0))
            _, clock = trace_recovery(prg, poll, control_tick=True)
            report, moved = trace_recovery(prg, clock, world_inputs=[(1, 0)] * 22)
            if report['boundary'] != 'player_control_ready':raise RuntimeError('Expected original unopposed control')
            for elapsed in (0, 1, 2, 5):
                for action in range(7):
                    source = bytearray(moved);source[0x5ff] = elapsed;source[0x43d] = action
                    checkpoints = []
                    handoff, _, _, _, _, _ = observe(prg, source, 104, 100, checkpoints=checkpoints)
                    clock_entry = [entry for entry in checkpoints if entry[0:2] == (30, 0xd193)]
                    field_entry = [entry for entry in checkpoints if entry[0:2] == (31, 0xe074)]
                    if len(clock_entry) != 1 or clock_entry[0][2] != 0 or len(field_entry) != 1:
                        raise RuntimeError('Expected a single zero-elapsed pass clock before field movement')
                    field_input = field_entry[0][3]
                    before.write_bytes(export_session(field_input, 19));after.write_bytes(export_session(handoff));raw_file.write_bytes(field_input)
                    result = subprocess.run([str(args.exe.resolve()), '--compare-pass-field-handoff', str(before), str(after),
                                             str(raw_file), str(args.pack.resolve())], capture_output=True, text=True)
                    if result.returncode:
                        raise RuntimeError(f'fixture={fixture}, elapsed={elapsed}, action={action}: {result.stderr}')
                    count += 1
    print(f'{count} constructed original pass field-movement/handoff comparisons matched')


if __name__ == '__main__':
    main()

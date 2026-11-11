#!/usr/bin/env python3
"""Compare provisional ordinary human shot preparation with constructed original CPU cases."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from export_phase_session import export_session
from check_kickoff_shot_launch_original import observe_clock
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_field_loop import trace_loop
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside the checkout')
    prg = rom[16:262160]
    matched = 0
    with tempfile.TemporaryDirectory(prefix='human-shot-', dir=args.private_temp) as temp:
        before_file, after_file = Path(temp) / 'before.cmjsession', Path(temp) / 'after.cmjsession'
        for fixture in range(4):
            for stamina in (80, 81, 100, 255, 400, 900):
                ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes())
                owner = ready[0x441]
                ready[0x301 + 12 * owner:0x303 + 12 * owner] = stamina.to_bytes(2, 'little')
                _, menu_input = trace_recovery(prg, ready, control_input=(64, 0))
                _, opened = trace_menu(prg, menu_input, mode='open')
                report, before = trace_menu(prg, opened, mode='poll', pressed=129)
                if report['boundary'] != 'attack_choice_pending' or before[0x43b] or before[0x600]:
                    raise RuntimeError('Expected unopposed ordinary shot choice')
                _, after = trace_menu(prg, before, mode='basic-shot')
                before_file.write_bytes(export_session(before, 19))
                after_file.write_bytes(export_session(after, 20))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina}: {result.stderr}')
                clocked, _, _, _ = observe_clock(prg, after)
                before_file.write_bytes(export_session(after, 20))
                after_file.write_bytes(export_session(clocked))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-clock', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina} dispatch: {result.stderr}')
                movement = bytearray()
                trace_loop(prg, clocked, dispatch_only=True, final_ram=movement)
                after_file.write_bytes(export_session(movement))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-movement', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina} movement: {result.stderr}')
                matched += 1
    print(f'{matched} constructed original unopposed shot preparations, dispatch clocks and field movements matched')


if __name__ == '__main__':
    main()

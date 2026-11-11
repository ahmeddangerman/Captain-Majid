#!/usr/bin/env python3
"""Observe ball-fraction preservation in constructed original control cycles."""
import argparse
import contextlib
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from trace_ball_recovery_phase import trace_recovery
from export_phase_session import export_session
from verify_action_strength import PAYLOAD_SHA256


def fractions(ram):
    return ram[0x634], ram[0x636]


def demo_randoms(seed=1):
    for _ in range(3):
        seed ^= (seed << 13) & 0xffffffff
        seed ^= seed >> 17
        seed ^= (seed << 5) & 0xffffffff
        yield seed & 255, (seed >> 8) & 255


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--ready-dir', type=Path, action='append', required=True)
    for name in ('pack', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if any((args.pack, args.exe, args.private_temp)) and not all((args.pack, args.exe, args.private_temp)):
        parser.error('Native comparisons need --pack, --exe and --private-temp together')
    if args.private_temp and args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside the checkout')
    prg = rom[16:262160]
    counts = Counter()
    temporary = tempfile.TemporaryDirectory(prefix='control-fraction-', dir=args.private_temp) if args.private_temp else contextlib.nullcontext(None)
    with temporary as temp:
      for directory in args.ready_dir:
        for fixture in range(4):
            ready = (directory / f'{fixture}-ready.bin').read_bytes()
            for xfrac, yfrac in ((0, 255), (1, 127), (127, 1), (255, 0), fractions(ready)):
                base = bytearray(ready)
                base[0x634], base[0x636], base[0x614] = xfrac, yfrac, 0
                setup_input = bytearray(base)
                setup_input[0x635] = 124 if base[0x635] != 124 else 116
                setup_input[0x637] = 124 if base[0x637] != 124 else 116
                _, setup = trace_recovery(prg, setup_input, prepare_control=True)
                if fractions(setup) != (xfrac, yfrac):
                    raise RuntimeError(f'Control setup changed fractions: {directory.name}/{fixture}')
                if setup[0x635] == setup_input[0x635] and setup[0x637] == setup_input[0x637]:
                    raise RuntimeError('Control setup did not exercise a coordinate refresh')
                counts['setup'] += 1
                for held in range(16):
                    native = temp is not None and held in (0, 1, 2, 4, 8)
                    randoms = list(demo_randoms())
                    poll_input = bytearray(base)
                    if native: poll_input[0xe2], poll_input[0xe3] = randoms[0]
                    report, polled = trace_recovery(prg, poll_input, control_input=(held, 0))
                    if fractions(polled) != (xfrac, yfrac):
                        raise RuntimeError(f'Control poll changed fractions: {directory.name}/{fixture}/{held}')
                    counts['poll:' + report['boundary']] += 1
                    if report['boundary'] != 'control_clock_pending':
                        continue
                    clock_input = bytearray(polled)
                    if native: clock_input[0xe2], clock_input[0xe3] = randoms[1]
                    report, clocked = trace_recovery(prg, clock_input, control_tick=True)
                    if fractions(clocked) != (xfrac, yfrac):
                        raise RuntimeError(f'Control clock changed fractions: {directory.name}/{fixture}/{held}')
                    counts['clock:' + report['boundary']] += 1
                    if report['boundary'] != 'field_round_pending':
                        continue
                    field_input = bytearray(clocked)
                    if native: field_input[0xe2], field_input[0xe3] = randoms[2]
                    report, after = trace_recovery(prg, field_input, world_inputs=[(held, 0)] * 22)
                    if fractions(after) != (xfrac, yfrac):
                        raise RuntimeError(f'Field round changed fractions: {directory.name}/{fixture}/{held}')
                    counts['field:' + report['boundary']] += 1
                    if native and report['boundary'] == 'player_control_ready':
                        files = [Path(temp) / name for name in ('source.cmjsession', 'poll.bin', 'clock.bin', 'field.bin')]
                        for path, data in zip(files, (export_session(base, 9), polled, clocked, after)):
                            path.write_bytes(data)
                        result = subprocess.run([str(args.exe.resolve()), '--compare-control-fractions', *(str(path) for path in files),
                                                 str(args.pack.resolve()), str(held)], capture_output=True, text=True)
                        if result.returncode:
                            raise RuntimeError(f'Native control {directory.name}/{fixture}/{held}/{xfrac}/{yfrac}: {result.stderr}')
                        counts['native-control-cycles'] += 1
    original_count = sum(value for name, value in counts.items() if not name.startswith('native-'))
    print(f'Original ball fractions remained unchanged at {original_count} constructed boundaries: {dict(sorted(counts.items()))}')


if __name__ == '__main__':
    main()

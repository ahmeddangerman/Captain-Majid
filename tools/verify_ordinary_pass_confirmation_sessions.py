#!/usr/bin/env python3
"""Compare confirmed ordinary-pass targets against private original RAM pairs.

The supplied *-128-before/after.ram pairs cover explicitly selected A input.
Multiple allied candidates are expected to stop at the original chooser and
must be rejected by the native single/empty-target operation.
"""
import argparse
import subprocess
import sys
from pathlib import Path

from verify_captured_pass_target import compare_target, compare_confirmed_target
from verify_observed_world_round import checkpoint
from verify_player7_pass_end_to_end import prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('observations', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-cases', type=int, required=True)
    args = parser.parse_args()
    sources = sorted(args.observations.glob('*-128-before.ram'))
    if args.expected_cases <= 0 or len(sources) != args.expected_cases:
        raise ValueError(f'Expected {args.expected_cases} observations, found {len(sources)}')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    successes = chooser = 0
    for index, source in enumerate(sources, 1):
        tag = source.name[:-len('-before.ram')]
        before, after = source.read_bytes(), (args.observations / f'{tag}-after.ram').read_bytes()
        entry = output / f'{index:03d}-entry.cmjsession'
        entry.write_bytes(checkpoint(before, 19))
        initialized = output / f'{index:03d}-initialized.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(entry),
                              '--begin-ordinary-human-pass-target', '--capture', str(initialized)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'case {tag}, boundary confirmation.setup: {run.stderr.strip()}')
        adjusted = bytearray(initialized.read_bytes())
        if len(adjusted) != 428 or adjusted[10] != 148 or before[0x430] > 4:
            raise ValueError(f'case {tag}, boundary confirmation.setup: invalid candidate layout')
        adjusted[419] = before[0x624]
        count, allies = before[0x430], before[0x625]
        adjusted[421:428] = bytes((1, count, allies, *(before[0x431 + i] if i < count else 0 for i in range(4))))
        prior = output / f'{index:03d}-target.cmjsession'
        prior.write_bytes(adjusted)
        try:
            compare_target(adjusted, before)
        except ValueError as error:
            raise ValueError(f'case {tag}, {error}'.replace('target.initial', 'confirmation.input')) from error
        destination = output / f'{index:03d}-confirmed.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(prior),
                              '--confirm-ordinary-human-pass-target', '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if allies > 1:
            if not run.returncode or destination.exists():
                raise ValueError(f'case {tag}, boundary confirmation.chooser: multiple allies were not rejected atomically')
            chooser += 1
            continue
        if run.returncode:
            raise RuntimeError(f'case {tag}, boundary confirmation.result: {run.stderr.strip()}')
        try:
            compare_confirmed_target(destination.read_bytes(), after)
        except ValueError as error:
            raise ValueError(f'case {tag}, {error}') from error
        successes += 1
    print(f'Matched {successes} original confirmation checkpoints and rejected {chooser} original multi-teammate chooser boundaries.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Ordinary pass confirmation comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

#!/usr/bin/env python3
"""Compare ordinary-pass cursor polls against private original RAM pairs.

Each *-before.ram and *-after.ram pair supplies a constructed D80A poll with
explicit held input in its filename. UI/frame waits were acknowledged by the
original observation harness; confirmation and cancellation are not covered.
"""
import argparse
import subprocess
import sys
from pathlib import Path

from verify_captured_pass_target import compare_target
from verify_observed_world_round import checkpoint
from verify_player7_pass_end_to_end import prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('observations', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-cases', type=int, required=True)
    args = parser.parse_args()
    sources = sorted(args.observations.glob('*-before.ram'))
    if args.expected_cases <= 0 or len(sources) != args.expected_cases:
        raise ValueError(f'Expected {args.expected_cases} original polls, found {len(sources)}')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    for index, source in enumerate(sources, 1):
        tag = source.name[:-len('-before.ram')]
        try:
            _, _, held = map(int, tag.split('-'))
        except ValueError as error:
            raise ValueError(f'Invalid observation name {source.name}') from error
        before, after = source.read_bytes(), (args.observations / f'{tag}-after.ram').read_bytes()
        entry = output / f'{index:03d}-entry.cmjsession'
        entry.write_bytes(checkpoint(before, 19))
        initialized = output / f'{index:03d}-initialized.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(entry),
                              '--begin-ordinary-human-pass-target', '--capture', str(initialized)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'case {tag}, boundary cursor.setup: {run.stderr.strip()}')
        adjusted = bytearray(initialized.read_bytes())
        adjusted[419] = before[0x624]
        prior = output / f'{index:03d}-cursor.cmjsession'
        prior.write_bytes(adjusted)
        try:
            compare_target(adjusted, before)
        except ValueError as error:
            raise ValueError(f'case {tag}, {error}'.replace('target.initial', 'cursor.input')) from error
        destination = output / f'{index:03d}-result.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(prior),
                              '--poll-ordinary-human-pass-cursor', str(held), '0', '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'case {tag}, boundary cursor.poll: {run.stderr.strip()}')
        try:
            compare_target(destination.read_bytes(), after)
        except ValueError as error:
            raise ValueError(f'case {tag}, {error}'.replace('target.initial', 'cursor.result')) from error
    print(f'Matched {len(sources)} complete captured cursor-poll checkpoints with supplied held input and acknowledged UI waits.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Ordinary pass cursor session comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

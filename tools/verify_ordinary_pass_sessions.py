#!/usr/bin/env python3
"""Compare saved ordinary-pass target initialization against private RAM pairs.

Each directory contains matching *-selected.ram and *-target.ram original
observations. Acknowledged ordinary pass-type selection is an explicit input.
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
    parser.add_argument('--observations', type=Path, action='append', required=True)
    for name in ('pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-cases', type=int, required=True)
    args = parser.parse_args()
    if len(args.observations) < 2 or args.expected_cases <= 0:
        raise ValueError('At least two observation directories and a positive expected count are required')
    pairs = [(directory, source) for directory in args.observations for source in sorted(directory.glob('*-selected.ram'))]
    if len(pairs) != args.expected_cases:
        raise ValueError(f'Expected {args.expected_cases} observations, found {len(pairs)}')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    for index, (directory, source) in enumerate(pairs, 1):
        original = directory / source.name.replace('-selected.ram', '-target.ram')
        raw, target = source.read_bytes(), original.read_bytes()
        entry = output / f'{index:03d}-entry.cmjsession'
        entry.write_bytes(checkpoint(raw, 19))
        destination = output / f'{index:03d}-target.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(entry),
                              '--begin-ordinary-human-pass-target', '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'case {index}, boundary target.initial: {run.stderr.strip()}')
        try:
            compare_target(destination.read_bytes(), target)
        except ValueError as error:
            raise ValueError(f'case {index}, {error}') from error
    print(f'Matched {len(pairs)} complete target-initialization checkpoints across {len(args.observations)} original encounter contexts.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Ordinary pass session comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

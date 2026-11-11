#!/usr/bin/env python3
"""Compare a provisional neutral control run with an original trace (not parity)."""
import argparse
import re
import subprocess
from pathlib import Path
from verify_captured_control_chain import trace_data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'pack', 'session', 'exe'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--start', type=int, required=True)
    parser.add_argument('--polls', type=int, default=11)
    parser.add_argument('--seed', type=int, default=1)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    if pcs[args.start] != 'E145' or not 1 <= args.polls <= 10000:
        raise ValueError('Expected original control poll start and valid count')
    positions = [i for i in range(args.start+1, len(pcs)) if pcs[i] == 'E14A'][:args.polls]
    if len(positions) != args.polls:
        raise ValueError('Not enough original polls')
    command = [str(args.exe.resolve()), str(args.pack.resolve()), str(args.session.resolve()), '--demo', str(args.seed), str(args.polls)]
    first = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
    second = subprocess.run(command, capture_output=True, text=True, timeout=30, check=True)
    if first.stdout != second.stdout or 'PROVISIONAL DEMO' not in first.stdout:
        raise ValueError('Demo did not report its mode or was nondeterministic')
    actual = [(int(x), int(y)) for x, y in re.findall(r'random (\d+),(\d+)', first.stdout)]
    expected = [(ram(i)[0xe2], ram(i)[0xe3]) for i in positions]
    if len(actual) != args.polls:
        raise ValueError('Demo stopped before requested poll count')
    differences = sum(a != b for a, b in zip(actual, expected))
    print(f'{len(actual)} deterministic demo polls; {differences} random pairs differ from original. Final demo {actual[-1]}, original {expected[-1]}.')


if __name__ == '__main__':
    main()

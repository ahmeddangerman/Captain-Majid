#!/usr/bin/env python3
"""Strict field-defender preparation research input and fixture preservation."""
import argparse
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--binary', type=Path, default=Path('build/captain-prototype'))
    binary = str(parser.parse_args().binary.resolve())
    # Action, situation, subtype, level; 31 skills; 15 selectors; five move
    # lists (count, four fields per move); then 192 curve entries.
    fields = [2, 0, 0, 3] + [20] * 31 + [6] * 15
    for _ in range(5):
        fields += [1, 77, 4, 52, 254]
    fields += [255 - i for i in range(192)]
    line = ' '.join(map(str, fields))
    run = subprocess.run([binary, '--field-defender-preparation-batch'],
                         input=line + '\n', text=True, capture_output=True, timeout=10)
    assert run.returncode == 0, run.stderr
    assert run.stdout.strip() == '6 30 225 77 564', run.stdout
    malformed = [line + ' 0', ' '.join(map(str, fields[:-1])), '-1' + line[1:],
                 '256' + line[1:]]
    for index, value in ((0, 5), (1, 3), (2, 63), (41, 0), (41, 31), (50, 65)):
        bad = fields.copy()
        bad[index] = value
        malformed.append(' '.join(map(str, bad)))
    for bad in malformed:
        run = subprocess.run([binary, '--field-defender-preparation-batch'],
                             input=bad + '\n', text=True, capture_output=True, timeout=10)
        assert run.returncode == 2 and not run.stdout, (bad, run.stdout, run.stderr)
    commands = ['state', 'defenderprep ' + line, 'state',
                'defenderprep ' + malformed[0], 'state', 'quit']
    run = subprocess.run([binary, '--replica'], input='\n'.join(commands) + '\n',
                         text=True, capture_output=True, check=True, timeout=10)
    replies = run.stdout.splitlines()
    assert replies[0] == replies[2] == replies[4], replies
    assert replies[1] == '6 30 225 77 564' and replies[3] == 'error malformed', replies
    print('Field defender preparation CLI checks passed')


if __name__ == '__main__':
    main()

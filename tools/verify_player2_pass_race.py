#!/usr/bin/env python3
"""Compare a same-cell numeric pass race with an explicitly captured original timeout."""
import argparse
import subprocess
import sys
from pathlib import Path

from export_field_state import export_state
from verify_captured_control_chain import trace_data


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('probe', 'pack', 'session', 'race-table', 'trace-directory'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    if [pcs[i] for i in (21282, 21283, 21284, 21300)] != ['DE64', 'E7CF', 'DE96', 'DED4']:
        raise ValueError('Unexpected original same-cell race boundaries')
    before, after = ram(21284), ram(21300)
    if (before[0xe1], after[0xe1]) != (70, 71):
        raise ValueError('Unexpected original random event ordering')
    result = subprocess.run([str(args.probe.resolve()), str(args.pack.resolve()), str(args.session.resolve()),
                             str(args.race_table.resolve())], capture_output=True, text=True, check=True)
    rows = result.stdout.splitlines()
    if len(rows) != 21:
        raise ValueError('Unexpected diagnostic output length')
    rounds, remaining, active, opposing, winner = map(int, rows[0].split())
    if (rounds, remaining, active, opposing, winner) != (2, after[0x430], after[0x431], after[0x432], -1):
        raise ValueError('Numeric contest does not match the original timeout')
    field = export_state(after)
    for slot, row in enumerate(rows[1:]):
        x, y, target, status = map(int, row.split())
        actual = bytes((x & 255, x >> 8, y & 255, y >> 8, target, status))
        if actual != field[111 + 6 * slot:117 + 6 * slot]:
            raise ValueError(f'Player motion slot {slot} differs from original timeout')
    print('Matched timeout, candidates and 20 player motion records; original random index 70 to 71 is supplied separately.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        print(f'Pass race comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

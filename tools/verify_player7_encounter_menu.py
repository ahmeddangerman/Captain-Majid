#!/usr/bin/env python3
"""Compare one captured player7 encounter and its original menu button edges.

The original trace supplies NMI random/travel and two presentation samples;
this replay does not simulate the original frame or presentation scheduler.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_player7_pass_end_to_end import compare_checkpoint, prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    encounter, sample, menu = 8106, 8159, 8711
    if [pcs[i] for i in (encounter, sample, menu)] != ['E4BE', 'C9C5', 'D5E5']:
        raise ValueError('Unexpected player7 encounter trace boundaries')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source, index, operation = args.source.resolve(), ram(encounter)[0xe1], 0

    def compare(path, at, phase, label):
        compare_checkpoint(path.read_bytes(), ram(at), phase, phase, 'player7', label, None, False, False)

    def run(option, *values):
        nonlocal source, operation
        operation += 1
        destination = output / f'{operation:04d}.cmjsession'
        process = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source), option,
                                  *map(str, values), '--capture', str(destination)],
                                 capture_output=True, text=True, timeout=30)
        if process.returncode:
            raise RuntimeError(f'{option}: {process.stderr.strip()}')
        source = destination
        return process.stdout

    def wait(start, stop, samples=()):
        nonlocal index
        rows, last = menu_wait_events(lines, pcs, ram, start, stop)
        events = output / f'{operation + 1:04d}-events.txt'
        events.write_text('CMJNMISEQ1\n' + '\n'.join([*samples, *rows]) + '\n')
        result = run('--advance-observed-nmi-sequence', events, index)
        found = re.findall(r'final random index (\d+);', result)
        if found != [str(ram(last)[0xe1])]:
            raise ValueError('Captured encounter random index differs')
        index = int(found[0])

    compare(source, encounter, 15, 'encounter')
    wait(encounter, sample)
    wait(sample - 1, menu, ('F 0', 'D 0'))
    run('--open-action-menu')
    compare(source, menu, 18, 'menu entry')
    previous, count = menu, 0
    for at in range(menu + 1, len(pcs)):
        if pcs[at] != 'D5EA' or at <= previous:
            continue
        wait(previous, at)
        compare(source, at, 18, f'poll {count + 1} input')
        run('--menu-input', ram(at)[0x1e])
        end = next(i for i in range(at + 1, len(pcs)) if pcs[i] in ('D5E5', 'D67C', 'D5EA'))
        if pcs[end] == 'D5EA':
            raise ValueError('Missing original menu result boundary')
        if any(pcs[i] == 'C9C5' for i in range(at + 1, end)):
            wait(at, end)
        phase = 19 if pcs[end] == 'D67C' else 18
        compare(source, end, phase, f'poll {count + 1} result')
        count += 1
        previous = end
        if phase == 19:
            print(f'Matched player7 encounter menu and {count} polls through shot confirmation; '
                  f'final random index {index}. Frame and presentation timing supplied.')
            return
    raise ValueError('No captured shot confirmation')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player7 encounter replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

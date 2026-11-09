#!/usr/bin/env python3
"""Replay captured original target-cursor polls with explicit wait events.

Random/travel inputs and changes to the control motion-request byte are read
from private instruction-boundary RAM. The latter are samples of presentation
state, not simulation of the original presentation task.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_captured_pass_target import compare_target, compare_confirmed_target
from verify_player7_pass_end_to_end import prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--initial-index', type=int, required=True)
    parser.add_argument('--start-frame', type=int, default=0)
    parser.add_argument('--end-frame', type=int, required=True)
    parser.add_argument('--expected-polls', type=int, required=True)
    parser.add_argument('--expected-moves', type=int, required=True)
    parser.add_argument('--confirm', action='store_true', help='compare the next original A-only target confirmation')
    args = parser.parse_args()
    if not 0 <= args.initial_index <= 255 or args.expected_polls <= 0 or args.expected_moves < 0:
        raise ValueError('Initial index must be a byte and expected counts valid')
    lines, pcs, ram = trace_data(args.trace_directory)
    points = [i for i, pc in enumerate(pcs) if pc == 'D80A' and args.start_frame <= int(lines[i].split()[-1][6:]) <= args.end_frame]
    if len(points) != args.expected_polls + 1:
        raise ValueError(f'Expected {args.expected_polls} polls, found {max(0, len(points) - 1)}')
    source = args.source.resolve()
    try:
        compare_target(source.read_bytes(), ram(points[0]))
    except ValueError as error:
        raise ValueError(str(error).replace('target.initial', 'cursor.entry')) from error
    random_index = args.initial_index
    if random_index != ram(points[0])[0xe1]:
        raise ValueError(f'boundary cursor.entry, random.index: native {random_index}, original {ram(points[0])[0xe1]}')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    operations = moves = motion_samples = 0

    def events_between(source_path, index, start, stop):
        nonlocal operations, motion_samples
        if not any(pcs[j] == 'C9C5' for j in range(start + 1, stop)):
            return source_path, index
        rows, last = menu_wait_events(lines, pcs, ram, start, stop)
        samples = [j for j in range(start + 1, stop) if pcs[j] in ('C9C5', 'E3CA')]
        if len(samples) != len(rows):
            raise ValueError('boundary cursor.wait: ambiguous motion-sample ordering')
        data = source_path.read_bytes()
        if (len(data), data[10]) not in ((421, 146), (428, 148)):
            raise ValueError('boundary cursor.wait: unexpected target-session layout')
        motion = data[305]
        ordered = []
        for row, sample in zip(rows, samples):
            observed = ram(sample)[0x532]
            if observed != motion:
                ordered.append(f'M {observed}')
                motion = observed
                motion_samples += 1
            ordered.append(row)
        operations += 1
        events = output / f'{operations:03d}-events.txt'
        events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
        destination = output / f'{operations:03d}-events.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source_path),
                              '--advance-observed-nmi-sequence', str(events), str(index),
                              '--capture', str(destination)], capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary cursor.wait: {run.stderr.strip()}')
        indices = re.findall(r'final random index (\d+);', run.stdout)
        if indices != [str(ram(last)[0xe1])]:
            raise ValueError(f'boundary cursor.wait, random.index: native {indices}, original {ram(last)[0xe1]}')
        return destination, int(indices[0])

    for number, (start, end) in enumerate(zip(points, points[1:]), 1):
        first = next(i for i in range(start + 1, end) if pcs[i] == 'E406')
        source, random_index = events_between(source, random_index, start, first + 1)
        try:
            compare_target(source.read_bytes(), ram(first))
        except ValueError as error:
            raise ValueError(str(error).replace('target.initial', f'cursor.poll[{number}].input')) from error
        held, pressed = ram(first)[0x1c], ram(first)[0x1e]
        operations += 1
        destination = output / f'{operations:03d}-poll.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source),
                              '--poll-ordinary-human-pass-cursor', str(held), str(pressed), '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary cursor.poll[{number}]: {run.stderr.strip()}')
        moves += int('changed 1' in run.stdout)
        source, random_index = events_between(destination, random_index, first, end)
        try:
            compare_target(source.read_bytes(), ram(end))
        except ValueError as error:
            raise ValueError(str(error).replace('target.initial', f'cursor.poll[{number}].result')) from error
        if random_index != ram(end)[0xe1]:
            raise ValueError(f'boundary cursor.poll[{number}].result, random.index: native {random_index}, original {ram(end)[0xe1]}')
    if moves != args.expected_moves:
        raise ValueError(f'Expected {args.expected_moves} moves, observed {moves}')
    if args.confirm:
        last = points[-1]
        end = next(i for i in range(last + 1, len(pcs)) if pcs[i] in ('D80A', 'D83E', 'D84B'))
        if pcs[end] != 'D84B':
            raise ValueError(f'boundary target.confirmed: unexpected original continuation {pcs[end]}')
        first = next(i for i in range(last + 1, end) if pcs[i] == 'E406')
        source, random_index = events_between(source, random_index, last, first + 1)
        try:
            compare_target(source.read_bytes(), ram(first))
        except ValueError as error:
            raise ValueError(str(error).replace('target.initial', 'target.confirmation_input')) from error
        held, pressed = ram(first)[0x1c], ram(first)[0x1e]
        if held & 15 or pressed & 192 != 128 or any(pcs[i] == 'C9C5' for i in range(first + 1, end)):
            raise ValueError('boundary target.confirmed: unsupported input or interleaved event')
        operations += 1
        destination = output / f'{operations:03d}-confirmed.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source),
                              '--confirm-ordinary-human-pass-target', '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary target.confirmed: {run.stderr.strip()}')
        compare_confirmed_target(destination.read_bytes(), ram(end))
        if random_index != ram(end)[0xe1]:
            raise ValueError(f'boundary target.confirmed, random.index: native {random_index}, original {ram(end)[0xe1]}')
        source = destination
    print(f'Matched {args.expected_polls} consecutive captured target polls, {moves} moves and {motion_samples} explicit motion samples; '
          f'final random index {random_index}; checkpoint {source}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Captured target-poll replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

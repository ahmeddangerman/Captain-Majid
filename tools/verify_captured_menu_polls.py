#!/usr/bin/env python3
"""Replay captured menu polls from a private original trace and session.

Each native result feeds the next. Recorded random/travel events and logical
button edges are supplied; this is not a live NMI or presentation scheduler.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_observed_world_round import checkpoint, compare_checkpoints
from verify_player7_pass_end_to_end import prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--initial-index', type=int, required=True)
    parser.add_argument('--expected-polls', type=int, required=True)
    parser.add_argument('--expected-action', type=int, required=True)
    args = parser.parse_args()
    if not 0 <= args.initial_index <= 255 or args.expected_polls <= 0 or not 0 <= args.expected_action <= 255:
        raise ValueError('Initial index/action must be bytes and expected polls positive')
    lines, pcs, ram = trace_data(args.trace_directory)
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source, random_index = args.source.resolve(), args.initial_index
    previous, count, operation = -1, 0, 0

    def events_between(source_path, index, start, stop):
        nonlocal operation
        if not any(pcs[j] == 'C9C5' for j in range(start + 1, stop)):
            return source_path, index
        rows, last = menu_wait_events(lines, pcs, ram, start, stop)
        operation += 1
        events = output / f'{operation:03d}-events.txt'
        events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
        destination = output / f'{operation:03d}-nmi.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source_path),
                              '--advance-observed-nmi-sequence', str(events), str(index),
                              '--capture', str(destination)], capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary menu.poll[{count + 1}].events: {run.stderr.strip()}')
        indices = re.findall(r'final random index (\d+);', run.stdout)
        if indices != [str(ram(last)[0xe1])]:
            raise ValueError(f'boundary menu.poll[{count + 1}].events, random.index: native {indices}, original {ram(last)[0xe1]}')
        return destination, int(indices[0])

    confirmed = False
    for index, pc in enumerate(pcs):
        if pc != 'D5EA' or index <= previous:
            continue
        source, random_index = events_between(source, random_index, previous, index)
        name = f'menu.poll[{count + 1}]'
        try:
            compare_checkpoints(source.read_bytes(), checkpoint(ram(index), 18))
        except ValueError as error:
            raise ValueError(str(error).replace('boundary field_round', f'boundary {name}.input')) from error
        if random_index != ram(index)[0xe1]:
            raise ValueError(f'boundary {name}.input, random.index: native {random_index}, original {ram(index)[0xe1]}')
        pressed = ram(index)[0x1e]
        operation += 1
        destination = output / f'{operation:03d}-poll.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source), '--menu-input',
                              str(pressed), '--capture', str(destination)], capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary {name}: {run.stderr.strip()}')
        source = destination
        end = next(j for j in range(index + 1, len(pcs)) if pcs[j] in ('D5E5', 'D67C', 'D5EA'))
        if pcs[end] == 'D5EA':
            raise ValueError(f'boundary {name}: missing original post-poll boundary')
        source, random_index = events_between(source, random_index, index, end)
        phase = 19 if pcs[end] == 'D67C' else 18
        try:
            compare_checkpoints(source.read_bytes(), checkpoint(ram(end), phase))
        except ValueError as error:
            raise ValueError(str(error).replace('boundary field_round', f'boundary {name}.result')) from error
        if random_index != ram(end)[0xe1]:
            raise ValueError(f'boundary {name}.result, random.index: native {random_index}, original {ram(end)[0xe1]}')
        count += 1
        previous = end
        if phase == 19:
            confirmed = True
            break
    if not confirmed or count != args.expected_polls:
        raise ValueError(f'Expected {args.expected_polls} polls ending in confirmation; got {count}, confirmed={confirmed}')
    actual_action = source.read_bytes()[315]
    if actual_action != args.expected_action:
        raise ValueError(f'boundary menu.confirm, action.id: native {actual_action}, expected {args.expected_action}')
    print(f'Continuously matched {count} captured menu polls through confirmed action {actual_action}; '
          f'final random index {random_index}; checkpoint {source}; event inputs supplied.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Captured menu replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

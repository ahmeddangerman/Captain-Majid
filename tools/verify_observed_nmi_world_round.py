#!/usr/bin/env python3
"""Compare a field round using private captured NMI inputs and task events.

The native random kernel calculates random bytes; the trace supplies memory
samples, carry and event ordering. This does not run a frame or sound engine.
"""
import argparse
import re
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

from verify_observed_world_round import checkpoint, compare_checkpoints, read_trace
from verify_player7_pass_end_to_end import prepare_output


def collect_nmi_samples(events, entry, end, observed_rows):
    frames = defaultdict(dict)
    for index, pc, frame, ram, carry in events:
        if pc not in ('C9C5', 'C9E8', 'E3CA', 'E412', 'E48F'):
            continue
        if pc in frames[frame]:
            raise ValueError(f'frame {frame}: duplicate {pc} boundary')
        frames[frame][pc] = (index, ram, carry)
    ordered = sorted(frame for frame, values in frames.items() if 'E412' in values)
    if len(ordered) != 22 or len(frames) != 22 or len(observed_rows) != 22:
        raise ValueError('Expected exactly 22 NMI/poll frames')
    previous = entry
    rows = []
    for frame, observed in zip(ordered, observed_rows):
        values = frames[frame]
        if not all(pc in values for pc in ('C9C5', 'C9E8', 'E3CA', 'E412', 'E48F')):
            raise ValueError(f'frame {frame}: missing NMI or poll boundary')
        pre_i, pre, carry = values['C9C5']
        post_i, post, _ = values['C9E8']
        travel_i, travel, _ = values['E3CA']
        poll_i, _, _ = values['E412']
        tail_i, tail, _ = values['E48F']
        if not pre_i < post_i < travel_i < poll_i < tail_i:
            raise ValueError(f'frame {frame}: NMI, travel and poll are out of order')
        if previous[0xe1:0xe4] != pre[0xe1:0xe4] or post[0xe1:0xe4] != travel[0xe1:0xe4]:
            raise ValueError(f'frame {frame}: random state changes outside captured boundaries')
        if carry not in (0, 1):
            raise ValueError(f'frame {frame}: missing random carry')
        index = pre[0xe1]
        held, pressed, _, _, present, travel_held, low, high = observed
        rows.append((held, pressed, pre[0x300 + index], pre[0x700 + index],
                     pre[(0x780 + index) & 2047], carry, present, travel_held, low, high))
        previous = tail
    if previous[0xe1:0xe4] != end[0xe1:0xe4]:
        raise ValueError('Random state changes between the final poll and control boundary')
    return rows


def read_nmi_trace(directory):
    entry, end, observed = read_trace(directory)
    lines = (directory / 'instructions.txt').read_text().splitlines()
    parsed = [(i, line.split()[0], int(line.split()[-1][6:]), line) for i, line in enumerate(lines)]
    start = next(i for i, pc, _, _ in parsed if pc == 'E407')
    stop = next(i for i, pc, _, _ in parsed if i > start and pc == 'E145')
    events = []
    for i, pc, frame, line in parsed:
        if not start < i < stop or pc not in ('C9C5', 'C9E8', 'E3CA', 'E412', 'E48F'):
            continue
        ram = (directory / f'{i:03d}-{pc}.ram').read_bytes()
        if len(ram) != 2048:
            raise ValueError(f'boundary {pc}: RAM capture must be exactly 2048 bytes')
        flags = [piece[2:] for piece in line.split() if piece.startswith('P=')]
        if len(flags) != 1:
            raise ValueError(f'boundary {pc}: missing processor flags')
        events.append((i, pc, frame, ram, int(flags[0], 16) & 1))
    return entry, end, collect_nmi_samples(events, entry, end, observed)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    entry, original, samples = read_nmi_trace(args.trace_directory)
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source = output / 'entry.cmjsession'
    source.write_bytes(checkpoint(entry, 13))
    events = output / 'observed-nmi-world.txt'
    events.write_text('CMJWORLDNMI1\n' + ''.join(' '.join(map(str, row)) + '\n' for row in samples))
    destination = output / 'native.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source),
                          '--advance-observed-nmi-world-file', str(events), str(entry[0xe1]),
                          '--capture', str(destination)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(f'boundary field_round: native operation failed: {run.stderr.strip()}')
    compare_checkpoints(destination.read_bytes(), checkpoint(original, 9))
    indices = re.findall(r'final random index (\d+);', run.stdout)
    if len(indices) != 1 or int(indices[0]) != original[0xe1]:
        raise ValueError(f'boundary field_round, random.index: native {indices}, original {original[0xe1]}')
    print('Matched 322 original-derived bytes, canonical context and random index across 22 observed NMI/poll events.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Observed NMI field-round replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

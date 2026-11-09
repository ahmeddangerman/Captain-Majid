#!/usr/bin/env python3
"""Compare individually seeded control polls against private instrumented traces.

This supplies recorded NMI memory/carry and travel events; it does not simulate
frames, audio or a continuous match. The output directory must be private.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_observed_world_round import checkpoint, compare_checkpoints
from verify_player7_pass_end_to_end import prepare_output


def replay(directory, pack, exe, output):
    lines = (directory / 'instructions.txt').read_text().splitlines()
    pcs = [line.split()[0] for line in lines]

    def ram(index):
        data = (directory / f'{index:03d}-{pcs[index]}.ram').read_bytes()
        if len(data) != 2048:
            raise ValueError(f'boundary {pcs[index]}: RAM capture must be 2048 bytes')
        return data

    count = 0
    for index, pc in enumerate(pcs):
        if pc != 'E14A':
            continue
        start = next(j for j in range(index - 1, -1, -1) if pcs[j] == 'E145')
        end = next(j for j in range(index + 1, len(pcs)) if pcs[j] in ('E145', 'E1E7'))
        group = {}
        for j in range(start + 1, index):
            if pcs[j] in ('C9C5', 'C9E8', 'E3CA', 'E3DE', 'E406'):
                if pcs[j] in group:
                    raise ValueError(f'poll {count + 1}: duplicate {pcs[j]} boundary')
                group[pcs[j]] = j
        if not all(k in group for k in ('C9C5', 'C9E8', 'E3CA', 'E406')):
            raise ValueError(f'poll {count + 1}: missing NMI or travel boundary')
        if not group['C9C5'] < group['C9E8'] < group['E3CA'] < group['E406'] < index:
            raise ValueError(f'poll {count + 1}: NMI/travel boundaries out of order')
        entry, nmi, post_random, travel, poll, original = map(
            ram, (start, group['C9C5'], group['C9E8'], group['E3CA'], index, end))
        if entry[0xe1:0xe4] != nmi[0xe1:0xe4] or post_random[0xe1:0xe4] != travel[0xe1:0xe4]:
            raise ValueError(f'poll {count + 1}: random state discontinuity')
        active = travel[0x5fb] != 0 or bool(travel[0x1c] & 15)
        if active != ('E3DE' in group):
            raise ValueError(f'poll {count + 1}: unexpected or missing speed boundary')
        speed = 0
        if active:
            loader = group['E3DE']
            if not group['E3CA'] < loader < group['E406']:
                raise ValueError(f'poll {count + 1}: speed boundary out of order')
            loaded = ram(loader)
            speed = loaded[0x32] | loaded[0x33] << 8
        random_index = nmi[0xe1]
        flags = re.search(r'\bP=([0-9A-Fa-f]{2})\b', lines[group['C9C5']])
        if flags is None:
            raise ValueError(f'poll {count + 1}: missing processor flags')
        carry = int(flags.group(1), 16) & 1
        row = (poll[0x1c], poll[0x1e], nmi[0x300 + random_index], nmi[0x700 + random_index],
               nmi[(0x780 + random_index) & 2047], carry, 1, travel[0x1c], speed & 255, speed >> 8)
        source = output / f'{count + 1:03d}-entry.cmjsession'
        source.write_bytes(checkpoint(entry, 9))
        events = output / f'{count + 1:03d}-events.txt'
        events.write_text('CMJPOLLNMI1\n' + ' '.join(map(str, row)) + '\n')
        destination = output / f'{count + 1:03d}-native.cmjsession'
        run = subprocess.run([str(exe.resolve()), str(pack.resolve()), str(source),
                              '--advance-observed-nmi-poll', str(events), str(entry[0xe1]),
                              '--capture', str(destination)], capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'poll {count + 1}: native operation failed: {run.stderr.strip()}')
        try:
            compare_checkpoints(destination.read_bytes(), checkpoint(original, 10 if pcs[end] == 'E1E7' else 9))
        except ValueError as error:
            raise ValueError(str(error).replace('boundary field_round', f'boundary poll[{count + 1}]')) from error
        indices = re.findall(r'final random index (\d+);', run.stdout)
        if indices != [str(original[0xe1])]:
            raise ValueError(f'boundary poll[{count + 1}], random.index: native {indices}, original {original[0xe1]}')
        count += 1
    if not count:
        raise ValueError('No observed control polls found')
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-polls', type=int, required=True)
    args = parser.parse_args()
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    count = replay(args.trace_directory, args.pack, args.exe, output)
    if count != args.expected_polls:
        raise ValueError(f'Expected {args.expected_polls} polls, found {count}')
    print(f'{count} independently seeded observed NMI/control polls matched 322 original-derived bytes, canonical context and random index.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Observed NMI control-poll replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

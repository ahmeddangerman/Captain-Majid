#!/usr/bin/env python3
"""Replay one captured control-to-encounter sequence with private evidence.

Every native boundary feeds the next. The trace supplies random memory/carry,
travel speeds, input masks and the ordering of clock/energy milestones; no
frame, sound or presentation scheduler is simulated.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_observed_nmi_poll import replay as verify_polls
from verify_observed_nmi_world_round import read_nmi_trace
from verify_observed_world_round import checkpoint, compare_checkpoints
from verify_player7_pass_end_to_end import field_name, prepare_output


def trace_data(directory):
    lines = (directory / 'instructions.txt').read_text().splitlines()
    pcs = [line.split()[0] for line in lines]

    def ram(index):
        data = (directory / f'{index:03d}-{pcs[index]}.ram').read_bytes()
        if len(data) != 2048:
            raise ValueError(f'boundary {pcs[index]}: expected 2048-byte RAM capture')
        return data
    return lines, pcs, ram


def clock_events(lines, pcs, ram, start, end):
    """Locate observed schedule and stamina writes among captured NMI events."""
    schedule = next(i for i in range(start, end) if pcs[i] == 'E1F7')
    tail = next(i for i in range(schedule, end) if pcs[i] == 'E1FA')
    relevant = [i for i in range(schedule, tail + 1)
                if pcs[i] in ('E1F7', 'E1FA', 'C9C5', 'C9E8', 'E3CA', 'E406')]
    energy = None
    for left, right in zip(relevant, relevant[1:]):
        before, after = checkpoint(ram(left), 10), checkpoint(ram(right), 10)
        changes = [field_name(j) for j in range(11, 361) if before[j] != after[j]]
        stamina = [name for name in changes if 'stamina' in name]
        if stamina:
            if energy is not None or len(stamina) != len(changes):
                raise ValueError(f'boundary clock.energy: ambiguous state change {changes}')
            energy = right
    if energy is None:
        raise ValueError('boundary clock.energy: no uniquely observed stamina boundary')
    rows = []
    for index in range(start + 1, end):
        pc = pcs[index]
        if index == schedule:
            rows.append('S')
        if index == energy:
            rows.append('E')
        if pc == 'C9C5':
            data = ram(index)
            random_index = data[0xe1]
            flags = re.search(r'\bP=([0-9A-Fa-f]{2})\b', lines[index])
            if flags is None:
                raise ValueError('boundary clock.random: missing carry')
            values = (data[0x300 + random_index], data[0x700 + random_index],
                      data[(0x780 + random_index) & 2047], int(flags.group(1), 16) & 1)
            rows.append('R ' + ' '.join(map(str, values)))
        elif pc == 'E3CA':
            data = ram(index)
            active = data[0x5fb] != 0 or bool(data[0x1c] & 15)
            stop = next(i for i in range(index + 1, end) if pcs[i] == 'E406')
            loaders = [i for i in range(index + 1, stop) if pcs[i] == 'E3DE']
            if len(loaders) != int(active):
                raise ValueError('boundary clock.travel: unexpected speed-loader count')
            speed = 0
            if active:
                loaded = ram(loaders[0])
                speed = loaded[0x32] | loaded[0x33] << 8
            rows.append(f'T {data[0x1c]} {speed & 255} {speed >> 8}')
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--expected-polls', type=int, required=True)
    parser.add_argument('--expected-rounds', type=int, required=True)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    poll_dir = output / 'independent-poll-checks'
    poll_dir.mkdir()
    count = verify_polls(args.trace_directory, args.pack, args.exe, poll_dir)
    if count != args.expected_polls:
        raise ValueError(f'Expected {args.expected_polls} observed polls, found {count}')
    first = pcs.index('E14A')
    start = next(i for i in range(first - 1, -1, -1) if pcs[i] == 'E145')
    source = output / '000-entry.cmjsession'
    source.write_bytes(checkpoint(ram(start), 9))
    random_index = ram(start)[0xe1]
    counts = {'poll': 0, 'clock': 0, 'world': 0}
    steps = 0
    ended = False
    for index in range(first, len(pcs)):
        pc = pcs[index]
        if pc not in ('E14A', 'E1E7', 'E407'):
            continue
        if pc == 'E14A':
            kind = 'poll'
            counts[kind] += 1
            events = poll_dir / f'{counts[kind]:03d}-events.txt'
            end = next(i for i in range(index + 1, len(pcs)) if pcs[i] in ('E145', 'E1E7'))
            phase = 10 if pcs[end] == 'E1E7' else 9
            entry = next(i for i in range(index - 1, -1, -1) if pcs[i] == 'E145')
            option = '--advance-observed-nmi-poll'
        elif pc == 'E1E7':
            kind = 'clock'
            counts[kind] += 1
            end = next(i for i in range(index + 1, len(pcs)) if pcs[i] == 'E407')
            rows = clock_events(lines, pcs, ram, index, end)
            events = output / f'clock-{counts[kind]:03d}.txt'
            events.write_text('CMJCLOCKNMI1\n' + '\n'.join(rows) + '\n')
            phase, entry, option = 13, index, '--advance-observed-nmi-clock'
        else:
            kind = 'world'
            counts[kind] += 1
            _, _, rows, boundary = read_nmi_trace(args.trace_directory, counts[kind])
            end = next(i for i in range(index + 1, len(pcs)) if pcs[i] in ('E145', 'E4BE', 'E38A'))
            if pcs[end] != boundary:
                raise ValueError(f'boundary world[{counts[kind]}]: inconsistent end boundary')
            events = output / f'world-{counts[kind]:03d}.txt'
            events.write_text('CMJWORLDNMI1\n' + ''.join(' '.join(map(str, row)) + '\n' for row in rows))
            phase, entry, option = (9 if boundary == 'E145' else 15), index, '--advance-observed-nmi-world-file'
        name = f'{kind}[{counts[kind]}]'
        if random_index != ram(entry)[0xe1]:
            raise ValueError(f'boundary {name}, random.index: native {random_index}, original {ram(entry)[0xe1]}')
        steps += 1
        destination = output / f'{steps:03d}-{kind}.cmjsession'
        run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source), option,
                              str(events), str(random_index), '--capture', str(destination)],
                             capture_output=True, text=True, timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary {name}: native operation failed: {run.stderr.strip()}')
        try:
            compare_checkpoints(destination.read_bytes(), checkpoint(ram(end), phase))
        except ValueError as error:
            raise ValueError(str(error).replace('boundary field_round', f'boundary {name}')) from error
        indices = re.findall(r'final random index (\d+);', run.stdout)
        if len(indices) != 1 or int(indices[0]) != ram(end)[0xe1]:
            raise ValueError(f'boundary {name}, random.index: native {indices}, original {ram(end)[0xe1]}')
        random_index = int(indices[0])
        source = destination
        if kind == 'world' and pcs[end] == 'E4BE':
            ended = True
            break
    if not ended or counts['poll'] != args.expected_polls or counts['world'] != args.expected_rounds or counts['clock'] != args.expected_rounds:
        raise ValueError(f'Incomplete control-to-encounter chain: {counts}, reached encounter={ended}')
    print(f'Continuously matched {steps} boundaries ({counts}) through E4BE with captured event inputs; no frame scheduler simulated.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Captured control-chain replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

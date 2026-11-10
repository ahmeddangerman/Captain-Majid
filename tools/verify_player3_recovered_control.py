#!/usr/bin/env python3
"""Replay the captured blocked-shot recovery through player3 control setup."""
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
    start, last_wait, setup = 11449, 12002, 12003
    if [pcs[i] for i in (start, last_wait, setup)] != ['8C31', 'E406', 'C9C5']:
        raise ValueError('Unexpected player3 recovery control boundaries')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source = args.source.resolve()

    def compare(path, at, phase, label):
        actual = path.read_bytes()
        if phase > 255:
            if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != phase:
                raise ValueError(f'{label}: unexpected extended session')
            actual = bytearray(actual[:259] + actual[260:])
            actual[10], actual[258] = 133, 20
            phase = 20
        compare_checkpoint(actual, ram(at), phase, phase, 'player3', label, None, False, False)

    def run(option, destination, *values):
        nonlocal source
        result = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source), option,
                                 *map(str, values), '--capture', str(destination)],
                                capture_output=True, text=True, timeout=30)
        if result.returncode:
            raise RuntimeError(result.stderr.strip())
        source = destination
        return result.stdout

    compare(source, start, 426, 'recovered owner')
    rows, last = menu_wait_events(lines, pcs, ram, start, setup)
    points = [i for i in range(start + 1, setup) if pcs[i] in ('C9C5', 'E3CA')]
    if last != last_wait or len(rows) != len(points):
        raise ValueError('Ambiguous player3 recovery wait ordering')
    facing, motion, samples, ordered = 0, 0, [], []
    for row, point in zip(rows, points):
        observed = ram(point)
        for token, address, previous in (('F', 0x517, facing), ('M', 0x532, motion)):
            value = observed[address]
            if value != previous:
                samples.append((token, value)); ordered.append(f'{token} {value}')
                if token == 'F': facing = value
                else: motion = value
        ordered.append(row)
    if samples != [('F', 64), ('M', 1), ('M', 0)]:
        raise ValueError(f'Unexpected captured player3 presentation: {samples}')
    events = output / 'recovery-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    result = run('--advance-observed-nmi-sequence', output / 'recovery-wait.cmjsession', events, ram(start)[0xe1])
    if re.findall(r'final random index (\d+);', result) != [str(ram(last)[0xe1])]:
        raise ValueError('Recovery wait random index differs')
    compare(source, last, 426, 'pre-setup wait')
    run('--setup-control', output / 'control-ready.cmjsession')
    compare(source, setup, 9, 'control setup')
    rows, last = menu_wait_events(lines, pcs, ram, setup - 1, 12011)
    events = output / 'control-setup-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    result = run('--advance-observed-nmi-sequence', output / 'control-poll-entry.cmjsession', events, ram(setup)[0xe1])
    if pcs[12011] != 'E145' or re.findall(r'final random index (\d+);', result) != [str(ram(last)[0xe1])]:
        raise ValueError('Control setup wait random index differs')
    compare(source, 12011, 9, 'first control poll entry')
    previous, count, index = 12011, 0, ram(12011)[0xe1]
    for at in range(12012, len(pcs)):
        if pcs[at] != 'E14A' or at <= previous:
            continue
        end = next(i for i in range(at + 1, len(pcs)) if pcs[i] in ('E145', 'E1E7'))
        relevant = {pcs[i]: i for i in range(previous + 1, at) if pcs[i] in ('C9C5', 'C9E8', 'E3CA', 'E3DE', 'E406')}
        if not all(pc in relevant for pc in ('C9C5', 'C9E8', 'E3CA', 'E406')):
            raise ValueError('Missing control poll NMI/travel boundary')
        nmi, travel, poll = ram(relevant['C9C5']), ram(relevant['E3CA']), ram(at)
        active = travel[0x5fb] != 0 or bool(travel[0x1c] & 15)
        if active != ('E3DE' in relevant):
            raise ValueError('Unexpected control poll speed boundary')
        speed = (ram(relevant['E3DE'])[0x32] | ram(relevant['E3DE'])[0x33] << 8) if active else 0
        flags = re.search(r'\bP=([0-9A-Fa-f]{2})\b', lines[relevant['C9C5']])
        if flags is None or nmi[0xe1] != index:
            raise ValueError('Control poll random input differs')
        row = (poll[0x1c], poll[0x1e], nmi[0x300 + index], nmi[0x700 + index],
               nmi[(0x780 + index) & 2047], int(flags.group(1), 16) & 1, 1, travel[0x1c], speed & 255, speed >> 8)
        events = output / f'poll-{count + 1:02d}-events.txt'
        events.write_text('CMJPOLLNMI1\n' + ' '.join(map(str, row)) + '\n')
        result = run('--advance-observed-nmi-poll', output / f'poll-{count + 1:02d}.cmjsession', events, index)
        if re.findall(r'final random index (\d+);', result) != [str(ram(end)[0xe1])]:
            raise ValueError(f'Control poll {count + 1} random index differs')
        index = ram(end)[0xe1]
        compare(source, end, 10 if pcs[end] == 'E1E7' else 9, f'control poll {count + 1}')
        previous, count = end, count + 1
        if pcs[end] == 'E1E7':
            break
    if count != 11:
        raise ValueError(f'Expected 11 captured polls, got {count}')
    print(f'Matched player3 recovery, control setup and {count} control polls; random index {index}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player3 recovered control replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

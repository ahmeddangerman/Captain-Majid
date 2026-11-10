#!/usr/bin/env python3
"""Replay captured blocked-shot or one-two recovery through resumed control."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import clock_events, menu_wait_events, trace_data
from verify_observed_nmi_world_round import read_nmi_trace
from verify_player7_pass_end_to_end import compare_checkpoint, prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--scenario', choices=('player3', 'player2'), default='player3')
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    start, last_wait, setup, first = ((11449,12002,12003,12011) if args.scenario=='player3' else (17374,17946,17947,17956))
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
            actual[10], actual[258] = 133, (20 if args.scenario=='player3' else 19)
            phase = 20 if args.scenario=='player3' else 19
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

    recovery_phase = 426 if args.scenario=='player3' else 429
    compare(source, start, recovery_phase, 'recovered owner')
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
    compare(source, last, recovery_phase, 'pre-setup wait')
    run('--setup-control', output / 'control-ready.cmjsession')
    compare(source, setup, 9, 'control setup')
    rows, last = menu_wait_events(lines, pcs, ram, setup - 1, first)
    events = output / 'control-setup-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    result = run('--advance-observed-nmi-sequence', output / 'control-poll-entry.cmjsession', events, ram(setup)[0xe1])
    if pcs[first] != 'E145' or re.findall(r'final random index (\d+);', result) != [str(ram(last)[0xe1])]:
        raise ValueError('Control setup wait random index differs')
    compare(source, first, 9, 'first control poll entry')
    def poll_cycle(previous, index, cycle):
      count = 0
      for at in range(previous + 1, len(pcs)):
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
        events = output / f'cycle-{cycle:02d}-poll-{count + 1:02d}-events.txt'
        events.write_text('CMJPOLLNMI1\n' + ' '.join(map(str, row)) + '\n')
        result = run('--advance-observed-nmi-poll', output / f'cycle-{cycle:02d}-poll-{count + 1:02d}.cmjsession', events, index)
        if re.findall(r'final random index (\d+);', result) != [str(ram(end)[0xe1])]:
            raise ValueError(f'Control poll {count + 1} random index differs')
        index = ram(end)[0xe1]
        compare(source, end, 10 if pcs[end] == 'E1E7' else 9, f'control poll {count + 1}')
        previous, count = end, count + 1
        if pcs[end] == 'E1E7':
            break
      if count != 11:
        raise ValueError(f'Expected 11 captured polls, got {count}')
      return previous, index

    previous, index = first, ram(first)[0xe1]
    cycles = (((9,12103,12276),(10,12369,12503),(11,12595,12729),(12,12822,12956),(13,13048,13182)) if args.scenario=='player3'
              else ((14,18048,18220),(15,18312,18447),(16,18540,18674),(17,18766,18900),(18,18993,19127)))
    for cycle, (round_number, clock_end, world_end) in enumerate(cycles, 1):
        previous, index = poll_cycle(previous, index, cycle)
        if pcs[previous] != 'E1E7' or pcs[clock_end] != 'E407':
            raise ValueError(f'Unexpected player3 clock boundary in cycle {cycle}')
        rows = clock_events(lines, pcs, ram, previous, clock_end)
        events = output / f'cycle-{cycle:02d}-clock-events.txt'
        events.write_text('CMJCLOCKNMI1\n' + '\n'.join(rows) + '\n')
        result = run('--advance-observed-nmi-clock', output / f'cycle-{cycle:02d}-clock.cmjsession', events, index)
        if re.findall(r'final random index (\d+);', result) != [str(ram(clock_end)[0xe1])]:
            raise ValueError(f'Player3 cycle {cycle} clock random index differs')
        compare(source, clock_end, 13, f'control clock {cycle}')
        entry, end, rows, boundary = read_nmi_trace(args.trace_directory, round_number)
        if boundary != ('E4BE' if cycle == 5 else 'E145') or entry != ram(clock_end) or end != ram(world_end):
            raise ValueError(f'Unexpected player3 field-round boundaries in cycle {cycle}')
        events = output / f'cycle-{cycle:02d}-field-events.txt'
        events.write_text('CMJWORLDNMI1\n' + ''.join(' '.join(map(str, row)) + '\n' for row in rows))
        result = run('--advance-observed-nmi-world-file', output / f'cycle-{cycle:02d}-field.cmjsession', events, entry[0xe1])
        if re.findall(r'final random index (\d+);', result) != [str(end[0xe1])]:
            raise ValueError(f'Player3 cycle {cycle} field-round random index differs')
        compare(source, world_end, 15 if boundary == 'E4BE' else 9, f'field round {cycle}')
        previous, index = world_end, end[0xe1]
    print(f'Matched {args.scenario} recovery and five control/clock/field cycles to the next encounter; random index {index}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player3 recovered control replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

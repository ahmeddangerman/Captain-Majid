#!/usr/bin/env python3
"""Replay the captured player7 terminal-defense presentation wait."""
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
    start, end, contact = 10421, 11204, 11205
    if [pcs[i] for i in (start, end, contact)] != ['80D9', 'E406', '81BC']:
        raise ValueError('Unexpected captured terminal defense boundaries')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])

    def compare(path, at, label):
        actual = path.read_bytes()
        if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 422:
            raise ValueError(f'{label}: unexpected native session format')
        if actual[408:413] != bytes((4, 1, 80, 2, 0)) or actual[414] != 2:
            raise ValueError(f'{label}: defender result changed')
        normalized = bytearray(actual[:259] + actual[260:])
        normalized[10], normalized[258] = 133, 20
        compare_checkpoint(normalized, ram(at), 20, 20, 'player7', label, None, False, False)

    compare(args.source, start, 'terminal result')
    rows, last = menu_wait_events(lines, pcs, ram, start, end + 1)
    points = [i for i in range(start + 1, end + 1) if pcs[i] in ('C9C5', 'E3CA')]
    if last != end or len(rows) != len(points):
        raise ValueError('Ambiguous original presentation event order')
    facing, motion, samples, ordered = 0, 2, [], []
    for row, point in zip(rows, points):
        observed = ram(point)
        for token, address, previous in (('F', 0x517, facing), ('M', 0x532, motion)):
            value = observed[address]
            if value != previous:
                samples.append((token, value))
                ordered.append(f'{token} {value}')
                if token == 'F': facing = value
                else: motion = value
        ordered.append(row)
    if samples != [('F', 64), ('M', 0), ('F', 0)]:
        raise ValueError(f'Unexpected original presentation samples: {samples}')
    events = output / 'terminal-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    destination = output / 'terminal-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(args.source.resolve()),
                          '--advance-observed-nmi-sequence', str(events), str(ram(start)[0xe1]),
                          '--capture', str(destination)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);', run.stdout) != [str(ram(end)[0xe1])]:
        raise ValueError('Terminal wait random index differs')
    compare(destination, end, 'terminal presentation wait')
    clocked = output / 'terminal-contact.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(destination),
                          '--advance-player7-shot-contact', '--capture', str(clocked)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    actual = clocked.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 423:
        raise ValueError('Unexpected terminal contact snapshot')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(11219), 20, 20, 'player7', 'terminal contact clock', None, False, False)
    if ram(11219)[0xe1] != ram(end)[0xe1]:
        raise ValueError('Unexpected random change across terminal contact')
    rows, last = menu_wait_events(lines, pcs, ram, 11218, 11258)
    if last != 11257:
        raise ValueError('Unexpected original loose-ball wait boundary')
    events = output / 'loose-ball-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    waiting = output / 'loose-ball-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(clocked),
                          '--advance-observed-nmi-sequence', str(events), str(ram(11219)[0xe1]),
                          '--capture', str(waiting)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);', run.stdout) != [str(ram(last)[0xe1])]:
        raise ValueError('Loose-ball wait random index differs')
    actual = waiting.read_bytes()
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(last), 20, 20, 'player7', 'loose-ball wait', None, False, False)
    jittered = output / 'loose-ball-jittered.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(waiting),
                          '--jitter-player7-shot-ball', '--capture', str(jittered)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    actual = jittered.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 424:
        raise ValueError('Unexpected jittered loose-ball snapshot')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(11259), 20, 20, 'player7', 'loose-ball jitter', None, False, False)
    if pcs[11448] != '8C10':
        raise ValueError('Unexpected original field movement end')
    rows, last = menu_wait_events(lines, pcs, ram, 11259, 11448)
    if len(rows) != 44:
        raise ValueError('Expected 22 captured random/travel pairs')
    events = output / 'loose-ball-movement-events.txt'
    events.write_text('CMJNMISEQ1\n' + ''.join(rows[2 * slot] + '\n' + rows[2 * slot + 1] + '\nP ' + str(slot) + '\n' for slot in range(22)))
    moved = output / 'loose-ball-moved.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(jittered),
                          '--advance-observed-field-movement', str(events), str(ram(11259)[0xe1]),
                          '--capture', str(moved)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    actual = moved.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 425:
        raise ValueError('Unexpected loose-ball field movement snapshot')
    if re.findall(r'final random index (\d+);', run.stdout) != [str(ram(11448)[0xe1])]:
        raise ValueError('Loose-ball movement random index differs')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(11448), 20, 20, 'player7', 'loose-ball field movement', None, False, False)
    recovered = output / 'loose-ball-recovered.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(moved),
                          '--recover-player7-shot-ball', '--capture', str(recovered)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    actual = recovered.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 426:
        raise ValueError('Unexpected recovered loose-ball snapshot')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(11449), 20, 20, 'player7', 'loose-ball owner', None, False, False)
    print(f'Matched terminal contact, loose-ball movement and player3 recovery; random index {ram(11448)[0xe1]}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player7 terminal replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

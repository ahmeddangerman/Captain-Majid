#!/usr/bin/env python3
"""Compare the captured one-two presentation and defender-selection wait."""
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
    start, final = 14986, 15288
    if [pcs[i] for i in (start, final)] != ['D67C', '804C']:
        raise ValueError('Unexpected one-two confirmation or defender handoff')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    compare_checkpoint(args.source.read_bytes(), ram(start), 19, 19, 'player3', 'one-two confirmation', None, False, False)
    rows, last = menu_wait_events(lines, pcs, ram, start, final)
    points = [i for i in range(start + 1, final) if pcs[i] in ('C9C5', 'E3CA')]
    if last != 15287 or len(points) != len(rows):
        raise ValueError('Ambiguous one-two presentation ordering')
    addresses = (('G', 0x443), ('M', 0x532), ('B', 0x62d), ('O', 0x442))
    previous = {token: ram(start)[address] for token, address in addresses}
    samples, ordered = [], []
    for row, point in zip(rows, points):
        observed = ram(point)
        for token, address in addresses:
            value = observed[address]
            if value != previous[token]:
                previous[token] = value; samples.append((token, value)); ordered.append(f'{token} {value}')
        ordered.append(row)
    for token, address in addresses:
        value = ram(final)[address]
        if value != previous[token]:
            previous[token] = value; samples.append((token, value)); ordered.append(f'{token} {value}')
    expected = [('G',3),('M',23),('M',0),('B',130),('M',1),('B',0),('O',16),('O',19),('O',20)]
    if samples != expected:
        raise ValueError(f'Unexpected captured one-two samples: {samples}')
    events = output / 'one-two-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    destination = output / 'one-two-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(args.source.resolve()),
                          '--advance-observed-nmi-sequence', str(events), str(ram(start)[0xe1]),
                          '--capture', str(destination)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);', run.stdout) != [str(ram(final)[0xe1])]:
        raise ValueError('One-two presentation random index differs')
    compare_checkpoint(destination.read_bytes(), ram(final), 19, 19, 'player3', 'defender handoff', None, False, False)
    choices = tuple(ram(final)[0x60b:0x60e])
    if choices != (1, 2, 2) or tuple(ram(final)[0x606:0x609]) != (0, 0, 0):
        raise ValueError('Unexpected original one-two defender choices')
    recorded = output / 'one-two-defenders.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(destination),
                          '--capture-player3-one-two-defenders', *map(str, choices), '--capture', str(recorded)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    def compare_choices(path, at, phase):
        actual = path.read_bytes()
        if len(actual) != 428 or actual[10] != 142 or actual[-10:] != bytes((*choices,0,0,0,0,0,0,0)):
            raise ValueError('One-two defender choices or session format differ')
        normalized = bytearray(actual[:418]); normalized[10] = 133
        original = bytearray(ram(at))
        if phase == 20:
            if normalized[315] != 3 or original[0x43b] != 3:
                raise ValueError('One-two action changed')
            normalized[315] = 0; original[0x43b] = 0
        compare_checkpoint(normalized, original, phase, phase, 'player3', 'one-two preparation', None, False, False)
    compare_choices(recorded, final, 19)
    prepared = output / 'one-two-prepared.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(recorded),
                          '--prepare-player3-one-two', '--capture', str(prepared)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if pcs[15290] != '804F' or ram(15290)[0xe1] != ram(final)[0xe1]:
        raise ValueError('Unexpected original one-two preparation boundary')
    compare_choices(prepared, 15290, 20)
    print(f'Matched one-two presentation, defender choices and attacker preparation; random index {ram(final)[0xe1]}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player3 one-two replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

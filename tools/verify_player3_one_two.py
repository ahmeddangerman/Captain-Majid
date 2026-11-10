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
    parser.add_argument('--comparison-content', nargs=7, type=int, required=True,
                        metavar=('RANGE','OFFSET','ELIGIBLE','T0','T1','T2','T3'))
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
    rows, last = menu_wait_events(lines, pcs, ram, 15290, 15450)
    points = [i for i in range(15291, 15450) if pcs[i] in ('C9C5', 'E3CA')]
    if last != 15448 or len(rows) != len(points):
        raise ValueError('Ambiguous first defender presentation ordering')
    addresses = (('L', 0x617), ('M', 0x532), ('F', 0x517))
    previous = {token: ram(15290)[address] for token, address in addresses}
    samples, ordered = [], []
    for row, point in zip(rows, points):
        observed = ram(point)
        for token, address in addresses:
            value = observed[address]
            if value != previous[token]:
                previous[token] = value; samples.append((token, value)); ordered.append(f'{token} {value}')
        ordered.append(row)
    if samples != [('L',0),('M',0),('F',64)]:
        raise ValueError(f'Unexpected first defender presentation samples: {samples}')
    events = output / 'first-defender-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    waiting = output / 'first-defender-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(prepared),
                          '--advance-observed-nmi-sequence', str(events), str(ram(15290)[0xe1]),
                          '--capture', str(waiting)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);', run.stdout) != [str(ram(last)[0xe1])]:
        raise ValueError('First defender wait random index differs')
    compare_choices(waiting, last, 20)
    defender = output / 'first-defender-prepared.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(waiting),
                          '--prepare-player3-one-two-defender', '--capture', str(defender)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    compare_choices(defender, 15450, 20)
    actual = defender.read_bytes(); observed = ram(15450)
    expected = bytes((1,0,1,0,observed[0x444],observed[0x32],observed[0x33],observed[0x43f],observed[0x440],0,0,0,0,0))
    if actual[398:412] != expected:
        raise ValueError('First defender preparation differs from original')
    compared = output / 'first-defender-compared.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(defender),
                          '--compare-player3-one-two-defender', *map(str,args.comparison_content), '--capture', str(compared)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    compare_choices(compared, 15453, 20)
    actual = compared.read_bytes()
    original = ram(15453)
    registers = re.search(r'\bA=([0-9A-F]{2}).*\bY=([0-9A-F]{2})', lines[15453])
    if registers is None:
        raise ValueError('Missing first defender original result registers')
    score, result = (int(value,16) for value in registers.groups())
    if actual[407:412] != bytes((original[0x71],original[0x619],score,result,0)) or actual[413] != result:
        raise ValueError('First defender comparison differs from original')
    rows, last = menu_wait_events(lines, pcs, ram, 15453, 15463)
    events = output / 'first-contact-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    waiting = output / 'first-contact-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(compared),
                          '--advance-observed-nmi-sequence', str(events), str(ram(15453)[0xe1]),
                          '--capture', str(waiting)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    if last != 15462 or re.findall(r'final random index (\d+);', run.stdout) != [str(ram(last)[0xe1])]:
        raise ValueError('First contact wait random index differs')
    compare_choices(waiting, last, 20)
    clocked = output / 'first-contact-clock.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(waiting),
                          '--advance-player3-one-two-contact', '--capture', str(clocked)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(run.stderr.strip())
    compare_choices(clocked, 15463, 20)
    print(f'Matched one-two first defender and contact clock; random index {ram(last)[0xe1]}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player3 one-two replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

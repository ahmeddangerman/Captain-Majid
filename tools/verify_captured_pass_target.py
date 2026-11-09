#!/usr/bin/env python3
"""Compare an acknowledged ordinary-pass submenu wait and target initialization.

A private original trace supplies NMI/travel events and the explicit ordinary
choice; the native source checkpoint must be the prior confirmed menu result.
The submenu's rendering and choice handling are acknowledged, not simulated.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_observed_world_round import checkpoint, compare_checkpoints
from verify_player7_pass_end_to_end import field_name, prepare_output


def compare_target(actual, raw):
    expected = checkpoint(raw, 19)
    if actual[:10] != b'CMJSESSION' or (len(actual), actual[10]) not in ((421, 146), (428, 148)):
        raise ValueError('boundary target.initial: invalid native session format or length')
    if int.from_bytes(actual[258:260], 'little') != 331:
        raise ValueError(f'boundary target.initial, session.phase: native {int.from_bytes(actual[258:260], "little")}, expected 331')
    for index in range(11, 413):
        if index == 258:
            continue
        native_index = index + (index > 258)
        if actual[native_index] != expected[index]:
            raise ValueError(f'boundary target.initial, {field_name(index)}: native {actual[native_index]}, original {expected[index]}')
    trailing = ((414, 'area_sequence_result', 255), (415, 'area_pass.present', 0),
                (416, 'area_pass.cost.low', 0), (417, 'area_pass.cost.high', 0),
                (418, 'ball.flight_flag', raw[0x61b]), (419, 'pass.cursor', raw[0x624]), (420, 'pass.steps', 0))
    for index, name, value in trailing:
        if actual[index] != value:
            raise ValueError(f'boundary target.initial, {name}: native {actual[index]}, original {value}')
    if actual[10] == 148:
        scanned = actual[421]
        if scanned not in (0, 1):
            raise ValueError(f'boundary target.initial, candidates.scanned: invalid value {scanned}')
        count, allies = actual[422:424]
        if not scanned:
            if any(actual[422:428]) or raw[0x625] != 0:
                raise ValueError('boundary target.initial, candidates: invalid unscanned state')
        else:
            wanted = (raw[0x430], raw[0x625], *(raw[0x431 + i] if i < raw[0x430] else 0 for i in range(4)))
            got = (count, allies, *actual[424:428])
            for name, native_value, original_value in zip(('count', 'allied_count', 'ids[0]', 'ids[1]', 'ids[2]', 'ids[3]'), got, wanted):
                if native_value != original_value:
                    raise ValueError(f'boundary target.initial, candidates.{name}: native {native_value}, original {original_value}')


def compare_confirmed_target(actual, raw):
    if len(actual) != 429 or actual[:10] != b'CMJSESSION' or actual[10] != 149:
        raise ValueError('boundary target.confirmed: invalid native session format or length')
    if int.from_bytes(actual[258:260], 'little') != 332:
        raise ValueError('boundary target.confirmed, session.phase: expected 332')
    prior = bytearray(actual[:-1])
    prior[10] = 148
    prior[258:260] = (331).to_bytes(2, 'little')
    try:
        compare_target(prior, raw)
    except ValueError as error:
        raise ValueError(str(error).replace('target.initial', 'target.confirmed')) from error
    if actual[-1] != raw[0x5fc]:
        raise ValueError(f'boundary target.confirmed, pass.recipient: native {actual[-1]}, original {raw[0x5fc]}')
    if actual[419] != raw[0x638]:
        raise ValueError(f'boundary target.confirmed, pass.destination: native {actual[419]}, original {raw[0x638]}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--initial-index', type=int, required=True)
    parser.add_argument('--expected-pairs', type=int, required=True)
    args = parser.parse_args()
    if not 0 <= args.initial_index <= 255 or args.expected_pairs <= 0:
        raise ValueError('Initial index must be a byte and expected pairs positive')
    lines, pcs, ram = trace_data(args.trace_directory)
    start = pcs.index('D67C')
    end = next(i for i in range(start + 1, len(pcs)) if pcs[i] == 'D80A')
    try:
        compare_checkpoints(args.source.read_bytes(), checkpoint(ram(start), 19))
    except ValueError as error:
        raise ValueError(str(error).replace('boundary field_round', 'boundary pass.confirmed')) from error
    if args.initial_index != ram(start)[0xe1]:
        raise ValueError(f'boundary pass.confirmed, random.index: native {args.initial_index}, original {ram(start)[0xe1]}')
    if ram(start)[0x43b] != 1 or ram(end)[0x43c] != 0:
        raise ValueError('Trace does not show the supported ordinary pass selection')
    rows, wait_end = menu_wait_events(lines, pcs, ram, start, end)
    if len(rows) != 2 * args.expected_pairs:
        raise ValueError(f'Expected {args.expected_pairs} captured pairs, found {len(rows) // 2}')
    if not any(pcs[i] == 'E3CA' and (ram(i)[0x1e] & 128) for i in range(start + 1, end)):
        raise ValueError('Trace lacks a captured ordinary submenu confirmation edge')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    events = output / 'submenu-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    waiting = output / 'submenu-wait.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(args.source.resolve()),
                          '--advance-observed-nmi-sequence', str(events), str(args.initial_index),
                          '--capture', str(waiting)], capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(f'boundary pass.submenu_wait: {run.stderr.strip()}')
    try:
        compare_checkpoints(waiting.read_bytes(), checkpoint(ram(wait_end), 19))
    except ValueError as error:
        raise ValueError(str(error).replace('boundary field_round', 'boundary pass.submenu_wait')) from error
    indices = re.findall(r'final random index (\d+);', run.stdout)
    if indices != [str(ram(wait_end)[0xe1])] or ram(wait_end)[0xe1] != ram(end)[0xe1]:
        raise ValueError(f'boundary pass.submenu_wait, random.index: native {indices}, original {ram(wait_end)[0xe1]}')
    destination = output / 'target.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(waiting),
                          '--begin-ordinary-human-pass-target', '--capture', str(destination)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(f'boundary target.initial: {run.stderr.strip()}')
    compare_target(destination.read_bytes(), ram(end))
    print(f'Matched {args.expected_pairs} captured random/travel pairs and acknowledged ordinary choice through target initialization; '
          f'final random index {indices[0]}; checkpoint {destination}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Captured ordinary pass target replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

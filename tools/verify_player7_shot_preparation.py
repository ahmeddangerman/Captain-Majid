#!/usr/bin/env python3
"""Compare a captured player7 defender choice and ordinary shot preparation."""
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
    start, choice, prepared = 9958, 10005, 10007
    if [pcs[i] for i in (start, choice, prepared)] != ['D67C', '804C', '804F']:
        raise ValueError('Unexpected original shot boundaries')
    if (ram(choice)[0x442], ram(choice)[0x60b], ram(choice)[0x606]) != (20, 2, 0):
        raise ValueError('Unexpected original defender choice')
    if (ram(choice)[0x26], ram(choice)[0x2b]) != (0, 3):
        raise ValueError('Expected the captured stage0/team3 context')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source = args.source.resolve()
    compare_checkpoint(source.read_bytes(), ram(start), 19, 19, 'player7', 'shot confirmation', None, False, False)
    rows, last = menu_wait_events(lines, pcs, ram, start, choice)
    events = output / 'events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')

    def run(option, destination, *values):
        nonlocal source
        process = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source), option,
                                  *map(str, values), '--capture', str(destination)],
                                 capture_output=True, text=True, timeout=30)
        if process.returncode:
            raise RuntimeError(f'{option}: {process.stderr.strip()}')
        source = destination
        return process.stdout

    result = run('--advance-observed-nmi-sequence', output / 'wait.cmjsession', events, ram(start)[0xe1])
    if re.findall(r'final random index (\d+);', result) != [str(ram(last)[0xe1])]:
        raise ValueError('Original random index differs before defender handoff')
    run('--record-ordinary-return-shot-defense', output / 'choice.cmjsession', 20, 2, 0)
    compare_checkpoint(source.read_bytes(), ram(choice), 19, 19, 'player7', 'defender choice', None, False, False)
    run('--prepare-basic-shot', output / 'prepared.cmjsession')
    compare_checkpoint(source.read_bytes(), ram(prepared), 20, 20, 'player7', 'shot preparation', None, False, False)
    last_wait = 10416
    if pcs[last_wait] != 'E406' or pcs[10418] != '80B6':
        raise ValueError('Unexpected original defender preparation boundaries')
    rows, last = menu_wait_events(lines, pcs, ram, prepared, last_wait + 1)
    points = [i for i in range(prepared + 1, last_wait + 1) if pcs[i] in ('C9C5', 'E3CA')]
    if last != last_wait or len(points) != len(rows):
        raise ValueError('Ambiguous original shot wait ordering')
    motion, samples, ordered = 0, [], []
    for row, point in zip(rows, points):
        observed = ram(point)[0x532]
        if motion != observed:
            motion = observed
            samples.append(motion)
            ordered.append(f'M {motion}')
        ordered.append(row)
    if samples != [11, 0, 2]:
        raise ValueError('Unexpected original shot presentation samples')
    events = output / 'shot-wait-events.txt'
    events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    result = run('--advance-observed-nmi-sequence', output / 'defender-wait.cmjsession', events, ram(prepared)[0xe1])
    if re.findall(r'final random index (\d+);', result) != [str(ram(last)[0xe1])]:
        raise ValueError('Original random index differs before defender preparation')
    compare_checkpoint(source.read_bytes(), ram(last), 20, 20, 'player7', 'defender wait', None, False, False)
    run('--prepare-ordinary-return-shot-defender', output / 'defender-prepared.cmjsession', ram(10418)[0x44e])
    actual = source.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 421:
        raise ValueError('Unexpected prepared defender session format')
    observed = ram(10418)
    wanted = bytes((1, 0, observed[0x43d], 0, observed[0x444], observed[0x32], observed[0x33],
                    observed[0x43f], observed[0x440], 0, 0, 0, 0, 0))
    if actual[399:413] != wanted:
        raise ValueError('Native defender skill, cost or action differs from the original')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, ram(10418), 20, 20, 'player7', 'defender preparation', None, False, False)
    run('--compare-player7-shot-defender', output / 'defender-compared.cmjsession')
    actual = source.read_bytes()
    if len(actual) != 419 or actual[10] != 143 or int.from_bytes(actual[258:260], 'little') != 422:
        raise ValueError('Unexpected compared defender session format')
    original = ram(10421)
    registers = re.search(r'\bA=([0-9A-F]{2}).*\bY=([0-9A-F]{2})', lines[10421])
    if registers is None:
        raise ValueError('Missing original defender result registers')
    score, result_code = (int(value, 16) for value in registers.groups())
    if (score, result_code) != (80, 2) or original[0xe1] != ram(last)[0xe1]:
        raise ValueError('Unexpected original defender result or random index')
    if actual[408:413] != bytes((original[0x71], original[0x619], score, result_code, 0)) or actual[414] != result_code:
        raise ValueError('Native defender comparison differs from the captured result')
    normalized = bytearray(actual[:259] + actual[260:])
    normalized[10], normalized[258] = 133, 20
    compare_checkpoint(normalized, original, 20, 20, 'player7', 'defender comparison', None, False, False)
    print(f'Matched player7 defender choice, shot and defender preparation; stamina {ram(prepared)[0x355] + 256 * ram(prepared)[0x356]}, '
          f'power {ram(prepared)[0x61c] + 256 * ram(prepared)[0x61d]}, defender strength {observed[0x32]}, '
          f'cost {observed[0x43f] + 256 * observed[0x440]}, result 2; final random index {ram(last)[0xe1]}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player7 shot preparation replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

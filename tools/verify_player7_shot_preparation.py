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
    print(f'Matched player7 defender choice and shot preparation; stamina {ram(prepared)[0x355] + 256 * ram(prepared)[0x356]}, '
          f'power {ram(prepared)[0x61c] + 256 * ram(prepared)[0x61d]}; final random index {ram(prepared)[0xe1]}.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, StopIteration, subprocess.SubprocessError) as error:
        print(f'Player7 shot preparation replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

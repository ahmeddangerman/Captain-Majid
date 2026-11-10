#!/usr/bin/env python3
"""Compare a same-cell numeric pass race with an explicitly captured original timeout."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from export_field_state import export_state
from export_phase_session import export_session
from verify_captured_control_chain import trace_data
from verify_player7_pass_end_to_end import compare_represented


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('probe', 'pack', 'session', 'race-table', 'trace-directory'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--session-exe', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    if [pcs[i] for i in (21282, 21283, 21284, 21300)] != ['DE64', 'E7CF', 'DE96', 'DED4']:
        raise ValueError('Unexpected original same-cell race boundaries')
    before, after = ram(21284), ram(21300)
    if (before[0xe1], after[0xe1]) != (70, 71):
        raise ValueError('Unexpected original random event ordering')
    result = subprocess.run([str(args.probe.resolve()), str(args.pack.resolve()), str(args.session.resolve()),
                             str(args.race_table.resolve())], capture_output=True, text=True, check=True)
    rows = result.stdout.splitlines()
    if len(rows) != 21:
        raise ValueError('Unexpected diagnostic output length')
    rounds, remaining, active, opposing, winner = map(int, rows[0].split())
    if (rounds, remaining, active, opposing, winner) != (2, after[0x430], after[0x431], after[0x432], -1):
        raise ValueError('Numeric contest does not match the original timeout')
    field = export_state(after)
    for slot, row in enumerate(rows[1:]):
        x, y, target, status = map(int, row.split())
        actual = bytes((x & 255, x >> 8, y & 255, y >> 8, target, status))
        if actual != field[111 + 6 * slot:117 + 6 * slot]:
            raise ValueError(f'Player motion slot {slot} differs from original timeout')
    if args.output.exists():
        raise ValueError('Output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(args.session.resolve()),
                    '--advance-player2-same-cell-pass-race', str(args.race_table.resolve()),
                    '--capture', str(args.output.resolve())], capture_output=True, text=True, check=True)
    saved = args.output.read_bytes()
    if int.from_bytes(saved[258:260], 'little') != 430:
        raise ValueError('Native contest did not save the first-round phase')
    first = ram(21298)
    compare_represented(saved, export_session(first), True, 'player2', 'race.first')
    if saved[333:337] != bytes((1, 1, 3, 20)):
        raise ValueError('Saved first-round contest context differs')
    rejected_path = args.output.with_name(args.output.stem + '-rejected.cmjsession')
    if rejected_path.exists():
        raise ValueError('Rejection output must not exist')
    rejected = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(args.output.resolve()),
                               '--advance-player2-same-cell-pass-race', str(args.race_table.resolve()),
                               '--capture', str(rejected_path.resolve())], capture_output=True, text=True)
    if rejected.returncode == 0 or rejected_path.exists() or args.output.read_bytes() != saved:
        raise ValueError('Early second round did not reject atomically')
    if saved[26:28] != bytes((before[0xe2], before[0xe3])):
        raise ValueError('Saved timeout must retain the pre-NMI random bytes')
    event_path = args.output.with_suffix('.events.txt')
    after_path = args.output.with_name(args.output.stem + '-nmi.cmjsession')
    if event_path.exists() or after_path.exists():
        raise ValueError('NMI evidence output must not exist')
    random_ram = ram(21298)
    index = random_ram[0xe1]
    flags = re.search(r'\bP=([0-9A-Fa-f]{2})\b', lines[21298])
    if flags is None or index != 70:
        raise ValueError('Missing captured contest random inputs')
    values = (random_ram[0x300 + index], random_ram[0x700 + index],
              random_ram[(0x780 + index) & 2047], int(flags.group(1), 16) & 1)
    event_path.write_text('CMJNMISEQ1\nR ' + ' '.join(map(str, values)) + '\n')
    command = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(args.output.resolve()),
                              '--advance-observed-nmi-sequence', str(event_path.resolve()), str(index),
                              '--capture', str(after_path.resolve())], capture_output=True, text=True, check=True)
    if re.findall(r'final random index (\d+);', command.stdout) != ['71']:
        raise ValueError('Contest random index differs')
    after_saved = after_path.read_bytes()
    compare_represented(after_saved, export_session(ram(21299)), True, 'player2', 'race.random')
    final_path = args.output.with_name(args.output.stem + '-timeout.cmjsession')
    if final_path.exists():
        raise ValueError('Timeout output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(after_path.resolve()),
                    '--advance-player2-same-cell-pass-race', str(args.race_table.resolve()),
                    '--capture', str(final_path.resolve())], capture_output=True, text=True, check=True)
    timeout = final_path.read_bytes()
    if int.from_bytes(timeout[258:260], 'little') != 431 or timeout[333:337] != bytes((1, 0, 3, 20)):
        raise ValueError('Saved timeout phase or contest context differs')
    compare_represented(timeout, export_session(after), True, 'player2', 'race.timeout')
    replay = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(final_path.resolve()),
                             '--advance-player2-same-cell-pass-race', str(args.race_table.resolve()),
                             '--capture', str(rejected_path.resolve())], capture_output=True, text=True)
    if replay.returncode == 0 or rejected_path.exists() or final_path.read_bytes() != timeout:
        raise ValueError('Repeated timeout did not reject atomically')
    roundtrip_path = args.output.with_name(args.output.stem + '-roundtrip.cmjsession')
    if roundtrip_path.exists():
        raise ValueError('Roundtrip output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(final_path.resolve()),
                    '--capture', str(roundtrip_path.resolve())], capture_output=True, text=True, check=True)
    if roundtrip_path.read_bytes() != timeout:
        raise ValueError('Timeout snapshot roundtrip differs')
    print('Matched first round, observed random event and timeout in original order; random index 70 to 71.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Pass race comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

#!/usr/bin/env python3
"""Compare a same-cell numeric pass race with an explicitly captured original timeout."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from export_field_state import export_state
from export_phase_session import export_session
from verify_captured_control_chain import trace_data, menu_wait_events
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
    wait_events = args.output.with_name(args.output.stem + '-wait.events.txt')
    wait_snapshot = args.output.with_name(args.output.stem + '-wait.cmjsession')
    if wait_events.exists() or wait_snapshot.exists():
        raise ValueError('Wait evidence output must not exist')
    travel = ram(21301)
    if pcs[21301:21303] != ['E3CA', 'E406'] or travel[0x5fb] or travel[0x1c] & 15:
        raise ValueError('Unexpected first post-timeout travel event')
    rows, last = menu_wait_events(lines, pcs, ram, 21302, 21412)
    if len(rows) != 48 or last != 21411:
        raise ValueError('Unexpected timeout presentation wait')
    wait_events.write_text('CMJNMISEQ1\nT ' + str(travel[0x1c]) + ' 0 0\n' + '\n'.join(rows) + '\n')
    command = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(final_path.resolve()),
                              '--advance-observed-nmi-sequence', str(wait_events.resolve()), '71',
                              '--capture', str(wait_snapshot.resolve())], capture_output=True, text=True, check=True)
    if re.findall(r'final random index (\d+);', command.stdout) != ['95']:
        raise ValueError('Timeout presentation random index differs')
    compare_represented(wait_snapshot.read_bytes(), export_session(ram(21412)), True, 'player2', 'race.recovery_entry')
    movement_events = args.output.with_name(args.output.stem + '-recovery.events.txt')
    movement_snapshot = args.output.with_name(args.output.stem + '-recovery.cmjsession')
    if movement_events.exists() or movement_snapshot.exists():
        raise ValueError('Recovery evidence output must not exist')
    rows, last = menu_wait_events(lines, pcs, ram, 21412, 21578)
    if len(rows) != 44 or last != 21575:
        raise ValueError('Unexpected recovery field movement events')
    movement_events.write_text('CMJNMISEQ1\n' + ''.join(rows[2*slot] + '\n' + rows[2*slot+1] + '\nP ' + str(slot) + '\n' for slot in range(22)))
    command = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(wait_snapshot.resolve()),
                              '--advance-observed-pass-movement', str(movement_events.resolve()), '95',
                              '--capture', str(movement_snapshot.resolve())], capture_output=True, text=True, check=True)
    if re.findall(r'final random index (\d+);', command.stdout) != ['117']:
        raise ValueError('Recovery movement random index differs')
    movement = movement_snapshot.read_bytes()
    if int.from_bytes(movement[258:260], 'little') != 432:
        raise ValueError('Unexpected recovery field movement phase')
    compare_represented(movement, export_session(ram(21578)), True, 'player2', 'race.recovery_movement')
    owner_snapshot = args.output.with_name(args.output.stem + '-owner.cmjsession')
    if owner_snapshot.exists():
        raise ValueError('Owner output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(movement_snapshot.resolve()),
                    '--recover-player2-same-cell-pass', '--capture', str(owner_snapshot.resolve())],
                   capture_output=True, text=True, check=True)
    owner = owner_snapshot.read_bytes()
    if int.from_bytes(owner[258:260], 'little') != 433:
        raise ValueError('Unexpected recovered owner phase')
    compare_represented(owner, export_session(ram(21579)), True, 'player2', 'race.recovered_owner')
    if owner[413] != ram(21579)[0x616] or owner[418] != ram(21579)[0x61b]:
        raise ValueError('Recovered owner defender cursor or flight flag differs')
    replay = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(owner_snapshot.resolve()),
                             '--recover-player2-same-cell-pass', '--capture', str(rejected_path.resolve())],
                            capture_output=True, text=True)
    if replay.returncode == 0 or rejected_path.exists() or owner_snapshot.read_bytes() != owner:
        raise ValueError('Repeated owner recovery did not reject atomically')
    owner_roundtrip = args.output.with_name(args.output.stem + '-owner-roundtrip.cmjsession')
    if owner_roundtrip.exists():
        raise ValueError('Owner roundtrip output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(owner_snapshot.resolve()),
                    '--capture', str(owner_roundtrip.resolve())], capture_output=True, text=True, check=True)
    if owner_roundtrip.read_bytes() != owner:
        raise ValueError('Owner snapshot roundtrip differs')
    owner_events = args.output.with_name(args.output.stem + '-owner-wait.events.txt')
    owner_wait = args.output.with_name(args.output.stem + '-owner-wait.cmjsession')
    if owner_events.exists() or owner_wait.exists():
        raise ValueError('Owner presentation outputs must not exist')
    rows, last = menu_wait_events(lines, pcs, ram, 21579, 21889)
    if len(rows) != 106 or last != 21888:
        raise ValueError('Unexpected opponent recovery presentation events')
    owner_events.write_text('CMJNMISEQ1\n' + '\n'.join(rows) + '\n')
    command = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(owner_snapshot.resolve()),
                              '--advance-observed-nmi-sequence', str(owner_events.resolve()), '117',
                              '--capture', str(owner_wait.resolve())], capture_output=True, text=True, check=True)
    if re.findall(r'final random index (\d+);', command.stdout) != ['170']:
        raise ValueError('Opponent recovery wait random index differs')
    compare_represented(owner_wait.read_bytes(), export_session(ram(21889)), True, 'player2', 'race.control_entry')
    setup_events = args.output.with_name(args.output.stem + '-setup-wait.events.txt')
    setup_wait = args.output.with_name(args.output.stem + '-setup-wait.cmjsession')
    if setup_events.exists() or setup_wait.exists():
        raise ValueError('Control setup evidence outputs must not exist')
    rows, last = menu_wait_events(lines, pcs, ram, 21889, 22253)
    points = [i for i in range(21890, 22253) if pcs[i] in ('C9C5', 'E3CA')]
    if len(rows) != 124 or len(points) != len(rows) or last != 22252:
        raise ValueError('Unexpected opponent control presentation ordering')
    addresses = (('F', 0x517), ('M', 0x532))
    values = {key: ram(21889)[address] for key, address in addresses}
    samples = []; ordered = []
    for row, point in zip(rows, points):
        for key, address in addresses:
            value = ram(point)[address]
            if value != values[key]:
                values[key] = value; samples.append((key, value)); ordered.append(f'{key} {value}')
        ordered.append(row)
    if samples != [('F', 0), ('M', 1), ('M', 0)]:
        raise ValueError(f'Unexpected opponent control presentation samples: {samples}')
    setup_events.write_text('CMJNMISEQ1\n' + '\n'.join(ordered) + '\n')
    command = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(owner_wait.resolve()),
                              '--advance-observed-nmi-sequence', str(setup_events.resolve()), '170',
                              '--capture', str(setup_wait.resolve())], capture_output=True, text=True, check=True)
    if re.findall(r'final random index (\d+);', command.stdout) != ['232']:
        raise ValueError('Opponent control setup wait random index differs')
    compare_represented(setup_wait.read_bytes(), export_session(ram(22252)), True, 'player2', 'race.pre_control_setup')
    setup = args.output.with_name(args.output.stem + '-control-setup.cmjsession')
    if setup.exists():
        raise ValueError('Control setup output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(setup_wait.resolve()),
                    '--setup-control', '--capture', str(setup.resolve())], capture_output=True, text=True, check=True)
    prepared = setup.read_bytes()
    if int.from_bytes(prepared[258:260], 'little') != 434:
        raise ValueError('Unexpected opponent control setup phase')
    compare_represented(prepared, export_session(ram(22253)), True, 'player2', 'race.control_setup')
    if prepared[413] != ram(22253)[0x616] or prepared[418] != ram(22253)[0x61b]:
        raise ValueError('Prepared control cursor or flight flag differs')
    replay = subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(setup.resolve()),
                             '--setup-control', '--capture', str(rejected_path.resolve())], capture_output=True, text=True)
    if replay.returncode == 0 or rejected_path.exists() or setup.read_bytes() != prepared:
        raise ValueError('Repeated control setup did not reject atomically')
    setup_roundtrip = args.output.with_name(args.output.stem + '-setup-roundtrip.cmjsession')
    if setup_roundtrip.exists():
        raise ValueError('Setup roundtrip output must not exist')
    subprocess.run([str(args.session_exe.resolve()), str(args.pack.resolve()), str(setup.resolve()),
                    '--capture', str(setup_roundtrip.resolve())], capture_output=True, text=True, check=True)
    if setup_roundtrip.read_bytes() != prepared:
        raise ValueError('Control setup snapshot roundtrip differs')
    print('Matched opponent20 recovery, captured presentation and control setup; random index 232.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Pass race comparison stopped: {error}', file=sys.stderr)
        sys.exit(1)

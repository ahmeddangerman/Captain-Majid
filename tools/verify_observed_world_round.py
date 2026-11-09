#!/usr/bin/env python3
"""Compare one field round using private instrumented original-emulator traces.

The trace must record the original E407, E3CA/E3DE/E406, E412 and E145
instruction boundaries with frame IDs and one 2048-byte RAM file per event.
It supplies observed inputs; it does not simulate the original frame engine.
"""
import argparse
import subprocess
import sys
from collections import defaultdict
from pathlib import Path

from export_phase_session import export_session
from verify_player7_pass_end_to_end import field_name, prepare_output


def collect_samples(events):
    """Return the 22 pre-poll samples, rejecting ambiguous frame ordering."""
    frames = defaultdict(dict)
    for index, pc, frame, ram in events:
        if pc not in ('E3CA', 'E3DE', 'E406', 'E412'):
            continue
        if pc in frames[frame]:
            raise ValueError(f'frame {frame}: duplicate {pc} boundary')
        frames[frame][pc] = (index, ram)
    poll_frames = sorted(frame for frame, entries in frames.items() if 'E412' in entries)
    if len(poll_frames) != 22:
        raise ValueError(f'Expected 22 observed polls, found {len(poll_frames)}')
    if len(frames) != len(poll_frames):
        raise ValueError('Trace contains travel events outside a captured poll frame')
    rows = []
    for frame in poll_frames:
        entries = frames[frame]
        if not all(name in entries for name in ('E3CA', 'E406', 'E412')):
            raise ValueError(f'frame {frame}: missing travel or poll boundary')
        before_i, before = entries['E3CA']
        after_i, _ = entries['E406']
        poll_i, poll = entries['E412']
        if not before_i < after_i < poll_i:
            raise ValueError(f'frame {frame}: unexpected travel/poll ordering')
        if any(before[address] != poll[address] for address in (0x1c, 0xe2, 0xe3, 0x517)):
            raise ValueError(f'frame {frame}: input, random or facing changed before the poll')
        active = before[0x5fb] != 0 or bool(before[0x1c] & 15)
        if active != ('E3DE' in entries):
            raise ValueError(f'frame {frame}: missing or unexpected speed-loader boundary')
        speed = 0
        if active:
            speed_i, loaded = entries['E3DE']
            if not before_i < speed_i < after_i:
                raise ValueError(f'frame {frame}: speed-loader boundary is out of order')
            speed = loaded[0x32] | loaded[0x33] << 8
        rows.append((poll[0x1c], poll[0x1e], poll[0xe2], poll[0xe3], 1,
                     before[0x1c], speed & 255, speed >> 8))
    return rows


def read_trace(directory):
    parsed = []
    for index, line in enumerate((directory / 'instructions.txt').read_text().splitlines()):
        parts = line.split()
        if not parts or not parts[-1].startswith('FRAME='):
            raise ValueError(f'trace line {index + 1}: missing frame ID')
        parsed.append((index, parts[0], int(parts[-1][6:])))
    start = next((i for i, pc, _ in parsed if pc == 'E407'), None)
    if start is None:
        raise ValueError('trace has no field-round entry E407')
    end = next((i for i, pc, _ in parsed if i > start and pc == 'E145'), None)
    if end is None:
        raise ValueError('trace has no subsequent control-ready boundary E145')

    def read_ram(index, pc):
        data = (directory / f'{index:03d}-{pc}.ram').read_bytes()
        if len(data) != 2048:
            raise ValueError(f'boundary {pc}: RAM capture must be exactly 2048 bytes')
        return data

    event_list = [(i, pc, frame, read_ram(i, pc)) for i, pc, frame in parsed
                  if start < i < end and pc in ('E3CA', 'E3DE', 'E406', 'E412')]
    return read_ram(start, 'E407'), read_ram(end, 'E145'), collect_samples(event_list)


def checkpoint(ram, phase):
    if len(ram) != 2048 or ram[0x600] > 5:
        raise ValueError('Checkpoint needs 2048-byte RAM and at most five selected players')
    sanitized = bytearray(ram)
    sanitized[0x600:0x606] = bytes(6)
    data = bytearray(export_session(sanitized, 0))
    data[10] = 129
    data[258] = phase
    data += bytes(89)
    count = ram[0x600]
    data[28] = count
    data[309:314] = ram[0x601:0x601 + count] + bytes(5 - count)
    data[361:365] = bytes((1, ram[0x28], ram[0x29], ram[0x629]))
    data[365:368] = bytes((1, ram[0x308], ram[0x38c]))
    data[368:370] = bytes(2)
    return bytes(data)


def compare_checkpoints(actual, expected):
    if len(actual) != 414 or len(expected) != 413:
        raise ValueError('boundary field_round: unexpected checkpoint length')
    if actual[:10] != b'CMJSESSION' or expected[:10] != b'CMJSESSION':
        raise ValueError('boundary field_round: invalid session magic')
    if actual[10] != 130 or expected[10] != 129:
        raise ValueError('boundary field_round: unexpected session version')
    extra_names = {361: 'score.present', 362: 'score.left', 363: 'score.right', 364: 'score.kickoff',
                   365: 'keeper_positions.present', 366: 'keeper_positions.left_y', 367: 'keeper_positions.right_y',
                   368: 'kickoff_receiver.present', 369: 'kickoff_receiver.value', 258: 'session.phase'}
    for index in range(11, 413):
        if actual[index] != expected[index]:
            name = extra_names.get(index, field_name(index))
            raise ValueError(f'boundary field_round, {name}: native {actual[index]}, original {expected[index]}')
    if actual[413] != 255:
        raise ValueError(f'boundary field_round, area_sequence_result: native {actual[413]}, expected 255')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    entry, original, samples = read_trace(args.trace_directory)
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source = output / 'entry.cmjsession'
    source.write_bytes(checkpoint(entry, 13))
    events = output / 'observed-world.txt'
    events.write_text('CMJWORLDOBS1\n' + ''.join(' '.join(map(str, row)) + '\n' for row in samples))
    destination = output / 'native.cmjsession'
    run = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(source),
                          '--advance-observed-world-file', str(events), '--capture', str(destination)],
                         capture_output=True, text=True, timeout=30)
    if run.returncode:
        raise RuntimeError(f'boundary field_round: native operation failed: {run.stderr.strip()}')
    actual, expected = destination.read_bytes(), checkpoint(original, 9)
    compare_checkpoints(actual, expected)
    print('Matched 322 original-derived bytes, 80 canonical context bytes and the trailing context across 22 observed polls.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(f'Observed field-round replay stopped: {error}', file=sys.stderr)
        sys.exit(1)

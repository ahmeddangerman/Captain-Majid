from pathlib import Path
import argparse
import subprocess
import sys

from export_phase_session import export_session


def main():
    parser = argparse.ArgumentParser(description='Compare the sampled player7 pass with private original checkpoints.')
    parser.add_argument('--exe', required=True, type=Path)
    parser.add_argument('--pack', required=True, type=Path)
    parser.add_argument('--velocity', required=True, type=Path)
    parser.add_argument('--targets', required=True, type=Path)
    parser.add_argument('--flight-original', required=True, type=Path)
    parser.add_argument('--receive-original', required=True, type=Path)
    parser.add_argument('--control-original', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    if any(args.output.iterdir()):
        raise ValueError('output directory must be empty to protect existing evidence')
    count = 0
    for case in range(24):
        before = args.targets / f'{case}-target.cmjsession'
        operations = [('setup', ['--begin-sampled-field-pass-flight', str(args.velocity), '167', '221'],
                       args.flight_original / f'{case}-e773-1.bin', 96, 19, True)]
        for step in range(2, 6):
            original = args.flight_original / f'{case}-e773-{step}.bin'
            operations.extend([(f'step{step}', ['--advance-area-ball-cell'], original, 98, 19, True),
                               (f'callback{step}', ['--resolve-area-ball-cell'], original, 96, 19, True)])
        operations.extend([
            ('arrival', ['--advance-area-ball-cell'], args.flight_original / f'{case}-e7cf-1.bin', 99, 19, True),
            ('received', ['--continue-area-already-arrived'], args.receive_original / f'{case}-write-0441-1.bin', 100, 19, False),
            ('positioned', ['--continue-area-pass-received'], args.receive_original / f'{case}-write-0637-1.bin', 101, 19, False),
            ('control', ['--continue-sampled-field-pass-control'], args.receive_original / f'{case}-e0df-1.bin', 1, 19, False),
            ('ready', ['--setup-control'], args.control_original / f'{case}-ready.bin', 9, 9, False),
        ])
        for label, command, original, phase, export_phase, trajectory in operations:
            destination = args.output / f'{case}-{label}.cmjsession'
            result = subprocess.run([str(args.exe), str(args.pack), str(before), *command, '--capture', str(destination)],
                                    capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError(f'case {case} {label}: native operation failed: {result.stderr.strip()}')
            actual, raw = destination.read_bytes(), original.read_bytes()
            expected = export_session(raw, export_phase)
            for offset in range(11, 324):
                if offset != 258 and actual[offset] != expected[offset]:
                    raise RuntimeError(f'case {case} {label}: byte {offset}: native {actual[offset]}, original {expected[offset]}')
            if actual[258] != phase or actual[412] != raw[0x616] or actual[-1] != raw[0x61b]:
                raise RuntimeError(f'case {case} {label}: phase, cursor or flag mismatch')
            if trajectory:
                expected_trajectory = raw[0x634:0x638] + raw[0x639:0x63d] + bytes((raw[0x62c], raw[0x61b]))
                if actual[345:355] != expected_trajectory:
                    raise RuntimeError(f'case {case} {label}: trajectory mismatch')
            before = destination
            count += 1
    print(f'Matched {count} player7 pass boundaries across 24 cases.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, RuntimeError, IndexError) as exc:
        print(f'Player7 pass comparison failed: {exc}', file=sys.stderr)
        sys.exit(1)

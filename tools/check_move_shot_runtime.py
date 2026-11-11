#!/usr/bin/env python3
"""Drive provisional pass and shot paths after movement using private original control fixtures."""
import argparse
import subprocess
import tempfile
from pathlib import Path

from export_phase_session import export_session


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('ready-dir', 'pack', 'velocity', 'distance', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside the checkout')
    count = 0
    with tempfile.TemporaryDirectory(prefix='move-shot-', dir=args.private_temp) as temporary:
        snapshot, metadata = Path(temporary) / 'ready.cmjsession', Path(temporary) / 'ready.bin'
        for fixture in range(4):
            raw = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes())
            if len(raw) != 2048 or raw[0x5fb] != 0 or raw[0x600] or raw[0x43d] > 6:
                raise ValueError('Expected supported home control-ready RAM')
            raw[0x614] = 0  # Explicit intervention: exercise a movement decision immediately.
            owner = raw[0x441]
            raw[0x301 + 12 * owner:0x303 + 12 * owner] = (400).to_bytes(2, 'little')  # Known affordable shot fixture.
            snapshot.write_bytes(export_session(raw, 9));metadata.write_bytes(raw)
            for action in ('shot', 'pass'):
                result = subprocess.run([str(args.exe.resolve()), '--drive-control-' + action, str(snapshot), str(args.pack.resolve()),
                                         str(args.velocity.resolve()), str(args.distance.resolve()), str(metadata)], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'Fixture {fixture} {action} stopped: {result.stderr}')
                count += 1
    print(f'{count} constructed native sequences completed: four moved passes to resumed control and four moved shots to flight (provisional timing/random)')


if __name__ == '__main__':
    main()

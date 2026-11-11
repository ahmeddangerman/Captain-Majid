#!/usr/bin/env python3
"""Launch the provisional Windows match window from an explicit private control-ready RAM capture."""
import argparse
import subprocess
import tempfile
from pathlib import Path

from export_phase_session import export_session


def private_path(path, checkout, description):
    resolved = path.resolve()
    if resolved.is_relative_to(checkout):
        raise ValueError(f'{description} must remain outside the Git checkout')
    return resolved


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('ram', 'pack', 'distance', 'velocity', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    root = Path(__file__).resolve().parents[1]
    parser.add_argument('--exe', type=Path, default=root / 'build/captain-match-window.exe')
    parser.add_argument('--seed', type=int, default=1)
    parser.add_argument('--render-check', type=Path, help='Write an off-screen BMP instead of opening a window')
    args = parser.parse_args()
    try:
        if not 1 <= args.seed <= 0xffffffff:
            raise ValueError('Seed must be 1..4294967295')
        paths = {name: private_path(getattr(args, name.replace('-', '_')), root, name)
                 for name in ('ram', 'pack', 'distance', 'velocity', 'private-temp')}
        for name in ('ram', 'pack', 'distance', 'velocity'):
            if not paths[name].is_file():
                raise ValueError(f'{name} is not a readable file')
        if not paths['private-temp'].is_dir():
            raise ValueError('Private temporary directory does not exist')
        if not args.exe.is_file():
            raise ValueError('Match window is not built; run cmake --build build --target captain-match-window')
        raw = paths['ram'].read_bytes()
        if len(raw) != 2048 or raw[0x600] or raw[0x5fb] not in (0, 11) or raw[0x43d] > 6:
            raise ValueError('Expected a compatible control-ready RAM capture with no selected defenders and a known action byte')
        snapshot = export_session(raw, 9)
        output = private_path(args.render_check, root, 'Render check output') if args.render_check else None
        if output is not None and output.exists():
            raise ValueError('Render check output already exists')
        with tempfile.TemporaryDirectory(prefix='captain-window-', dir=paths['private-temp']) as temporary:
            session = Path(temporary) / 'control.cmjsession'
            session.write_bytes(snapshot)
            command = [str(args.exe.resolve()), str(paths['pack']), str(session), '--distance', str(paths['distance']),
                       '--velocity', str(paths['velocity']), '--seed', str(args.seed),
                       '--ball-fractions', str(raw[0x634]), str(raw[0x636]),
                       '--retained-defense-action', str(raw[0x43d]),
                       '--score', str(raw[0x28]), str(raw[0x29]), str(raw[0x629])]
            if output is not None:
                command.extend(('--render-check', str(output)))
            print('Starting provisional development view from the supplied control-ready capture.', flush=True)
            return subprocess.run(command, check=False).returncode
    except (OSError, ValueError) as error:
        parser.exit(1, f'Development launch stopped: {error}\n')


if __name__ == '__main__':
    raise SystemExit(main())

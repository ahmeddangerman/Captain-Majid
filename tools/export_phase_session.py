#!/usr/bin/env python3
"""Export an explicit private RAM checkpoint into a bounded CMJSESSION2 file."""
import argparse
import struct
from pathlib import Path
from export_field_state import export_state


def export_session(ram, phase=0, pending=None, include_experience=True):
    field = export_state(ram)
    if phase not in range(5) or (phase == 3) != (pending is not None):
        raise ValueError('Only added-time phase3 requires pending terminal data')
    if pending is not None:
        code, defender, action, subtype = pending
        if (code not in (2, 3) or defender not in (*range(1, 11), *range(12, 22))
                or action not in (0, 2) or subtype not in (0, 128)
                or (0 if defender < 11 else 11) == ram[0x5FB]):
            raise ValueError('Unsupported pending terminal contact')
    clock = bytes(ram[a] for a in (0x5F7, 0x5F8, 0x5F9, 0x63E, 0x615, 0x62D, 0x449, 0x44A))
    experience = bytes(24)
    if include_experience:
        campaign, stage = ram[0x2A], ram[0x26]
        if campaign >= 3 or stage >= 34:
            raise ValueError('Unsupported experience campaign/stage')
        experience = bytes((campaign, stage)) + ram[0x454:0x46A]
    return (b'CMJSESSION2' + field + clock
            + struct.pack('<BB4B', phase, int(pending is not None), *(pending or (0, 0, 0, 0)))
            + bytes((int(include_experience),)) + experience)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ram', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--phase', type=int, choices=range(5), default=0)
    parser.add_argument('--pending', type=int, nargs=4, metavar=('CODE', 'DEFENDER', 'ACTION', 'SUBTYPE'))
    parser.add_argument('--without-experience', action='store_true', help='Do not infer campaign/experience context')
    args = parser.parse_args()
    if args.output.suffix != '.cmjsession':
        parser.error('Use a private .cmjsession output path (ignored by Git)')
    try:
        data = export_session(args.ram.read_bytes(), args.phase, args.pending, not args.without_experience)
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, f'Session export stopped: {error}\n')
    print(f'Exported {len(data)} bytes; development phase session, not a complete match save.')


if __name__ == '__main__':
    main()

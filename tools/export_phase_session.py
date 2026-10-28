#!/usr/bin/env python3
"""Export an explicit private RAM checkpoint into a bounded CMJSESSION3 file."""
import argparse
import struct
from pathlib import Path
from export_field_state import export_state


def export_session(ram, phase=0, pending=None, include_experience=True, include_recovery=True):
    field = export_state(ram)
    if phase not in range(9) or (phase == 3) != (pending is not None):
        raise ValueError('Only added-time phase3 requires pending terminal data')
    if phase >= 5 and ram[0x600] != 0:
        raise ValueError('Recovery continuation requires zero selected defenders')
    if phase >= 6 and (ram[0x441] in (0, 11) or (0 if ram[0x441] < 11 else 11) != ram[0x5FB]):
        raise ValueError('Recovered owner must be a field player on the possession side')
    if phase >= 6 and not (48 <= ram[0x635] < 208 and 80 <= ram[0x637] < 176):
        raise ValueError('Recovered-owner continuation requires an interior ball')
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
    recovery = bytes(6)
    if include_recovery:
        if ram[0x442] > 21:
            raise ValueError('Unsupported recovery opposing player')
        recovery = bytes(ram[a] for a in (0x442, 0x43C, 0x44C, 0x44E, 0x446, 0x62C))
    elif phase >= 5:
        raise ValueError('Recovery continuation requires captured recovery context')
    return (b'CMJSESSION3' + field + clock
            + struct.pack('<BB4B', phase, int(pending is not None), *(pending or (0, 0, 0, 0)))
            + bytes((int(include_experience),)) + experience
            + bytes((int(include_recovery),)) + recovery)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ram', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--phase', type=int, choices=range(9), default=0)
    parser.add_argument('--pending', type=int, nargs=4, metavar=('CODE', 'DEFENDER', 'ACTION', 'SUBTYPE'))
    parser.add_argument('--without-experience', action='store_true', help='Do not infer campaign/experience context')
    parser.add_argument('--without-recovery', action='store_true', help='Omit unavailable recovery context')
    args = parser.parse_args()
    if args.output.suffix != '.cmjsession':
        parser.error('Use a private .cmjsession output path (ignored by Git)')
    try:
        data = export_session(args.ram.read_bytes(), args.phase, args.pending,
                              not args.without_experience, not args.without_recovery)
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, f'Session export stopped: {error}\n')
    print(f'Exported {len(data)} bytes; development phase session, not a complete match save.')


if __name__ == '__main__':
    main()

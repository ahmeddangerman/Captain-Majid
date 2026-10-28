#!/usr/bin/env python3
"""Export an explicit private RAM checkpoint into a bounded CMJSESSION7 file."""
import argparse
import struct
from pathlib import Path
from export_field_state import export_state


def export_session(ram, phase=0, pending=None, include_experience=True, include_recovery=True,
                   include_control=True, include_scheduling=True, include_action=True):
    field = export_state(ram)
    if phase not in range(21) or (phase == 3) != (pending is not None):
        raise ValueError('Only added-time phase3 requires pending terminal data')
    if phase >= 5 and phase not in (15, 18, 19, 20) and ram[0x600] != 0:
        raise ValueError('Recovery continuation requires zero selected defenders')
    if phase >= 6 and (ram[0x441] in (0, 11) or (0 if ram[0x441] < 11 else 11) != ram[0x5FB]):
        raise ValueError('Recovered owner must be a field player on the possession side')
    if phase >= 6 and not (48 <= ram[0x635] < 208 and 80 <= ram[0x637] < 176):
        raise ValueError('Recovered-owner continuation requires an interior ball')
    if phase >= 9 and not (include_experience and include_recovery and include_control):
        raise ValueError('Control-ready state requires all captured contexts')
    if phase >= 9 and ram[0x5FB] == 11 and not 1 <= ram[0x5FD] <= 10:
        raise ValueError('Opposing control-ready state needs a side0 field defender')
    if phase >= 13 and not include_scheduling:
        raise ValueError('Control-cycle phase requires captured scheduling context')
    if phase >= 18 and (ram[0x5FB] != 0 or not include_action or ram[0x621] >= 6):
        raise ValueError('Human action phase requires a supported captured action context')
    if include_action and ((phase >= 19 and ram[0x43B] > 9) or
                           (phase == 18 and ram[0x61E] & 128 and ram[0x43B] > 9)):
        raise ValueError('An active menu/confirmed action needs a supported action ID')
    if phase == 20 and ram[0x43B] != 0:
        raise ValueError('Prepared attack phase currently supports ordinary shot action0 only')
    if phase in (13, 14) and ram[0x5FF] != 1:
        raise ValueError('Control-cycle phase requires elapsed1')
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
    control = bytes(ram[a] for a in (0x614, 0x517, 0x5FD, 0x617, 0x642, 0x643, 0x8E, 0x532)) if include_control else bytes(8)
    schedule = bytes(8)
    if include_scheduling:
        count = ram[0x600]
        if count > 5:
            raise ValueError('Selected defender prefix exceeds five slots')
        ids = list(ram[0x601:0x601 + count])
        if any(id not in (*range(1, 11), *range(12, 22)) for id in ids):
            raise ValueError('Selected prefix needs field-player IDs')
        schedule = bytes((ram[0x613], ram[0x618], ram[0x44B], *ids, *([0] * (5 - count))))
        if ((phase == 15 and count == 0) or
                (phase in (15, 18, 19, 20) and (count > (5 if ram[0x5FB] == 0 else 4)
                 or any((0 if id < 11 else 11) == ram[0x5FB] for id in ids)))):
            raise ValueError('Selected contact/menu prefix must contain opposing field players')
    action = bytes(ram[a] for a in (0x43B, 0x621, 0x61E, 0x61F, 0x628, 0x444, 0x443, 0x61C, 0x61D)) if include_action else bytes(9)
    return (b'CMJSESSION7' + field + clock
            + struct.pack('<BB4B', phase, int(pending is not None), *(pending or (0, 0, 0, 0)))
            + bytes((int(include_experience),)) + experience
            + bytes((int(include_recovery),)) + recovery
            + bytes((int(include_control),)) + control
            + bytes((int(include_scheduling),)) + schedule
            + bytes((int(include_action),)) + action)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ram', type=Path)
    parser.add_argument('output', type=Path)
    parser.add_argument('--phase', type=int, choices=range(21), default=0)
    parser.add_argument('--pending', type=int, nargs=4, metavar=('CODE', 'DEFENDER', 'ACTION', 'SUBTYPE'))
    parser.add_argument('--without-experience', action='store_true', help='Do not infer campaign/experience context')
    parser.add_argument('--without-recovery', action='store_true', help='Omit unavailable recovery context')
    parser.add_argument('--without-control', action='store_true', help='Omit unavailable control context')
    parser.add_argument('--without-scheduling', action='store_true', help='Omit unavailable cycle/selection context')
    parser.add_argument('--without-action', action='store_true', help='Omit unavailable action/menu context')
    args = parser.parse_args()
    if args.output.suffix != '.cmjsession':
        parser.error('Use a private .cmjsession output path (ignored by Git)')
    try:
        data = export_session(args.ram.read_bytes(), args.phase, args.pending,
                              not args.without_experience, not args.without_recovery, not args.without_control,
                              not args.without_scheduling, not args.without_action)
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, f'Session export stopped: {error}\n')
    print(f'Exported {len(data)} bytes; development phase session, not a complete match save.')


if __name__ == '__main__':
    main()

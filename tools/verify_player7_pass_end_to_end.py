"""Replay a sampled player7 pass from phase 317 using private original evidence.

The compared bytes describe a bounded session, not a complete game save. Choices,
content, target and presentation acknowledgements are the sampled inputs; this
script does not simulate live AI, hardware timing, sound or rendering.
"""
import argparse
import subprocess
import sys
from pathlib import Path

from export_phase_session import export_session


class EvidenceMismatch(RuntimeError):
    pass


def field_name(offset):
    fields = ("side", "ball_owner", "ball_x", "ball_y", "ball_cell", "elapsed",
              "random_first", "random_second", "selected_defenders", "movement_region")
    if 20 <= offset < 30:
        return "field." + fields[offset - 20]
    if 30 <= offset < 34:
        return ("teams[0].formation", "teams[0].tactic", "teams[1].formation", "teams[1].tactic")[offset - 30]
    if 34 <= offset < 122:
        slot, part = divmod(offset - 34, 4)
        return f"roster[{slot}]." + ("type", "level", "stamina.low", "stamina.high")[part]
    if 122 <= offset < 242:
        slot, part = divmod(offset - 122, 6)
        player = slot + 1 if slot < 10 else slot + 2
        return f"motion[{player}]." + ("x.low", "x.high", "y.low", "y.high", "target", "status")[part]
    if 242 <= offset < 250:
        keeper, part = divmod(offset - 242, 4)
        return f"keeper[{keeper}]." + ("timer", "penalty_duration", "penalty", "stun")[part]
    if 250 <= offset < 258:
        return "clock." + ("remaining.low", "remaining.high", "extra_time", "flags", "script_flags", "ball_mode", "shared_recovery_flags", "shared_recovery_timer")[offset - 250]
    if 11 <= offset < 20:
        return f"field.magic[{offset - 11}]"
    if 267 <= offset < 289:
        slot, part = divmod(offset - 267, 2)
        return f"experience[{slot}]." + ("low" if part == 0 else "high")
    names = {259: "pending.present", 260: "pending.result_code", 261: "pending.defender_id", 262: "pending.action", 263: "pending.subtype",
             264: "experience.present", 265: "experience.campaign", 266: "experience.stage",
             289: "recovery.present", 290: "recovery.opposing_player", 291: "recovery.attack_subtype",
             292: "recovery.special_owner_flags", 293: "recovery.ball_variant", 294: "recovery.game_mode", 295: "recovery.x_jitter_control",
             296: "control.present", 297: "control.decision_timer", 298: "control.facing", 299: "control.controlled_player",
             300: "control.last_carrier_cell", 301: "control.travel.low", 302: "control.travel.high", 303: "control.display_parameter", 304: "control.motion_requested",
             305: "scheduling.present", 306: "scheduling.field_cycle", 307: "scheduling.energy_cycle", 308: "scheduling.special_flags",
             314: "action.present", 315: "action.id", 316: "action.menu_route", 317: "action.cursor", 318: "action.flash_counter",
             319: "action.auxiliary", 320: "action.move_effect", 321: "action.move_flags", 322: "action.power.low", 323: "action.power.high"}
    if 309 <= offset < 314:
        return f"scheduling.selected_ids[{offset - 309}]"
    return names.get(offset, f"session.byte[{offset}]")


def mismatch(case, boundary, field, actual, expected):
    raise EvidenceMismatch(f"case {case}, boundary {boundary}, {field}: native {actual}, original {expected}")


def compare_represented(actual, expected, extended, case, boundary):
    if len(actual) < 324 + int(extended):
        raise EvidenceMismatch(f"case {case}, boundary {boundary}: truncated native checkpoint")
    for offset in range(11, 324):
        if offset == 258:
            continue
        actual_offset = offset + int(extended and offset > 258)
        if actual[actual_offset] != expected[offset]:
            mismatch(case, boundary, field_name(offset), actual[actual_offset], expected[offset])


def case_ids(directory, suffix, expected_count=None):
    if not suffix or '/' in suffix or '\\' in suffix:
        raise ValueError("Start suffix must be a nonempty filename suffix")
    cases = sorted(path.name[:-len(suffix)] for path in directory.glob('*' + suffix) if path.is_file())
    if not cases or any(not case for case in cases):
        raise ValueError("No starting checkpoints with nonempty case names")
    if expected_count is not None and expected_count <= 0:
        raise ValueError("Expected case count must be positive")
    if expected_count is not None and len(cases) != expected_count:
        raise ValueError(f"Expected {expected_count} starting checkpoints, found {len(cases)}")
    return cases


def prepare_output(directory, checkout):
    output = directory.resolve()
    root = checkout.resolve()
    if output == root or root in output.parents:
        raise ValueError('Private replay output must be outside the Git checkout')
    output.mkdir(parents=True, exist_ok=True)
    if any(output.iterdir()):
        raise ValueError('Output directory must be empty to protect existing evidence')
    return output


def check_value(case, boundary, name, actual, expected):
    if actual != expected:
        mismatch(case, boundary, name, actual, expected)


def compare_contact_context(actual, source, cursor, score, case, boundary):
    wanted = (1, cursor, source[0x43d], 0, 0, source[0x32], 0, source[0x43f], 0, 0, 0, score, 0, 0)
    labels = ('present', 'cursor', 'action', 'subtype', 'effect', 'power.low', 'power.high', 'cost.low', 'cost.high',
              'strength.low', 'strength.high', 'score', 'result', 'weakened')
    for name, actual_value, expected_value in zip(labels, actual[399:413], wanted):
        check_value(case, boundary, 'contact.' + name, actual_value, expected_value)


def compare_flight_context(actual, raw, case, boundary):
    values = (1, raw[0x62b], raw[0x638], raw[0x61a], 0, 1)
    names = ('present', 'distance', 'destination', 'counter', 'loose_ball', 'trajectory_present')
    for name, actual_value, expected_value in zip(names, actual[339:345], values):
        check_value(case, boundary, 'flight.' + name, actual_value, expected_value)
    target_values = (1, raw[0x5fc], raw[0x638], 0, 0)
    target_names = ('present', 'recipient', 'cell', 'cost.low', 'cost.high')
    for name, actual_value, expected_value in zip(target_names, actual[393:398], target_values):
        check_value(case, boundary, 'flight_target.' + name, actual_value, expected_value)
    wanted = raw[0x634:0x638] + raw[0x639:0x63d] + bytes((raw[0x62c], raw[0x61b]))
    labels = ('x.low', 'x.high', 'y.low', 'y.high', 'vx.low', 'vx.high', 'vy.low', 'vy.high', 'direction', 'flags')
    for name, actual_value, expected_value in zip(labels, actual[345:355], wanted):
        check_value(case, boundary, 'trajectory.' + name, actual_value, expected_value)


def compare_checkpoint(actual, raw, phase, export_phase, case, boundary, contact=None, trajectory=False, target=False, contact_source=None):
    extended = phase > 255
    minimum = 425 if target else 421 if extended else 418
    if len(actual) < minimum:
        raise EvidenceMismatch(f"case {case}, boundary {boundary}: truncated native checkpoint")
    check_value(case, boundary, 'session.magic', actual[:10], b'CMJSESSION')
    check_value(case, boundary, 'session.version', actual[10], 147 if target else 146 if extended else 133)
    expected = export_session(raw, export_phase)
    compare_represented(actual, expected, extended, case, boundary)
    check_value(case, boundary, 'session.phase', actual[258] + (256 * actual[259] if extended else 0), phase)
    cursor_offset = 412 + int(extended)
    check_value(case, boundary, 'defenders.cursor', actual[cursor_offset], raw[0x616])
    flag = actual[-7] if target else actual[-3] if extended else (actual[-1] if actual[10] >= 133 else 0)
    check_value(case, boundary, 'ball.flight_flag', flag, raw[0x61b])
    score_offset = 361 + int(extended)
    for name, actual_value, expected_value in zip(('score.present', 'score.left', 'score.right', 'score.kickoff'),
                                                   actual[score_offset:score_offset + 4], (1, raw[0x28], raw[0x29], raw[0x629])):
        check_value(case, boundary, name, actual_value, expected_value)
    formation_offset = 365 + int(extended)
    for name, actual_value, expected_value in zip(('formation.present', 'formation.keeper_y[0]', 'formation.keeper_y[1]'),
                                                   actual[formation_offset:formation_offset + 3], (1, raw[0x308], raw[0x38c])):
        check_value(case, boundary, name, actual_value, expected_value)
    if extended:
        check_value(case, boundary, 'pass.cursor', actual[-2], raw[0x624])
        check_value(case, boundary, 'pass.steps', actual[-1], 5)
    if target:
        for name, actual_value, expected_value in zip(('target.recipient', 'target.cell', 'target.counter', 'target.distance'),
                                                       actual[-6:-2], (raw[0x5fc], raw[0x638], raw[0x61a], raw[0x62b])):
            check_value(case, boundary, name, actual_value, expected_value)
    if contact is not None:
        cursor, score = contact
        # Original scratch bytes are reused during the field update; retain the
        # third defender's independently recorded preparation as the source.
        compare_contact_context(actual, contact_source if contact_source is not None else raw, cursor, score, case, boundary)
    if trajectory:
        compare_flight_context(actual, raw, case, boundary)
    if phase in (1, 9):
        check_value(case, boundary, 'flight.present', actual[339], 0)
        check_value(case, boundary, 'flight.trajectory_present', actual[344], 0)
        check_value(case, boundary, 'flight_target.present', actual[393], 0)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('exe', 'pack', 'velocity', 'starts', 'encounter-original', 'handoff-original', 'flight-original', 'receive-original', 'control-original', 'output'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--start-suffix', default='-clock.cmjsession')
    parser.add_argument('--expected-cases', type=int, help='fail if the starting directory has a different case count')
    parser.add_argument('--flight-fractions', required=True, type=int, nargs=2, metavar=('X', 'Y'))
    args = parser.parse_args()
    if any(value < 0 or value > 255 for value in args.flight_fractions):
        raise ValueError('Flight fractions must be bytes')
    cases = case_ids(args.starts, args.start_suffix, args.expected_cases)
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    count = 0
    for case in cases:
        before = args.starts / (case + args.start_suffix)
        steps = [('phase317', None, args.encounter_original / f'{case}-80dc-1.bin', 317, 19, (0, 255), False, False),
                 ('defender19.prepare', ['--prepare-sampled-player3-defender'], args.encounter_original / f'{case}-80b6-2.bin', 318, 19, (1, 0), False, False),
                 ('defender19.compare', ['--compare-sampled-player3-defender', '153', '0', '0', '154', '96', '68', '0'], args.encounter_original / f'{case}-80d6-2.bin', 319, 19, (1, 255), False, False),
                 ('defender19.clock', ['--advance-sampled-player3-contact'], args.encounter_original / f'{case}-80dc-2.bin', 320, 19, (1, 255), False, False),
                 ('defender20.prepare', ['--prepare-sampled-player3-defender'], args.encounter_original / f'{case}-80b6-3.bin', 321, 19, (2, 0), False, False),
                 ('defender20.compare', ['--compare-sampled-player3-defender', '153', '0', '0', '154', '96', '68', '0'], args.encounter_original / f'{case}-80d6-3.bin', 322, 19, (2, 255), False, False),
                 ('defender20.clock', ['--advance-sampled-player3-contact'], args.encounter_original / f'{case}-80dc-3.bin', 323, 19, (2, 255), False, False),
                 ('dispatch', ['--advance-sampled-player3-dispatch'], args.encounter_original / f'{case}-80ed-1.bin', 324, 19, (2, 255), False, False),
                 ('field.handoff', ['--advance-sampled-score23-pass-handoff'], args.handoff_original / f'{case}-c612-1.bin', 325, 19, (2, 255), False, False),
                 ('pass.target', ['--capture-sampled-field-pass-target', '7', '103', '255', '3'], args.handoff_original / f'{case}-de64-1.bin', 326, 19, (2, 255), False, True),
                 ('flight.setup', ['--begin-sampled-field-pass-flight', str(args.velocity), *map(str, args.flight_fractions)], args.flight_original / f'{case}-e773-1.bin', 96, 19, None, True, False)]
        for step in range(2, 6):
            original = args.flight_original / f'{case}-e773-{step}.bin'
            steps.extend([(f'flight.step{step}', ['--advance-area-ball-cell'], original, 98, 19, None, True, False),
                          (f'flight.callback{step}', ['--resolve-area-ball-cell'], original, 96, 19, None, True, False)])
        steps.extend([('flight.arrival', ['--advance-area-ball-cell'], args.flight_original / f'{case}-e7cf-1.bin', 99, 19, None, True, False),
                      ('reception.owner', ['--continue-area-already-arrived'], args.receive_original / f'{case}-write-0441-1.bin', 100, 19, None, True, False),
                      ('reception.position', ['--continue-area-pass-received'], args.receive_original / f'{case}-write-0637-1.bin', 101, 19, None, True, False),
                      ('control.entry', ['--continue-sampled-field-pass-control'], args.receive_original / f'{case}-e0df-1.bin', 1, 19, None, False, False),
                      ('control.ready', ['--setup-control'], args.control_original / f'{case}-ready.bin', 9, 9, None, False, False)])
        for boundary, operation, original, phase, export_phase, contact, trajectory, target in steps:
            try:
                raw = original.read_bytes()
                if operation is not None:
                    destination = output / f'{case}-{boundary}.cmjsession'
                    result = subprocess.run([str(args.exe.resolve()), str(args.pack.resolve()), str(before.resolve()), *operation, '--capture', str(destination)], capture_output=True, text=True)
                    if result.returncode:
                        raise EvidenceMismatch(f'case {case}, boundary {boundary}: native operation failed: {result.stderr.strip()}')
                    before = destination
                contact_source = ((args.encounter_original / f'{case}-80b6-3.bin').read_bytes()
                                  if contact is not None and phase >= 325 else None)
                compare_checkpoint(before.read_bytes(), raw, phase, export_phase, case, boundary, contact, trajectory, target, contact_source)
                count += 1
            except (OSError, ValueError) as exc:
                raise EvidenceMismatch(f'case {case}, boundary {boundary}: {exc}') from exc
    print(f'Matched {count} boundaries across {len(cases)} cases from phase 317 through resumed control.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, EvidenceMismatch, IndexError) as exc:
        print(f'Player7 replay stopped: {exc}', file=sys.stderr)
        sys.exit(1)

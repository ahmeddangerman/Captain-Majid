"""Compare private native pass checkpoints through interception and control."""
import argparse
import hashlib
from pathlib import Path

from export_phase_session import export_session
from reference_identity import PAYLOAD_SHA256
from trace_ball_recovery_phase import trace_recovery
from check_ai_field_plan_original import original_choice
from verify_player7_pass_end_to_end import compare_represented


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'original', 'native-prefix', 'control-prefix'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--next-observation',type=Path)
    parser.add_argument('--one-two-original',type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese reference')
    for boundary, suffix, extended in (('prepared','2',True),
            ('defender-prepared','3',True), ('compared','4',True), ('handoff','5',False)):
        native = Path(str(args.native_prefix) + '-equalizer5-pass-' + suffix + '.cmjsession').read_bytes()
        raw = (args.original / (boundary + '.ram')).read_bytes()
        compare_represented(native, export_session(raw,0), extended, 'equalizer pass', boundary)
        if boundary == 'compared' and (raw[0x612],raw[0x442],raw[0x71]) != (3,17,7):
            raise ValueError('Expected observed interception by defender17')
    raw = (args.original / 'control.ram').read_bytes()
    native = Path(str(args.control_prefix) + '-equalizer17-control.cmjsession').read_bytes()
    compare_represented(native,export_session(raw,0),False,'equalizer pass','control')
    if (raw[0x441],raw[0x5fb],raw[0x635],raw[0x637],raw[0x5f7]) != (17,11,118,107,163):
        raise ValueError('Unexpected original control owner, location or clock')
    poll = Path(str(args.control_prefix) + '-equalizer17-poll.cmjsession').read_bytes()
    injected = bytearray(raw)
    injected[0xe2:0xe4] = poll[26:28]
    observation, final = trace_recovery(rom[16:262160],injected,control_input=(0,0))
    if observation['boundary'] != 'control_poll_wait':
        raise ValueError('Original next control poll reached a different boundary')
    compare_represented(poll,export_session(final,0),False,'equalizer pass','next control poll')
    journey = Path(str(args.control_prefix) + '-equalizer17-journey.txt')
    count = 0
    for line in journey.read_text().splitlines():
        index, phase, first, second, next_phase = map(int,line.split())
        injected = bytearray(raw)
        injected[0xe2:0xe4] = bytes((first,second))
        params = {9: {'control_input': (0,0)}, 10: {'control_tick': True},
                  13: {'world_inputs': [(0,0)] * 22}}[phase]
        observation, raw = trace_recovery(rom[16:262160],injected,**params)
        expected = 'player_control_ready' if phase == 13 and next_phase == 9 else {
            9:'control_poll_wait',10:'control_clock_pending',12:'ai_cell_decision',13:'field_round_pending'}[next_phase]
        if observation['boundary'] != expected:
            raise ValueError(f'Journey {index}: unexpected original boundary')
        native = Path(str(args.control_prefix) + f'-equalizer17-journey-{index}.cmjsession').read_bytes()
        compare_represented(native,export_session(raw,0),False,'interceptor journey',str(index))
        count += 1
    if count != 37 or next_phase != 12 or (raw[0x441],raw[0x5f7]) != (17,161):
        raise ValueError('Journey did not reach the observed player17 decision')
    unsupported, seed, choice_phase = map(int,Path(str(args.control_prefix) + '-equalizer17-choice.txt').read_text().split())
    if unsupported != -1:
        raise ValueError('Native interceptor AI choice stopped')
    injected = bytearray(raw)
    injected[0xe2:0xe4] = bytes((seed & 255,(seed >> 8) & 255))
    choice = original_choice(rom[16:262160],injected)
    native = Path(str(args.control_prefix) + '-equalizer17-choice.cmjsession').read_bytes()
    compare_represented(native,export_session(choice,0),False,'interceptor','AI choice')
    raw = choice
    for line in Path(str(args.control_prefix) + '-equalizer17-continuation.txt').read_text().splitlines():
        index, phase, first, second, next_phase = map(int,line.split())
        injected = bytearray(raw)
        injected[0xe2:0xe4] = bytes((first,second))
        if phase == 12:
            raw = original_choice(rom[16:262160],injected)
        else:
            _, raw = trace_recovery(rom[16:262160],injected,**{
                9:{'control_input':(0,0)},10:{'control_tick':True},13:{'world_inputs':[(0,0)]*22}}[phase])
        native = Path(str(args.control_prefix) + f'-equalizer17-continuation-{index}.cmjsession').read_bytes()
        compare_represented(native,export_session(raw,0),False,'interceptor continuation',str(index))
    stopped = Path(str(args.control_prefix) + '-equalizer17-stopped.txt')
    if next_phase == 12 and stopped.exists():
        seed = int(stopped.read_text())
        seed ^= (seed << 13) & 0xffffffff
        seed ^= seed >> 17
        seed ^= (seed << 5) & 0xffffffff
        injected = bytearray(raw)
        injected[0xe2:0xe4] = bytes((seed & 255,(seed >> 8) & 255))
        pending = original_choice(rom[16:262160],injected)
        print(f'Next original choice: action{pending[0x43b]}, subtype{pending[0x43c]}, recipient{pending[0x5fc]}, seed{seed}, clock{pending[0x5f7]}.')
        if args.next_observation:
            if args.next_observation.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
                raise ValueError('Private observations must stay outside checkout')
            from check_unopposed_ai_pass_original import observe
            outcome, checkpoints = observe(rom[16:262160],injected,expected_action=3,stop_at_second_arrival=True)
            args.next_observation.mkdir(parents=True,exist_ok=False)
            for name, checkpoint in checkpoints.items():
                (args.next_observation / (name + '.ram')).write_bytes(checkpoint)
            (args.next_observation / 'decision.ram').write_bytes(injected)
            print(f'Original one-two outcome: {outcome}; boundaries: {list(checkpoints)}')
    print(f'Original/native AI choice matches: action{choice[0x43b]}, native phase{choice_phase}.')
    if args.one_two_original:
        for suffix, boundary in (('prepared','prepared'), ('setup-1','preclock'),
                ('setup-3','return-target'), ('setup-4','flight'),
                ('swap','swap-after'), ('return-flight','return-flight'),
                ('return-contact','return-defender-cleanup'), ('received','return-area'),
                ('area','area-setup'), ('area-selection','area-selection'),
                ('area-choice','area-choice'), ('area-prepared','area-prepared')):
            native = Path(str(args.control_prefix) + '-equalizer17-one-two-' + suffix + '.cmjsession').read_bytes()
            raw = (args.one_two_original / (boundary + '.ram')).read_bytes()
            compare_represented(native,export_session(raw,0),False,'interceptor one-two',boundary)
        print('Twelve one-two/goal-area boundaries match through player17 shot preparation.')
    print('Six represented boundaries match; opponent player17 controls at (118,107), clock163, and continues control.')
    print(f'{count} further movement/clock/control boundaries match through player17 AI decision, clock161.')


if __name__ == '__main__':
    main()

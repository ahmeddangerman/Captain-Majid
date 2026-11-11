#!/usr/bin/env python3
"""Compare field AI fallback preparation after constructed skip-defense menus."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original_preparation(prg, raw, through_clock=False, through_return=False, through_control=False, through_handoff=False, through_target=False, through_flight=False, through_crossing=False):
    memory = ObservedCartridge(prg, raw, None, set())
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1),
                           (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb),
                           (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26, 27]
    memory[0x151:0x155] = [0x67, 0xd5, 0x46, 0x80]
    memory[0x1c], memory[0x1e] = 128, 128
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = 0xd3ab, 0x50, cpu.UNUSED
    flight_hits = 0
    for _ in range(500000):
        bank = memory.bank_at(cpu.pc)
        if through_crossing and cpu.pc == 0xe773:
            flight_hits += 1
            if flight_hits == 2:
                return bytes(memory.ram[:2048])
        if (through_flight and cpu.pc == 0xe773) or (through_target and not through_flight and bank == 30 and cpu.pc == 0xde64) or (through_handoff and not through_target and not through_flight and bank == 30 and cpu.pc == 0xc612) or (through_control and cpu.pc == 0xe145) or (through_return and not through_control and cpu.pc == 0xe0df) or (not through_return and not through_control and not through_handoff and not through_target and not through_flight and not through_crossing and bank == 26 and cpu.pc == (0x80ed if through_clock else 0x804f)):
            return bytes(memory.ram[:2048])
        if cpu.pc < 0x8000:
            raise RuntimeError(f'Original preparation left PRG at {cpu.pc:04x}')
        if cpu.pc == 0xcaae:
            memory.instruction = None
            memory[0x1b] |= 128
            memory[0x515] = 0
            memory[0x1c], memory[0x1e] = 128, 128
            if memory[0x498]:
                memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc)
        cpu.step()
    raise RuntimeError('Original field preparation instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'session-dir', 'exe', 'private-temp'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--vary-profile', action='store_true', help='Construct different original player types, flags and levels')
    parser.add_argument('--through-clock', action='store_true', help='Continue original and runtime through attack dispatch clock')
    parser.add_argument('--through-return', action='store_true', help='Continue through action presentation and field control handoff')
    parser.add_argument('--through-control', action='store_true', help='Continue through opponent control setup')
    parser.add_argument('--through-handoff', action='store_true', help='Continue an ordinary pass through field movement and handoff')
    parser.add_argument('--through-target', action='store_true', help='Continue an ordinary pass through target distance calculation')
    parser.add_argument('--through-flight', action='store_true', help='Continue an ordinary pass through numeric flight setup')
    parser.add_argument('--first-crossing', action='store_true', help='Continue an ordinary pass through its first empty defender callback')
    parser.add_argument('--pass-reception', action='store_true', help='Continue an ordinary pass through receiver handoff')
    parser.add_argument('--pass-control', action='store_true', help='Continue an ordinary pass through receiver control setup')
    parser.add_argument('--distance', type=Path, help='Private original-derived pass distance table')
    parser.add_argument('--velocity', type=Path, help='Private original-derived ball velocity table')
    parser.add_argument('--pass', dest='pass_choice', action='store_true', help='Compare an ordinary zero-option pass instead of fallback action2')
    args = parser.parse_args()
    if (args.through_target or args.through_flight or args.first_crossing or args.pass_reception or args.pass_control) and (args.distance is None or (args.vary_profile and not args.pass_control)):
        parser.error('Target comparison requires --distance and does not support --vary-profile')
    if (args.through_flight or args.first_crossing or args.pass_reception or args.pass_control) and args.velocity is None:
        parser.error('Flight comparison requires --velocity')
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Temporary original RAM must stay outside the checkout')
    prg, count = rom[16:262160], 0
    skipped = []
    contested = 0
    with tempfile.TemporaryDirectory(prefix='field-ai-preparation-', dir=args.private_temp) as temp:
        sessions = sorted(args.session_dir.glob('*-complete.cmjsession'))
        if args.vary_profile:
            sessions = sessions[:1]
        for session in sessions:
            complete = session.with_suffix('.bin').read_bytes()
            before = session.with_name(session.name.replace('-complete.cmjsession', '-register.bin')).read_bytes()
            kinds = (7, 23, 35, 70) if args.pass_control else (1, 7, 23, 35, 70)
            profiles = [(kind, flags, alternate, level) for kind in kinds
                        for flags, alternate in ((0, 0), (128, 35)) for level in (0, 5)] if args.vary_profile else [None]
            for profile in profiles:
                checked = False
                profile_contested = 0
                actor = complete[0x441]
                complete_case, before_case = bytearray(complete), bytearray(before)
                if profile is not None:
                    for ram in (complete_case, before_case):
                        ram[0x300 + 12 * actor:0x304 + 12 * actor] = bytes(profile)
                seed_limit = 512 if args.vary_profile and args.pass_control else 2000
                for seed in range(1, seed_limit):
                    value = seed
                    value ^= (value << 13) & 0xffffffff
                    value ^= value >> 17
                    value ^= (value << 5) & 0xffffffff
                    value &= 0xffffffff
                    raw = bytearray(complete_case)
                    raw[0xe2], raw[0xe3] = value & 255, (value >> 8) & 255
                    expected_action = 1 if args.pass_choice or args.through_handoff or args.through_target or args.through_flight or args.first_crossing or args.pass_reception or args.pass_control else 2
                    if original_candidate(prg, raw)[2] != 2 or original_choice(prg, raw)[0x43b] != expected_action:
                        continue
                    start = bytearray(before_case)
                    start[0xe2], start[0xe3] = value & 255, (value >> 8) & 255
                    final = original_preparation(prg, start, args.through_clock, args.through_return or args.pass_reception, args.through_control or args.pass_control, args.through_handoff, args.through_target, args.through_flight, args.first_crossing)
                    output = Path(temp) / 'original.bin'
                    output.write_bytes(final)
                    mode = '--compare-field-ai-pass-control' if args.pass_control else '--compare-field-ai-reception' if args.pass_reception else '--compare-field-ai-crossing' if args.first_crossing else '--compare-field-ai-flight' if args.through_flight else '--compare-field-ai-target' if args.through_target else '--compare-field-ai-handoff' if args.through_handoff else '--compare-field-ai-control' if args.through_control else '--compare-field-ai-return' if args.through_return else '--compare-field-ai-clock' if args.through_clock else '--compare-field-ai-preparation'
                    command = [str(args.exe.resolve()), mode, str(session.resolve()),
                               str(output), str(args.pack.resolve()), str(seed)]
                    if args.through_target or args.through_flight or args.first_crossing or args.pass_reception or args.pass_control:
                        command.append(str(args.distance.resolve()))
                    if args.through_flight or args.first_crossing or args.pass_reception or args.pass_control:
                        command.append(str(args.velocity.resolve()))
                    if args.first_crossing or args.pass_reception or args.pass_control:
                        setup = original_preparation(prg, start, through_flight=True)
                        setup_path = Path(temp) / 'setup.bin'
                        setup_path.write_bytes(setup)
                        command.append(str(setup_path))
                    if profile is not None:
                        command.extend(map(str, profile))
                    result = subprocess.run(command, capture_output=True, text=True)
                    if result.returncode:
                        if args.vary_profile and args.pass_control and 'pass-flight defender contact' in result.stderr:
                            contested += 1
                            profile_contested += 1
                            if profile_contested >= 3:
                                break
                            continue
                        raise RuntimeError(f'{session.name} profile {profile} seed {seed}, exit {result.returncode}: {result.stderr[-500:]} {result.stdout[-500:]}')
                    count += 1
                    checked = True
                    break
                if not checked:
                    if args.vary_profile and args.pass_control:
                        skipped.append((profile, 'contested' if profile_contested else 'no pass'))
                    else:
                        raise RuntimeError(f'No original requested action found for {session.name} profile {profile}')
    if not count:
        raise RuntimeError('No matching input sessions found')
    suffix = ' and receiver control setups' if args.pass_control else ' and receiver handoffs' if args.pass_reception else ' and first pass crossings' if args.first_crossing else ' and pass flight setups' if args.through_flight else ' and pass targets' if args.through_target else ' and pass handoffs' if args.through_handoff else ' and control setups' if args.through_control else ' and control handoffs' if args.through_return else ' and dispatch clocks' if args.through_clock else ''
    print(f'{count} constructed runtime field attack preparations{suffix} matched original')
    if skipped:
        print(f'{len(skipped)} constructed profiles remained unmatched in the bounded survey: {skipped}')
    if contested:
        print(f'{contested} candidate runs stopped at unsupported pass-flight defender contact')


if __name__ == '__main__':
    main()

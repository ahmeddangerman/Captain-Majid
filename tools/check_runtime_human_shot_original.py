#!/usr/bin/env python3
"""Compare provisional ordinary human shot preparation with constructed original CPU cases."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from export_phase_session import export_session
from check_kickoff_shot_launch_original import observe_clock
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_field_loop import trace_loop
from trace_shot_approach import trace_approach
from trace_field_script import ObservedCartridge
from check_kickoff_flight_original import until
from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--tail', action='store_true', help='Intervene on the retained defensive action and compare the attack tail')
    parser.add_argument('--flight', action='store_true', help='Also compare numeric flight setup')
    parser.add_argument('--defenders', action='store_true', help='Intervene on flight defenders and random bytes')
    parser.add_argument('--keeper', action='store_true', help='Compare unopposed arrival and normal keeper entry')
    parser.add_argument('--velocity', type=Path, help='Private ball velocity table for --flight')
    args = parser.parse_args()
    if args.defenders and args.keeper: parser.error('Keeper survey needs an unopposed arrival')
    if args.defenders or args.keeper: args.flight = True
    if args.flight:
        args.tail = True
        if not args.velocity: parser.error('--flight requires --velocity')
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside the checkout')
    prg = rom[16:262160]
    matched = flight_boundaries = defended_arrivals = 0
    with tempfile.TemporaryDirectory(prefix='human-shot-', dir=args.private_temp) as temp:
        before_file, after_file, raw_file, records_file = Path(temp) / 'before.cmjsession', Path(temp) / 'after.cmjsession', Path(temp) / 'approach.bin', Path(temp) / 'flight-records.bin'
        for fixture in range(4):
            cases = ((stamina, action) for stamina in (80, 400) for action in range(7)) if args.tail else ((stamina, 0) for stamina in (80, 81, 100, 255, 400, 900))
            for stamina, action in cases:
                ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes())
                owner = ready[0x441]
                ready[0x301 + 12 * owner:0x303 + 12 * owner] = stamina.to_bytes(2, 'little')
                ready[0x43d] = action
                _, menu_input = trace_recovery(prg, ready, control_input=(64, 0))
                _, opened = trace_menu(prg, menu_input, mode='open')
                report, before = trace_menu(prg, opened, mode='poll', pressed=129)
                if report['boundary'] != 'attack_choice_pending' or before[0x43b] or before[0x600]:
                    raise RuntimeError('Expected unopposed ordinary shot choice')
                _, after = trace_menu(prg, before, mode='basic-shot')
                before_file.write_bytes(export_session(before, 19))
                after_file.write_bytes(export_session(after, 20))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina}: {result.stderr}')
                clocked, _, _, _ = observe_clock(prg, after)
                before_file.write_bytes(export_session(after, 20))
                after_file.write_bytes(export_session(clocked))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-clock', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina} dispatch: {result.stderr}')
                movement = bytearray()
                trace_loop(prg, clocked, dispatch_only=True, final_ram=movement)
                after_file.write_bytes(export_session(movement))
                result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-movement', str(before_file),
                                         str(after_file), str(args.pack.resolve())], capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'fixture={fixture}, stamina={stamina} movement: {result.stderr}')
                if args.tail:
                    if movement[0x43d] != action:
                        raise RuntimeError('Original shot path changed the retained defensive action')
                    tail = bytearray()
                    report = trace_loop(prg, movement, attack_tail=True, final_ram=tail)
                    if report['boundary'] != 'shot_approach_entry':
                        raise RuntimeError(f'Unexpected original attack tail: {report["boundary"]}')
                    after_file.write_bytes(export_session(tail))
                    result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-tail', str(before_file),
                                             str(after_file), str(args.pack.resolve()), str(action)], capture_output=True, text=True)
                    if result.returncode:
                        raise RuntimeError(f'fixture={fixture}, stamina={stamina}, action={action} tail: {result.stderr}')
                    approach_report, approach = trace_approach(prg, tail)
                    raw_file.write_bytes(approach)
                    after_file.write_bytes(export_session(approach))
                    result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-approach', str(before_file),
                                             str(after_file), str(args.pack.resolve()), str(action), str(raw_file),
                                             str(int(approach_report['loose_ball']))], capture_output=True, text=True)
                    if result.returncode:
                        raise RuntimeError(f'fixture={fixture}, stamina={stamina}, action={action} approach: {result.stderr}')
                    if args.flight:
                        if args.defenders:
                            approach = bytearray(approach);approach[0xe2],approach[0xe3] = 1,0
                            for player,x,y in ((12,148,112),(13,164,116)):
                                record=0x300+12*player;approach[record+6],approach[record+8],approach[record+10]=x,y,0
                        memory = ObservedCartridge(prg, approach, None, set());memory.registers[6:8] = [26, 27]
                        cpu = MPU(memory=memory);cpu.pc, cpu.sp, cpu.p = 0xe73e, 0xfc, cpu.UNUSED
                        memory[0x1fd],memory[0x1fe],memory[0x1ff] = 0x10,0xdd,cpu.UNUSED|int(approach_report['loose_ball'])
                        until(cpu,{0xe773,0xe7cf})
                        if cpu.pc != 0xe773: raise RuntimeError('Same-cell shot flight requires separate coverage')
                        flight = bytes(memory.ram[:2048]);raw_file.write_bytes(flight);after_file.write_bytes(export_session(flight))
                        if not args.defenders:
                            result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-flight-setup', str(before_file),
                                                     str(after_file), str(args.pack.resolve()), str(action), str(raw_file),
                                                     str(int(approach_report['loose_ball'])), str(args.velocity.resolve())], capture_output=True, text=True)
                            if result.returncode:
                                raise RuntimeError(f'fixture={fixture}, stamina={stamina}, action={action} flight (exit {result.returncode}): {result.stderr}')
                        if approach_report['loose_ball']: raise RuntimeError('Loose-ball shot approach needs separate flight coverage')
                        records = bytearray();boundaries = 0
                        for _ in range(100):
                            cpu.pc = 0xe778;until(cpu,{0xe7b4,0xe7cc})
                            records.append(0 if cpu.pc == 0xe7b4 else 1);records.extend(memory.ram[:2048])
                            boundaries += 1
                            if cpu.pc == 0xe7cc: break
                            until(cpu,{0xe773});records.append(2);records.extend(memory.ram[:2048]);boundaries += 1
                        else: raise RuntimeError('Original shot flight did not arrive')
                        defended_at_arrival = memory[0x600] != 0
                        if args.keeper:
                            if memory[0x600]: raise RuntimeError('Keeper survey reached a field defender')
                            until(cpu,{0xdd1c});records.append(3);records.extend(memory.ram[:2048]);boundaries += 1
                            until(cpu,{0x8509,0x8900})
                            if cpu.pc!=0x8509: raise RuntimeError('Keeper survey encountered a stunned keeper')
                            records.append(4);records.extend(memory.ram[:2048]);boundaries += 1
                        records_file.write_bytes(records);before_file.write_bytes(export_session(flight));raw_file.write_bytes(flight)
                        result = subprocess.run([str(args.exe.resolve()), '--compare-human-shot-flight-chain', str(before_file),
                                                 str(raw_file), str(records_file), str(args.pack.resolve())], capture_output=True, text=True)
                        if result.returncode:
                            raise RuntimeError(f'fixture={fixture}, stamina={stamina}, action={action} flight chain (exit {result.returncode}): {result.stderr}')
                        flight_boundaries += boundaries;defended_arrivals += int(defended_at_arrival)
                matched += 1
    print(f'{matched} constructed original unopposed shot preparations, dispatch clocks and field movements' + (', attack tails and approaches' if args.tail else '') + (', and flight setups' if args.flight and not args.defenders else '') + ' matched')
    if args.flight: print(f'{flight_boundaries} numeric flight/callback boundaries matched; {defended_arrivals} arrivals had defenders')


if __name__ == '__main__':
    main()

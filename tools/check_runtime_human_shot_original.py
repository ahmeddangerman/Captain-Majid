#!/usr/bin/env python3
"""Compare provisional ordinary human shot preparation with constructed original CPU cases."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from export_phase_session import export_session
from check_kickoff_shot_launch_original import observe_clock
from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_field_loop import trace_loop
from trace_shot_approach import trace_approach
from check_scored_goal_original import observe_scored_goal
from check_keeper_entry_original import observe as observe_keeper_entry
from check_runtime_goal_kickoff_original import capture as kickoff_capture
from trace_field_script import ObservedCartridge
from check_kickoff_flight_original import until, snapshot as flight_snapshot
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
    parser.add_argument('--end-to-end', action='store_true', help='Compare uninterrupted shot through an AI keeper and goal')
    parser.add_argument('--kickoff-end-to-end', action='store_true', help='Continue uninterrupted shot through opponent kickoff control')
    parser.add_argument('--fraction-survey', action='store_true', help='Observe original fractional ball bytes through goal/kickoff')
    parser.add_argument('--post-kickoff', action='store_true', help='Also exercise native neutral control until an opponent decision or named stop')
    parser.add_argument('--velocity', type=Path, help='Private ball velocity table for --flight')
    parser.add_argument('--distance', type=Path, help='Private pass distance table for post-kickoff passing')
    parser.add_argument('--after-pass', action='store_true', help='Continue the native provisional run past receiver control to the next stop')
    parser.add_argument('--limit', type=int, default=0, help='Stop after this many constructed cases (zero means all)')
    args = parser.parse_args()
    if args.fraction_survey and not args.kickoff_end_to_end: parser.error('--fraction-survey requires --kickoff-end-to-end')
    if args.post_kickoff and not args.fraction_survey: parser.error('--post-kickoff requires --fraction-survey')
    if args.distance and not args.post_kickoff: parser.error('--distance requires --post-kickoff')
    if args.after_pass and not args.distance: parser.error('--after-pass requires --distance')
    if args.kickoff_end_to_end: args.end_to_end = True
    if args.end_to_end: args.keeper = True
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
    matched = flight_boundaries = defended_arrivals = goals = 0
    fraction_counts = Counter()
    post_counts = Counter()
    with tempfile.TemporaryDirectory(prefix='human-shot-', dir=args.private_temp) as temp:
        before_file, after_file, raw_file, records_file = Path(temp) / 'before.cmjsession', Path(temp) / 'after.cmjsession', Path(temp) / 'approach.bin', Path(temp) / 'flight-records.bin'
        for fixture in range(4):
            if args.limit and matched >= args.limit: break
            cases = ((stamina, action) for stamina in (80, 400) for action in range(7)) if args.tail else ((stamina, 0) for stamina in (80, 81, 100, 255, 400, 900))
            for stamina, action in cases:
                if args.limit and matched >= args.limit: break
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
                choice_source = export_session(before, 19)
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
                        arrival_fractions=(memory[0x634],memory[0x636])
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
                        if args.end_to_end:
                            for _ in range(2_000_000):
                                if cpu.pc==0x8599:break
                                if cpu.pc<0x8000:raise RuntimeError(f'Original keeper left PRG at {cpu.pc:04x}')
                                if cpu.pc==0xcaae:
                                    memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0
                                    if memory[0x498]:memory[0x498]-=1
                                memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
                            else:raise RuntimeError('Original keeper comparison instruction bound')
                            keeper_result=bytes(memory.ram[:2048])
                            if keeper_result[0x612]!=0: raise RuntimeError('End-to-end goal fixture did not produce a goal')
                            goal,_=observe_scored_goal(prg,keeper_result)
                            if goal[0x62d]:raise RuntimeError('End-to-end restart requires ball mode zero')
                            restart,_,_=observe_keeper_entry(prg,goal,0x85bf,{0x85dd});before_file.write_bytes(choice_source)
                            if args.kickoff_end_to_end:
                                kickoff_memory=ObservedCartridge(prg,restart,None,set());kickoff_memory.registers[6:8]=[26,27]
                                kickoff_memory[0x24],kickoff_memory[0x25]=26,27;kickoff_memory[0x23]=kickoff_memory[0x22]|7;kickoff_memory.select=kickoff_memory[0x23]
                                kickoff_cpu=MPU(memory=kickoff_memory);kickoff_cpu.pc,kickoff_cpu.sp,kickoff_cpu.p=0x85dd,0xfd,kickoff_cpu.UNUSED
                                for _ in range(2_000_000):
                                    if kickoff_cpu.pc==0xdb0e:break
                                    if kickoff_cpu.pc<0x8000:raise RuntimeError('Original kickoff left PRG')
                                    if kickoff_cpu.pc==0xcaae:
                                        kickoff_memory.instruction=None;kickoff_memory[0x1b]|=128;kickoff_memory[0x515]=0
                                        if kickoff_memory[0x498]:kickoff_memory[0x498]-=1
                                    kickoff_memory.instruction=(kickoff_memory.bank_at(kickoff_cpu.pc),kickoff_cpu.pc);kickoff_cpu.step()
                                else:raise RuntimeError('Original kickoff instruction bound')
                                kickoff_cpu.pc=0xdb13;until(kickoff_cpu,{0xdb1e})
                                _,ready=trace_recovery(prg,bytes(kickoff_memory.ram[:2048]),prepare_control=True);after_file.write_bytes(kickoff_capture(ready,9))
                                if ready[0x43d]!=keeper_result[0x43d]:raise RuntimeError('Original keeper action was not retained through kickoff')
                                if args.fraction_survey:
                                    previous=arrival_fractions
                                    for name,raw in (('keeper',keeper_result),('goal',goal),('restart',restart),('kickoff-control',ready)):
                                        current=(raw[0x634],raw[0x636]);fraction_counts[name+(':same' if current==previous else ':changed')]+=1;previous=current
                            else:after_file.write_bytes(flight_snapshot(restart,60,False))
                            command=[str(args.exe.resolve()),'--compare-human-shot-end-to-end',str(before_file),str(after_file),str(args.pack.resolve()),str(action),str(raw_file),str(args.velocity.resolve()),str(keeper_result[0x28]),str(keeper_result[0x29]),str(keeper_result[0x629])]
                            if args.kickoff_end_to_end:command.extend(('control',str(keeper_result[0x43d])))
                            if args.fraction_survey:command.extend((str(ready[0x634]),str(ready[0x636])))
                            if args.post_kickoff:command.append('post')
                            if args.distance:command.append(str(args.distance.resolve()))
                            if args.after_pass:command.append('continue')
                            result=subprocess.run(command,capture_output=True,text=True)
                            if result.returncode:raise RuntimeError(f'fixture={fixture}, stamina={stamina}, action={action} end-to-end: {result.stderr}')
                            if args.post_kickoff:
                                outcome=result.stdout.strip()
                                if not outcome.startswith('post: '):raise RuntimeError('Missing native post-kickoff outcome')
                                post_counts[outcome]+=1
                            goals+=1
                matched += 1
    print(f'{matched} constructed original unopposed shot preparations, dispatch clocks and field movements' + (', attack tails and approaches' if args.tail else '') + (', and flight setups' if args.flight and not args.defenders else '') + ' matched')
    if args.flight: print(f'{flight_boundaries} numeric flight/callback boundaries matched; {defended_arrivals} arrivals had defenders')
    if args.end_to_end: print(f'{goals} uninterrupted native shot-to-'+('kickoff-control' if args.kickoff_end_to_end else 'restart')+' sequences matched the original boundary')
    if args.fraction_survey: print(f'Original goal/kickoff fraction survey: {dict(sorted(fraction_counts.items()))}')
    if args.post_kickoff: print(f'Native provisional post-kickoff continuations: {dict(sorted(post_counts.items()))}')


if __name__ == '__main__':
    main()

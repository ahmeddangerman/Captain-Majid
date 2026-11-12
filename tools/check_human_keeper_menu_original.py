#!/usr/bin/env python3
"""Trace normal home goalkeeper choices after a constructed opponent shot."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_unopposed_ai_shot_original import observe
from check_scored_goal_original import observe_scored_goal
from check_keeper_saves_original import observe_keeper_save
from check_keeper_entry_original import observe as observe_keeper_entry
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_action_menu import trace_menu
from verify_ordinary_pass_initialization import original_target
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def next_random(value):
    value ^= (value << 13) & 0xffffffff; value ^= value >> 17; value ^= (value << 5) & 0xffffffff
    return value


def menu_trace(prg, entry, pressed, seed):
    memory = ObservedCartridge(prg, entry, None, {0x43d, 0x61e, 0x621})
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1), (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0xdd1c, 0x50, cpu.UNUSED
    checkpoints = {}; stage = 0; button = 0; rng = seed
    for _ in range(2_000_000):
        bank = memory.bank_at(cpu.pc)
        if (bank, cpu.pc) == (26, 0x8509) and 'entry' not in checkpoints: checkpoints['entry'] = bytes(memory.ram[:2048])
        if (bank, cpu.pc) == (30, 0xd2b4):
            if stage == 0:
                checkpoints['open'] = bytes(memory.ram[:2048]); stage = 1; button = pressed; rng = next_random(rng)
            elif stage == 1:
                checkpoints['selected'] = bytes(memory.ram[:2048]); stage = 2; button = 128; rng = next_random(rng)
        if (bank, cpu.pc) == (26, 0x852f) and 'confirmed' not in checkpoints:
            checkpoints['confirmed'] = bytes(memory.ram[:2048]); stage = 3; button = 0
        if (bank, cpu.pc) == (26, 0x8599) and 'confirmed' in checkpoints:
            checkpoints['compared'] = bytes(memory.ram[:2048]); return checkpoints
        if cpu.pc < 0x8000: raise RuntimeError(f'Original goalkeeper menu left PRG at {cpu.pc:04x}')
        if cpu.pc == 0xcaae:
            memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = button
            if stage in (1, 2): memory[0xe2], memory[0xe3] = rng & 255, (rng >> 8) & 255
            if memory[0x498]: memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc); cpu.step()
    raise RuntimeError('Original goalkeeper menu instruction limit')


def kickoff_trace(prg, restart):
    memory = ObservedCartridge(prg, restart, None, set()); memory.registers[6:8] = [26, 27]
    memory[0x24], memory[0x25] = 26, 27; memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0x85dd, 0xfd, cpu.UNUSED
    def until(target):
        for _ in range(2_000_000):
            if cpu.pc == target: return
            if cpu.pc < 0x8000: raise RuntimeError(f'Original kickoff left PRG at {cpu.pc:04x}')
            if cpu.pc == 0xcaae:
                memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0
                if memory[0x498]: memory[0x498] -= 1
            memory.instruction = (memory.bank_at(cpu.pc), cpu.pc); cpu.step()
        raise RuntimeError('Original kickoff instruction limit')
    until(0xdae9); until(0xdaf5); formation = bytes(memory.ram[:2048])
    until(0xdb0b); receiver = bytes(memory.ram[:2048])
    until(0xdb0e); ball = bytes(memory.ram[:2048])
    cpu.pc = 0xdb13; until(0xdb1e); handoff = bytes(memory.ram[:2048])
    _, control = trace_recovery(prg, handoff, prepare_control=True)
    return {'formation': formation, 'receiver': receiver, 'ball': ball, 'handoff': handoff, 'control': control}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'velocity', 'exe', 'private-temp'): parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--original-only', action='store_true')
    parser.add_argument('--save-survey', action='store_true')
    parser.add_argument('--catch-survey', action='store_true')
    parser.add_argument('--keeper-action-survey', action='store_true')
    parser.add_argument('--keeper-action-one', action='store_true')
    parser.add_argument('--vary-keeper-y', action='store_true')
    args = parser.parse_args(); rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    prg = rom[16:262160]; counts = Counter(); recovery_counts = Counter(); catch_changes = Counter()
    with tempfile.TemporaryDirectory(prefix='human-keeper-menu-', dir=args.private_temp) as temporary:
        temp = Path(temporary); source = temp / 'source.cmjsession'; records = temp / 'records.bin'
        for fixture in range(4):
            ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes()); ready[0x614] = ready[0x617] = 0
            report, decision = trace_recovery(prg, ready, control_input=(0, 0))
            if report['boundary'] != 'ai_cell_decision': raise RuntimeError('Expected original cell decision')
            for x in (85, 95):
                base = bytearray(decision); owner = base[0x441]; cell = ((x - 48) // 8) * 12 + 7
                base[0x635], base[0x637], base[0x5fe], base[0x617], base[0x5ff], base[0x43d] = x, 137, cell, cell, 0, 6
                base[0x306 + 12 * owner], base[0x308 + 12 * owner] = x, 137
                for seed in range(1, 500):
                    value = next_random(seed); base[0xe2], base[0xe3] = value & 255, (value >> 8) & 255
                    if original_candidate(prg, base)[2] == 1: break
                else: raise RuntimeError('No original shot candidate')
                outcome, shot = observe(prg, base)
                if outcome != 'flight': raise RuntimeError('Expected original shot flight')
                flight = shot['flight-records']; entry = next((flight[i + 1:i + 2049] for i in range(0, len(flight), 2049) if flight[i] == 3), None)
                if entry is None: counts['defended-or-loose'] += 1; continue
                for keeper_stamina in (20, 100, 400):
                    for pressed, power in ((1, 2), (1, 11), (1, 30), (2, 11), (8, 2), (8, 11), (8, 30)):
                        for menu_seed in (1, 123, 98765):
                            raw = bytearray(entry); raw[0x301:0x303] = keeper_stamina.to_bytes(2, 'little'); raw[0x61c:0x61e] = power.to_bytes(2, 'little')
                            if args.vary_keeper_y: raw[0x308] = {1:112, 123:124, 98765:140}[menu_seed]
                            states = menu_trace(prg, raw, pressed, menu_seed)
                            result = states['compared'][0x612]
                            states['goal'] = (observe_scored_goal(prg, states['compared'])[0] if result == 0 else
                                              observe_keeper_save(prg, states['compared'])[0] if result in (2, 3) else states['compared'])
                            states['caught_ready'] = states['caught_poll'] = states['caught_menu'] = states['caught_open'] = states['caught_selected'] = states['caught_confirmed'] = states['caught_target'] = states['compared']
                            if result == 3 and states['compared'][0x43d] == 0:
                                report, caught_ready = trace_recovery(prg, states['goal'], prepare_control=True)
                                if report['boundary'] != 'player_control_ready': raise RuntimeError('Unexpected caught keeper control boundary')
                                states['caught_ready'] = caught_ready
                                poll_input = bytearray(caught_ready); next_rng = next_random(next_random(next_random(menu_seed)))
                                poll_input[0xe2], poll_input[0xe3] = next_rng & 255, (next_rng >> 8) & 255
                                poll_report, states['caught_poll'] = trace_recovery(prg, poll_input, control_input=(0,0))
                                if poll_report['boundary'] != 'control_poll_wait': raise RuntimeError('Unexpected caught keeper neutral poll')
                                menu_input = bytearray(states['caught_poll']); next_rng = next_random(next_rng)
                                menu_input[0xe2], menu_input[0xe3] = next_rng & 255, (next_rng >> 8) & 255
                                menu_report, states['caught_menu'] = trace_recovery(prg, menu_input, control_input=(64,64))
                                if menu_report['boundary'] != 'action_menu_pending': raise RuntimeError('Unexpected caught keeper menu boundary')
                                open_input = bytearray(states['caught_menu']); next_rng = next_random(next_rng)
                                open_input[0xe2], open_input[0xe3] = next_rng & 255, (next_rng >> 8) & 255
                                action_report, states['caught_open'] = trace_menu(prg, open_input, allow_keeper=True)
                                if action_report['boundary'] != 'human_action_menu': raise RuntimeError('Unexpected caught keeper menu open')
                                selected_input = bytearray(states['caught_open']); next_rng = next_random(next_rng)
                                selected_input[0xe2], selected_input[0xe3] = next_rng & 255, (next_rng >> 8) & 255
                                selected_report, states['caught_selected'] = trace_menu(prg, selected_input, mode='poll', pressed=2, allow_keeper=True)
                                if selected_report['boundary'] != 'human_action_menu' or selected_report['action'] != 1: raise RuntimeError('Unexpected caught keeper pass selection')
                                confirmed_input = bytearray(states['caught_selected']); next_rng = next_random(next_rng)
                                confirmed_input[0xe2], confirmed_input[0xe3] = next_rng & 255, (next_rng >> 8) & 255
                                confirmed_report, states['caught_confirmed'] = trace_menu(prg, confirmed_input, mode='poll', pressed=128, allow_keeper=True)
                                if confirmed_report['boundary'] != 'attack_choice_pending' or confirmed_report['action'] != 1: raise RuntimeError('Unexpected caught keeper pass confirmation')
                                target_input = bytearray(states['caught_open']); target_input[0xe2],target_input[0xe3] = next_rng & 255,(next_rng >> 8) & 255
                                x = target_input[0x305] | (target_input[0x306] << 8); y = target_input[0x307] | (target_input[0x308] << 8)
                                states['caught_target'] = original_target(prg,target_input,x,y,True,allow_keeper=True,return_ram=True)
                                if args.keeper_action_survey or args.keeper_action_one:
                                    action_report, opened = trace_menu(prg, states['caught_menu'], allow_keeper=True)
                                    recovery_counts['keeper-menu:' + action_report['boundary']] += 1
                                    posx = states['caught_open'][0x305] | (states['caught_open'][0x306] << 8)
                                    posy = states['caught_open'][0x307] | (states['caught_open'][0x308] << 8)
                                    target = original_target(prg, states['caught_open'], posx, posy, True, allow_keeper=True)
                                    recovery_counts['keeper-ball:' + ('same' if target[0:2] == (states['caught_open'][0x635], states['caught_open'][0x637]) else 'different')] += 1
                                    for mask in (0,1,2,4,8,64,128):
                                        poll, _ = trace_menu(prg, opened, mode='poll', pressed=mask, allow_keeper=True)
                                        recovery_counts[f"keeper-poll:{mask}:{poll['boundary']}:action{poll['action']}"] += 1
                                    if args.keeper_action_one:
                                        print('keeper position, ball and original pass target:', hex(posx), hex(posy), states['caught_open'][0x635], states['caught_open'][0x637], target)
                                        print('keeper action survey:', dict(sorted(recovery_counts.items())))
                                        return
                                if args.catch_survey:
                                    menu_report, _ = trace_recovery(prg, poll_input, control_input=(64,64))
                                    recovery_counts['catch-button:' + menu_report['boundary']] += 1
                                recovery_counts['catch:' + report['boundary']] += 1
                                for address in range(0x1a, 0x800):
                                    if states['goal'][address] != caught_ready[address]: catch_changes[address] += 1
                            states['recovered'] = states['tail'] = states['compared']
                            if result == 3 and states['compared'][0x43d] == 1:
                                report, states['recovered'] = trace_recovery(prg, states['goal'], skip_jitter=True)
                                if report['boundary'] != 'recovered_owner_handoff': raise RuntimeError('Unexpected punch recovery boundary')
                                recovery_counts[report['boundary']] += 1
                                tail_report, states['tail'] = trace_recovery(prg, states['recovered'], continue_handoff=True)
                                if tail_report['boundary'] not in ('keeper_area_interaction', 'player_control_handoff'): raise RuntimeError('Unexpected punch recovery tail')
                                recovery_counts['tail:' + tail_report['boundary']] += 1
                            states['restart'] = observe_keeper_entry(prg, states['goal'], 0x85bf, {0x85dd})[0] if states['compared'][0x612] == 0 else states['compared']
                            if states['compared'][0x612] == 0: states.update(kickoff_trace(prg, states['restart']))
                            else: states.update({name: states['compared'] for name in ('formation', 'receiver', 'ball', 'handoff', 'control')})
                            counts[(pressed, states['selected'][0x43d], states['compared'][0x612])] += 1
                            if not args.original_only:
                                source.write_bytes(export_session(raw)); names = ('entry', 'open', 'selected', 'confirmed', 'compared', 'goal', 'restart', 'formation', 'receiver', 'ball', 'handoff', 'control', 'recovered', 'tail', 'caught_ready', 'caught_poll', 'caught_menu', 'caught_open', 'caught_selected', 'caught_confirmed', 'caught_target')
                                records.write_bytes(b''.join(states[name] for name in names))
                                for name in names:
                                    normalized = bytearray(states[name]); normalized[0x600:0x606] = bytes(6)
                                    (temp / (name + '.cmjsession')).write_bytes(export_session(normalized))
                                run = subprocess.run([str(args.exe.resolve()), '--compare-human-keeper-menu', str(source), str(records), str(args.pack.resolve()), str(pressed), str(menu_seed), temporary], capture_output=True, text=True)
                                if run.returncode: raise RuntimeError(f'fixture={fixture} x={x} stamina={keeper_stamina} pressed={pressed} power={power} seed={menu_seed}: {run.stderr}')
    print(f'Constructed original human goalkeeper menu: {dict(sorted(counts.items()))}')
    if args.save_survey: print(f'Original home punch recovery: {dict(sorted(recovery_counts.items()))}')
    if args.catch_survey: print(f'Caught keeper setup changed RAM addresses: {[(hex(a), n) for a,n in sorted(catch_changes.items())]}')


if __name__ == '__main__': main()

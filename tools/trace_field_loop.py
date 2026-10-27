#!/usr/bin/env python3
"""Observe original field/shot calculations with explicit host frame acknowledgements."""
import argparse
import hashlib
import json
import sys
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def word(memory, address):
    return memory[address] | memory[address + 1] << 8


def trace_loop(prg, internal_ram, cartridge_ram=None, max_instructions=2_000_000,
               max_frames=10000, until='field'):
    watched = {0x61C, 0x61D, 0x612, 0x616, 0x617, 0xE2, 0xE3,
               *range(0x5F7, 0x600), *(0x300 + 12 * slot + 10 for slot in range(22))}
    memory = ObservedCartridge(prg, internal_ram, cartridge_ram, watched)
    if memory[0x22] & 64:
        raise ValueError('Snapshot PRG mode bit6 at $22 must be clear')
    if until not in ('field', 'keeper'):
        raise ValueError('Expected field or keeper endpoint')
    minimum = 0 if until == 'keeper' else 1
    if not minimum <= memory[0x600] <= 5:
        raise ValueError(f'This entry requires {minimum}..5 selected defenders')
    if memory[0x5FB] not in (0, 11) or memory[0x43B] >= 5:
        raise ValueError('Expected possession side0/11 and attack action0..4')
    if max_instructions < 1 or max_frames < 1:
        raise ValueError('Trace limits must be positive')
    if until == 'keeper' and memory[0x43B] != 0:
        raise ValueError('Keeper trace mode supports attack action0 only')
    for index in range(memory[0x600]):
        slot, action = memory[0x601 + index], memory[0x60B + index]
        if slot > 21 or action > 6:
            raise ValueError('Invalid selected defender/action')
        if until == 'keeper' and (slot in (0, 11) or action not in (2, 3)):
            raise ValueError('Keeper trace currently supports selected field actions2/3 only')

    # A constructed scheduler entry; these host writes are not original instructions.
    memory.ram[:0x1A] = bytes(0x1A)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27),
                           (5, 1), (6, 0x50), (7, 26), (8, 27),
                           (0x191, 0x85), (0x192, 0xEB), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory)
    entry = 0x829D if until == 'keeper' else 0x82F4
    cpu.pc, cpu.sp, cpu.p = entry, 0x50, cpu.UNUSED
    initial_power = word(memory, 0x61C)
    host_frames = []
    comparisons = []
    checkpoints = []
    attacker = {}
    keeper = {}
    for steps in range(max_instructions):
        if cpu.pc < 0x8000:
            raise RuntimeError(f'Unexpected execution outside original PRG: ${cpu.pc:04X}')
        bank = memory.bank_at(cpu.pc)
        boundary = None
        if bank == 26:
            if cpu.pc == 0x83F5 and (until == 'field' or memory[0x612] >= 2):
                boundary = 'field_dispatch'
            elif until == 'keeper' and cpu.pc == 0x856B:
                boundary = 'keeper_comparison'
                keeper.update({'adjusted_base': word(memory, 0x32), 'strength': memory[0x71],
                               'ratio': memory[0x3A], 'score': cpu.a,
                               'raw_result': cpu.y, 'subtype': memory[0x43E]})
            elif until == 'keeper' and cpu.pc in (0x8BE5, 0x88F3):
                boundary = 'loose_ball' if cpu.pc == 0x8BE5 else 'other_continuation'
        if boundary is not None:
            memory.instruction = None
            return {'format': 'captain-selected-defender-loop-trace-v1',
                    'mode': until, 'scope': 'Constructed CPU/scheduler trace with host frame acknowledgements; '
                             'no NMI, rendering, sound or hardware/frame timing emulation.',
                    'internal_ram_sha256': hashlib.sha256(internal_ram).hexdigest(),
                    'cartridge_ram_sha256': hashlib.sha256(cartridge_ram).hexdigest() if cartridge_ram is not None else None,
                    'entry': entry, 'exit': cpu.pc, 'boundary': boundary,
                    'original_instructions': steps,
                    'host_setup': ['Replace scheduler slots/flag $00..$19 with constructed task state.',
                                   f'Foreground task5 at ${entry:04X}/SP$50; background task1 at $EB86/SP$90.',
                                   'Other tasks inactive until original code starts them; banks26/27, '
                                   'bank/select shadows consistent with $22.',
                                   'Snapshot scratch/stack bytes outside this setup are explicit input.'],
                    'host_frames': host_frames, 'comparisons': comparisons,
                    'attacker': attacker, 'keeper': keeper,
                    'checkpoints': checkpoints, 'original_writes': memory.writes,
                    'initial_power': initial_power, 'final_power': word(memory, 0x61C),
                    'result_code': memory[0x612], 'defender_index': memory[0x616],
                    'active_defenders': memory[0x617], 'elapsed': memory[0x5FF],
                    'clock': word(memory, 0x5F7), 'random_bytes': [memory[0xE2], memory[0xE3]],
                    'statuses': [memory[0x300 + 12 * slot + 10] for slot in range(22)],
                    'bank_writes': memory.bank_writes}
        if bank == 26 and until == 'keeper':
            if cpu.pc == 0x8F72:
                slot = memory[0x441]
                if slot > 21:
                    raise ValueError('Invalid attacking player ID')
                record = 0x300 + 12 * slot
                attacker['input'] = {'slot': slot, 'type': memory[record],
                                     'stamina': word(memory, record + 1), 'level': memory[record + 3],
                                     'action': memory[0x43B], 'situation': memory[0x44E],
                                     'subtype': memory[0x43C], 'side': memory[0x5FB],
                                     'defenders': memory[0x600], 'random': [memory[0xE2], memory[0xE3]]}
            elif cpu.pc == 0x8F7A:
                attacker.update({'loaded_base': word(memory, 0x32), 'effect': memory[0x444],
                                 'loaded_cost': word(memory, 0x43F), 'move_flags': memory[0x443]})
            elif cpu.pc == 0x8F9A:
                attacker.update({'stamina_after': word(memory, 0x300 + 12 * memory[0x441] + 1),
                                 'cost': word(memory, 0x43F), 'weakened': bool(memory[0x3A] & 128),
                                 'energy_carry': bool(cpu.p & 1)})
            elif cpu.pc == 0x8FDD:
                attacker['multiplier'] = word(memory, 0x67)
            elif cpu.pc == 0x8FF2:
                attacker.update({'adjusted_base': word(memory, 0x32),
                                 'power': word(memory, 0x61C), 'subtype': memory[0x43C]})
            elif cpu.pc == 0x852F:
                slot = memory[0x442]
                record = 0x300 + 12 * slot
                keeper['input'] = {'slot': slot, 'type': memory[record],
                                   'stamina': word(memory, record + 1), 'level': memory[record + 3],
                                   'action': memory[0x43D], 'subtype': memory[0x43E],
                                   'random': [memory[0xE2], memory[0xE3]],
                                   'context': memory[0x621], 'elapsed': memory[0x62B],
                                   'timer': memory[record + 5], 'penalty': memory[record + 7]}
            elif cpu.pc == 0x8537:
                keeper.update({'loaded_base': word(memory, 0x32), 'effect': memory[0x445],
                               'stamina_after': word(memory, 0x300 + 12 * memory[0x442] + 1),
                               'cost': word(memory, 0x43F), 'weakened': bool(memory[0x3A] & 128)})
            elif cpu.pc == 0x8F4F:
                keeper['adjustment'] = cpu.a
        if bank == 26 and cpu.pc == 0x8306:
            checkpoints.append({'kind': 'defender_entry', 'index': memory[0x616],
                                'power': word(memory, 0x61C)})
        if bank == 26 and cpu.pc == 0x837F:
            comparisons.append({'index': memory[0x616], 'defender': memory[0x442],
                                'action': memory[0x43D], 'subtype': memory[0x43E],
                                'base': word(memory, 0x32), 'strength': memory[0x71],
                                'candidate_loss': memory[0x619], 'score': cpu.a,
                                'raw_result': cpu.y, 'power': word(memory, 0x61C)})
        if cpu.pc == 0xCAAE:
            memory.instruction = None
            if len(host_frames) >= max_frames:
                raise RuntimeError('Host frame acknowledgement limit reached')
            values = {address: memory[address] for address in (0x1B, 0x515, 0x1C, 0x1E)}
            memory[0x1B] |= 128
            memory[0x515] = 0
            memory[0x1C], memory[0x1E] = 128, 128
            host_frames.append({'instruction': steps,
                                'writes': [{'address': address, 'before': before,
                                            'after': memory[address]} for address, before in values.items()]})
        memory.instruction = (bank, cpu.pc)
        try:
            cpu.step()
        except RuntimeError as error:
            raise RuntimeError(f'{error}; instruction bank{bank} PC=${memory.instruction[1]:04X}') from error
    raise RuntimeError(f'Original instruction limit reached at ${cpu.pc:04X}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--ram', required=True, type=Path, help='Explicit 2048-byte internal RAM snapshot')
    parser.add_argument('--cartridge-ram', type=Path)
    parser.add_argument('--until', choices=('field', 'keeper'), default='field',
                        help='Field dispatch, or action0 attacker entry through keeper comparison')
    args = parser.parse_args()
    try:
        rom = args.rom.read_bytes()
        if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
            parser.error('ROM does not match verified Japanese reference')
        result = trace_loop(rom[16:262160], args.ram.read_bytes(),
                            args.cartridge_ram.read_bytes() if args.cartridge_ram else None,
                            until=args.until)
    except (OSError, ValueError, RuntimeError) as error:
        parser.exit(1, f'Trace stopped: {error}\n')
    json.dump(result, sys.stdout, indent=2)
    sys.stdout.write('\n')


if __name__ == '__main__':
    main()

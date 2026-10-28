#!/usr/bin/env python3
"""Observe private original recovery phases with disclosed cooperative host acknowledgements."""
import argparse
import hashlib
import json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def keeper_area(x, y):
    return 96 <= y < 160 and (x < 80 or x >= 176)


def story_gate(memory):
    owner = memory[0x441]
    return (memory[0x446] == 4 and memory[0x5FB] == 0
            and memory[0x300 + 12 * owner] != 1
            and memory[0x306 + 12 * owner] >= 128)


def trace_recovery(prg, ram, continue_handoff=False, max_instructions=2_000_000, max_frames=10000):
    if len(ram) != 2048 or ram[0x22] & 64 or ram[0x5FB] not in (0, 11) or ram[0x441] > 21:
        raise ValueError('Expected2048-byte supported field checkpoint')
    if ram[0x600] != 0:
        raise ValueError('Connected terminal recovery requires zero selected defenders')
    if max_instructions < 1 or max_frames < 1:
        raise ValueError('Observation bounds must be positive')
    memory = ObservedCartridge(prg, ram, None, set())
    memory.ram[:0x1A] = bytes(0x1A)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27),
                           (5, 1), (6, 0x50), (7, 26), (8, 27),
                           (0x191, 0x85), (0x192, 0xEB), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = (0x8C31 if continue_handoff else 0x8BDF), 0x50, cpu.UNUSED
    entry = cpu.pc
    frames, checkpoints = [], []
    for steps in range(max_instructions):
        if cpu.pc < 0x8000:
            raise RuntimeError(f'Unexpected execution outside original PRG: ${cpu.pc:04X}')
        bank = memory.bank_at(cpu.pc)
        boundary = None
        if bank == 26:
            if not continue_handoff and cpu.pc == 0x8CA4:
                if not (48 <= memory[0x635] < 208 and 80 <= memory[0x637] < 176):
                    boundary = 'boundary_restart'
            if not continue_handoff and cpu.pc == 0x8C31:
                boundary = 'recovered_owner_handoff'
            if continue_handoff and cpu.pc == 0x8B4A and keeper_area(memory[0x635], memory[0x637]):
                boundary = 'keeper_area_interaction'
            if continue_handoff and cpu.pc == 0x8E86 and story_gate(memory):
                boundary = 'story_transition'
            if cpu.pc in (0x8BE2, 0x8BE5, 0x8C12):
                checkpoints.append({'pc': cpu.pc, 'instruction': steps, 'elapsed': memory[0x5FF],
                    'random': [memory[0xE2], memory[0xE3]], 'ball': [memory[0x635], memory[0x637]],
                    'owner': memory[0x441], 'side': memory[0x5FB]})
        if continue_handoff and bank == 30 and cpu.pc == 0xC60F:
            boundary = 'player_control_handoff'
        if boundary:
            return {'format': 'captain-recovery-phase-observation-v1', 'boundary': boundary,
                'entry': entry, 'exit': cpu.pc, 'original_instructions': steps,
                'internal_ram_sha256': hashlib.sha256(ram).hexdigest(),
                'scope': 'Constructed original CPU/task observation; no skipped calls, NMI, sound, PPU or hardware timing.',
                'host_setup': 'Foreground task5/SP50, background task1/EB86/SP90, banks26/27; scheduler slots replaced.',
                'host_policy': 'Each CAAE frame: set1B.bit7, clear515, set held/edge buttons129, consume one498 descriptor.',
                'host_frames': frames, 'checkpoints': checkpoints,
                'ball': [memory[0x635], memory[0x637]], 'owner': memory[0x441],
                'side': memory[0x5FB], 'elapsed': memory[0x5FF],
                'random': [memory[0xE2], memory[0xE3]]}, bytes(memory.ram[:2048])
        if cpu.pc == 0xCAAE:
            if len(frames) >= max_frames:
                raise RuntimeError('Host frame acknowledgement bound reached')
            memory.instruction = None
            memory[0x1B] |= 128
            memory[0x515] = 0
            memory[0x1C], memory[0x1E] = 129, 129
            if memory[0x498]: memory[0x498] -= 1
            frames.append(steps)
        memory.instruction = (bank, cpu.pc)
        cpu.step()
    raise RuntimeError(f'Original instruction bound reached at ${cpu.pc:04X}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('ram', type=Path)
    parser.add_argument('report', type=Path)
    parser.add_argument('output_ram', type=Path)
    parser.add_argument('--continue-handoff', action='store_true')
    parser.add_argument('--max-instructions', type=int, default=2_000_000)
    parser.add_argument('--max-frames', type=int, default=10000)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('ROM does not match the verified Japanese reference')
    report, final = trace_recovery(rom[16:262160], args.ram.read_bytes(), args.continue_handoff,
                                   args.max_instructions, args.max_frames)
    with args.output_ram.open('xb') as output: output.write(final)
    with args.report.open('x') as output: json.dump(report, output, indent=2)
    print(f"Observed {report['boundary']} at ${report['exit']:04X}, {report['original_instructions']} original instructions.")


if __name__ == '__main__':
    main()

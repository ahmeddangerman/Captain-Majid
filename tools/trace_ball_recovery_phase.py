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


def trace_recovery(prg, ram, continue_handoff=False, max_instructions=2_000_000, max_frames=10000,
                   prepare_control=False, control_input=None):
    if len(ram) != 2048 or ram[0x22] & 64 or ram[0x5FB] not in (0, 11) or ram[0x441] > 21:
        raise ValueError('Expected2048-byte supported field checkpoint')
    if sum((bool(prepare_control), bool(continue_handoff), control_input is not None)) > 1:
        raise ValueError('Choose one observation mode')
    if control_input is not None and (len(control_input) != 2 or any(not isinstance(v, int) or not 0 <= v <= 255 for v in control_input)):
        raise ValueError('Control input requires held/pressed byte masks')
    if ram[0x600] != 0 and not prepare_control:
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
    cpu.pc, cpu.sp, cpu.p = (0xE14A if control_input is not None else 0xE0DF if prepare_control else 0x8C31 if continue_handoff else 0x8BDF), 0x50, cpu.UNUSED
    held, pressed = control_input if control_input is not None else (129, 129)
    if control_input is not None: memory[0x1C], memory[0x1E] = held, pressed
    entry = cpu.pc
    frames, checkpoints, presentation = [], [], []
    for steps in range(max_instructions):
        if cpu.pc < 0x8000:
            raise RuntimeError(f'Unexpected execution outside original PRG: ${cpu.pc:04X}')
        bank = memory.bank_at(cpu.pc)
        boundary = None
        if prepare_control and bank == 31 and cpu.pc == 0xE145:
            boundary = 'player_control_ready'
        if control_input is not None and bank == 31:
            boundary = {0xE145: 'control_poll_wait', 0xE1E7: 'control_clock_pending',
                        0xE38A: 'action_menu_pending', 0xE19B: 'ai_cell_decision'}.get(cpu.pc)
        if (prepare_control or control_input is not None) and bank == 31:
            if cpu.pc in (0xE0E1, 0xE0E6, 0xE26E, 0xE273, 0xE279):
                presentation.append({'kind': 0, 'id': cpu.a})
            elif cpu.pc == 0xE235:
                presentation.append({'kind': 1, 'id': cpu.a})
            elif cpu.pc == 0xE247:
                presentation.append({'kind': 2, 'id': 0})
        if bank == 26 and not prepare_control and control_input is None:
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
                'host_policy': f'Each CAAE frame: set1B.bit7, clear515, set held{held}/pressed{pressed}, consume one498 descriptor.',
                'host_frames': frames, 'checkpoints': checkpoints,
                'presentation_requests': presentation,
                'ball': [memory[0x635], memory[0x637]], 'owner': memory[0x441],
                'side': memory[0x5FB], 'elapsed': memory[0x5FF],
                'random': [memory[0xE2], memory[0xE3]]}, bytes(memory.ram[:2048])
        if cpu.pc == 0xCAAE:
            if len(frames) >= max_frames:
                raise RuntimeError('Host frame acknowledgement bound reached')
            memory.instruction = None
            memory[0x1B] |= 128
            memory[0x515] = 0
            memory[0x1C], memory[0x1E] = held, pressed
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
    modes = parser.add_mutually_exclusive_group()
    modes.add_argument('--continue-handoff', action='store_true')
    modes.add_argument('--prepare-control', action='store_true')
    modes.add_argument('--poll-control', nargs=2, type=int, metavar=('HELD', 'PRESSED'))
    parser.add_argument('--max-instructions', type=int, default=2_000_000)
    parser.add_argument('--max-frames', type=int, default=10000)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('ROM does not match the verified Japanese reference')
    report, final = trace_recovery(rom[16:262160], args.ram.read_bytes(), args.continue_handoff,
                                   args.max_instructions, args.max_frames, args.prepare_control, args.poll_control)
    with args.output_ram.open('xb') as output: output.write(final)
    with args.report.open('x') as output: json.dump(report, output, indent=2)
    print(f"Observed {report['boundary']} at ${report['exit']:04X}, {report['original_instructions']} original instructions.")


if __name__ == '__main__':
    main()

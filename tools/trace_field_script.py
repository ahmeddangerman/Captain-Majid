#!/usr/bin/env python3
"""Observe original field script parsing from private RAM; no rendering/frame emulation."""
import argparse
import hashlib
import json
import sys
from pathlib import Path

from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


class ObservedCartridge(ReferenceCartridge):
    def __init__(self, prg, internal_ram, cartridge_ram, watched):
        super().__init__(prg)
        if len(internal_ram) != 2048:
            raise ValueError('Internal RAM snapshot must be exactly 2048 bytes')
        if cartridge_ram is not None and len(cartridge_ram) != 8192:
            raise ValueError('Cartridge RAM snapshot must be exactly 8192 bytes')
        self.ram[:2048] = internal_ram
        if cartridge_ram is not None:
            self.ram[0x6000:0x8000] = cartridge_ram
        self.has_cartridge_ram = cartridge_ram is not None
        self.watched = set(watched)
        self.instruction = None
        self.writes = []

    def __getitem__(self, address):
        if isinstance(address, int) and 0x6000 <= address < 0x8000 and not self.has_cartridge_ram:
            raise RuntimeError('Original parser accessed cartridge RAM without a snapshot')
        return super().__getitem__(address)

    def __setitem__(self, address, value):
        if isinstance(address, int):
            if 0x6000 <= address < 0x8000 and not self.has_cartridge_ram:
                raise RuntimeError('Original parser wrote cartridge RAM without a snapshot')
            canonical = address & 2047 if address < 0x2000 else address
            if self.instruction is not None and canonical in self.watched:
                bank, pc = self.instruction
                self.writes.append({'bank': bank, 'pc': pc, 'address': canonical,
                                    'before': self[address], 'after': value})
        super().__setitem__(address, value)


def trace_parser(prg, internal_ram, cartridge_ram, script, watched, max_lines=256,
                 max_instructions=20000):
    if script not in (5, 6, 7):
        raise ValueError('Expected field script5,6 or7')
    if max_lines < 1 or max_instructions < 1:
        raise ValueError('Trace limits must be positive')
    memory = ObservedCartridge(prg, internal_ram, cartridge_ram, watched)
    if memory[0x22] & 64:
        raise ValueError('Snapshot PRG mode bit6 at $22 must be clear for this parser entry')
    cpu = MPU(memory=memory)
    # Host entry reproduces EC15..EC31 setup but not its scheduler/presentation context.
    memory.registers[6:8] = [16, 17]
    memory[0x24], memory[0x25] = 16, 17
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory[0x516], memory[0x518] = 1, script
    host_actions = ['Map PRG banks16/17 and set bank shadows $24/$25; '
                    'set mapper select/shadow $23 to $22|7 with PRG mode bit6 clear.',
                    'Set pending script ID $0518 and script state $0516=1.',
                    'Use a synthetic return at $0200 and stack $01FD for each parser call.',
                    'CPU entry registers A/X/Y=0 and status UNUSED for each call; no frame interrupts.']
    total_steps = 0

    def call(entry):
        nonlocal total_steps
        if memory.bank_at(0x8000) != 16 or memory.bank_at(0xA000) != 17:
            raise RuntimeError('Parser entry no longer maps expected banks16/17')
        memory.instruction = None
        memory[0x1FE], memory[0x1FF] = 255, 1
        cpu.pc, cpu.sp, cpu.a, cpu.x, cpu.y, cpu.p = entry, 0xFD, 0, 0, 0, cpu.UNUSED
        for _ in range(max_instructions):
            if cpu.pc == 0x200:
                memory.instruction = None
                return
            if cpu.pc < 0x8000:
                raise RuntimeError(f'Unexpected execution outside original PRG: ${cpu.pc:04X}')
            memory.instruction = (memory.bank_at(cpu.pc), cpu.pc)
            try:
                cpu.step()
            except RuntimeError as error:
                raise RuntimeError(f'{error}; bank{memory.bank_at(cpu.pc)} PC=${cpu.pc:04X}') from error
            total_steps += 1
        raise RuntimeError(f'Parser instruction limit reached at ${cpu.pc:04X}')

    power_before = memory[0x61C] | memory[0x61D] << 8
    call(0x8000)
    memory[0x522], memory[0x539] = 0, 0
    host_actions.append('Clear parser return depth $0522 and presentation field $0539.')
    events = []
    for _ in range(max_lines):
        call(0x8003)
        if memory[0x516] & 8:
            break
        if not memory[0x516] & 64:
            raise RuntimeError('Parser returned without an end or presentation request')
        events.append({'parser_pointer': memory[0x5D] | memory[0x5E] << 8,
                       'request': memory[0x523], 'presentation': memory[0x524],
                       'argument_first': memory[0x528], 'argument_second': memory[0x529],
                       'power': memory[0x61C] | memory[0x61D] << 8})
        # Acknowledge the request; drawing/audio/input effects are deliberately absent.
        memory[0x516] &= ~64
    else:
        raise RuntimeError('Parser line limit reached without script termination')
    host_actions.append(f'Acknowledge {len(events)} presentation requests by clearing $0516 bit6; '
                        'rendering, audio and input callbacks are omitted.')
    return {'format': 'captain-field-script-parser-trace-v1', 'script': script,
            'scope': 'Original parser only; host acknowledgements omit presentation and frame effects.',
            'internal_ram_sha256': hashlib.sha256(internal_ram).hexdigest(),
            'cartridge_ram_sha256': hashlib.sha256(cartridge_ram).hexdigest() if cartridge_ram is not None else None,
            'host_actions': host_actions, 'original_instructions': total_steps,
            'watched_addresses': sorted(memory.watched), 'original_writes': memory.writes,
            'power_before': power_before, 'power_after': memory[0x61C] | memory[0x61D] << 8,
            'events': events, 'bank_writes': memory.bank_writes,
            'final_script_state': memory[0x516]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--ram', required=True, type=Path, help='Explicit 2048-byte internal RAM snapshot')
    parser.add_argument('--cartridge-ram', type=Path, help='Optional 8192-byte cartridge RAM snapshot')
    parser.add_argument('--script', required=True, type=int, choices=(5, 6, 7))
    parser.add_argument('--watch', action='append', type=lambda value: int(value, 0), default=[],
                        help='Additional canonical RAM address, e.g. 0x612 (power bytes always watched)')
    args = parser.parse_args()
    if any(not (0 <= address < 2048 or 0x6000 <= address < 0x8000) for address in args.watch):
        parser.error('Watch addresses must be canonical internal or cartridge RAM')
    try:
        rom = args.rom.read_bytes()
        if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
            parser.error('ROM does not match verified Japanese reference')
        result = trace_parser(rom[16:262160], args.ram.read_bytes(),
                              args.cartridge_ram.read_bytes() if args.cartridge_ram else None,
                              args.script, {*args.watch, 0x61C, 0x61D})
    except (OSError, ValueError, RuntimeError) as error:
        parser.exit(1, f'Trace stopped: {error}\n')
    json.dump(result, sys.stdout, indent=2)
    sys.stdout.write('\n')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Bounded shared restart UI logic, with explicit presentation acknowledgements."""
import argparse
import hashlib
import json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def trace_options(prg, ram, *, poll=False, held=0, pressed=0, cursor=4):
    if len(ram) != 2048 or ram[0x22] & 64:
        raise ValueError('Expected2048-byte mode0 RAM')
    if not all(isinstance(v, int) and 0 <= v <= 255 for v in (held, pressed, cursor)) or cursor not in (4, 68):
        raise ValueError('Expected byte inputs and restart cursor4/68')
    memory = ObservedCartridge(prg, ram, None, set())
    memory.registers[6:8] = [26, 27]
    memory[0x24], memory[0x25] = 26, 27
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory[0x1C], memory[0x1E] = held, pressed
    if poll:
        memory[0x624] = cursor
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = (0x98AC if poll else 0x9880), 0xFD, cpu.UNUSED
    memory[0x1FE], memory[0x1FF] = 255, 1  # synthetic return0200
    acknowledgements = []
    for step in range(2000):
        if cpu.pc in (0x98A7, 0x98D5, 0x200):
            return {'entry': 0x98AC if poll else 0x9880, 'exit': cpu.pc,
                    'instructions': step, 'cursor': memory[0x624],
                    'acknowledgements': acknowledgements,
                    'scope': 'Constructed entry after opening scripts; wait/overlay/display/sprite calls '
                             'are acknowledged without executing them. No frame tasks, scripts, rendering or audio.'}, bytes(memory.ram[:2048])
        if cpu.pc < 0x8000:
            raise RuntimeError('Unexpected restart menu execution outside PRG')
        if memory[cpu.pc] == 0x20:
            target = memory[cpu.pc+1] | (memory[cpu.pc+2] << 8)
            if target in (0xC515, 0xC52D, 0xC52A, 0x9D1B):
                acknowledgements.append({'pc': cpu.pc, 'target': target, 'a': cpu.a})
                cpu.pc += 3
                continue
        cpu.step()
    raise RuntimeError('Restart menu instruction bound reached')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('rom', type=Path); p.add_argument('ram', type=Path)
    p.add_argument('--poll', action='store_true')
    p.add_argument('--held', type=int, default=0); p.add_argument('--pressed', type=int, default=0)
    p.add_argument('--cursor', type=int, default=4)
    p.add_argument('--report', type=Path, required=True); p.add_argument('--output-ram', type=Path, required=True)
    a = p.parse_args(); rom = a.rom.read_bytes()
    if rom[:4] != b'NES\x1a' or rom[6] & 4 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved private Japanese source')
    report, output = trace_options(rom[16:16+rom[4]*16384], a.ram.read_bytes(),
                                  poll=a.poll, held=a.held, pressed=a.pressed, cursor=a.cursor)
    a.report.write_text(json.dumps(report, indent=2)+'\n'); a.output_ram.write_bytes(output)


if __name__ == '__main__':
    main()

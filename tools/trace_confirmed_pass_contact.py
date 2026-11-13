"""Observe a constructed human pass from menu entry, preserving its call stack.

Private RAM, ROM and output directory are supplied by the caller. Cooperative
display acknowledgements and explicit random samples are not live NES timing.
"""
import argparse
import hashlib
import json
from pathlib import Path

from py65.devices.mpu6502 import MPU
from reference_identity import PAYLOAD_SHA256
from trace_field_script import ObservedCartridge


def observe(prg, ram, keys, first, second, output):
    memory = ObservedCartridge(prg, ram, None, set())
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0,5),(1,255),(2,0x90),(3,26),(4,27),(5,1),
                           (6,0x50),(7,26),(8,27),(0x191,0x85),(0x192,0xeb),
                           (0x24,26),(0x25,27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26,27]
    cpu = MPU(memory=memory)
    # D70C pops the menu's return addresses. Starting at D80A with SP50
    # discards that caller and can silently enter a different gameplay path.
    cpu.pc, cpu.sp, cpu.p = 0xe4be, 0x50, cpu.UNUSED
    stage = entries = index = frames = 0
    captured = {}
    boundaries = {(30,0xd84b): 'confirmed', (26,0x804f): 'prepared',
                  (26,0x80b6): 'defender-prepared', (26,0x80d9): 'compared',
                  (31,0xe0df): 'handoff', (31,0xe145): 'control'}
    for instruction in range(2_000_000):
        pc = cpu.pc
        bank = memory.bank_at(pc)
        if bank == 30 and pc == 0xd5e5:
            stage += 1
        if bank == 30 and pc == 0xd80a:
            if entries:
                index += 1
            entries += 1
            if index >= len(keys):
                raise RuntimeError('Pass target did not confirm with supplied inputs')
        if bank == 30 and pc == 0xd84b:
            index = len(keys)
        if bank == 26 and pc in (0x804c,0x80b6):
            memory[0xe2], memory[0xe3] = first, second
        label = boundaries.get((bank,pc))
        if label and label not in captured:
            (output / (label + '.ram')).write_bytes(bytes(memory.ram[:2048]))
            captured[label] = dict(instruction=instruction, frames=frames,
                bank=bank, pc=pc, sp=cpu.sp, owner=memory[0x441],
                side=memory[0x5fb], x=memory[0x635], y=memory[0x637],
                power=memory[0x61c] | memory[0x61d] << 8,
                result=memory[0x612], strength=memory[0x71])
            if label == 'control':
                if set(captured) != set(boundaries.values()):
                    raise RuntimeError('Control reached without required initial contact boundaries')
                return captured
        if pc < 0x8000:
            raise RuntimeError(f'Original left PRG at ${pc:04X}')
        if pc == 0xcaae:
            held = (2 if stage == 1 else 128) if not entries else keys[index] if index < len(keys) else 0
            memory.instruction = None
            memory[0x1b] |= 128
            memory[0x515] = 0
            memory[0x1c] = memory[0x1e] = held
            if memory[0x498]:
                memory[0x498] -= 1
            frames += 1
        memory.instruction = (bank,pc)
        cpu.step()
    raise RuntimeError(f'Original instruction bound at ${cpu.pc:04X}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ram','output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--keys', type=int, nargs='+', required=True)
    parser.add_argument('--random', type=int, nargs=2, required=True)
    args = parser.parse_args()
    rom, ram = args.rom.read_bytes(), args.ram.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256 or len(ram) != 2048:
        parser.error('Expected verified Japanese ROM and 2048-byte RAM')
    if any(not 0 <= value <= 255 for value in args.keys + args.random):
        parser.error('Inputs must be bytes')
    if args.output.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Original observations must stay outside the checkout')
    args.output.mkdir(parents=True, exist_ok=False)
    result = observe(rom[16:262160], ram, args.keys, *args.random, args.output)
    report = dict(scope='Constructed menu replay; controlled random and display acknowledgements; no NMI/audio/rendering.',
                  seed_sha256=hashlib.sha256(ram).hexdigest(), keys=args.keys,
                  random=args.random, boundaries=result)
    (args.output / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps(result['control']))


if __name__ == '__main__':
    main()

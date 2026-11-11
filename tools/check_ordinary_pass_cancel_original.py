#!/usr/bin/env python3
"""Compare target cancellation and menu return using controlled original CPU runs."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256
from verify_observed_world_round import checkpoint


def observe_cancel(prg, ready, mask, move):
    _, raw = trace_recovery(prg, ready, control_input=(64, 0))
    memory = ObservedCartridge(prg, raw, None, set())
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1),
                           (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb),
                           (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7
    memory.select = memory[0x23]
    memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory)
    cpu.pc, cpu.sp, cpu.p = 0xe38a, 0x50, cpu.UNUSED
    stage, held, pressed, before = 0, 0, 0, None
    for _ in range(3000000):
        if cpu.pc < 0x8000:
            raise RuntimeError(f'Original cancellation left PRG at {cpu.pc:04x}')
        bank = memory.bank_at(cpu.pc)
        if bank == 30 and cpu.pc == 0xd5e5:
            if stage == 0:
                held, pressed, stage = 1, 1, 1
            elif stage == 1:
                held, pressed, stage = 128, 128, 2
            elif stage == 3:
                held, pressed, stage = 2, 2, 4
            elif stage == 4:
                held, pressed, stage = 128, 128, 5
            elif stage == 9:
                return before, bytes(memory.ram[:2048])
        if bank == 30 and cpu.pc == 0xd67c:
            if stage == 2:
                held, pressed, stage = 0, 0, 3
            elif stage == 5:
                held, pressed, stage = 0, 0, 6
        if bank == 30 and cpu.pc == 0xd80a:
            if stage == 6 and move:
                held, pressed, stage = 1, 0, 7
            elif stage in (6, 7):
                before = bytes(memory.ram[:2048])
                held, pressed, stage = mask, mask, 8
        if bank == 30 and cpu.pc == 0xd83e and stage == 8:
            held, pressed, stage = 0, 0, 9
        if cpu.pc == 0xcaae:
            memory.instruction = None
            memory[0x1b] |= 128
            memory[0x515] = 0
            memory[0x1c], memory[0x1e] = held, pressed
            if memory[0x498]:
                memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc)
        cpu.step()
    raise RuntimeError('Original cancellation instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'exe', 'private-temp'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Original RAM captures must stay outside the checkout')
    sources = sorted(args.ready_dir.glob('*-ready.bin'))
    if not sources:
        raise ValueError('No original ready snapshots found')
    count = 0
    with tempfile.TemporaryDirectory(prefix='pass-cancel-', dir=args.private_temp) as temp:
        temp = Path(temp)
        for source in sources:
            for move in (False, True):
                for mask in (64, 192):
                    before, after = observe_cancel(rom[16:262160], source.read_bytes(), mask, move)
                    entry, before_path, expected = temp/'entry.cmjsession', temp/'before.bin', temp/'expected.cmjsession'
                    entry.write_bytes(checkpoint(before, 19))
                    before_path.write_bytes(before)
                    expected.write_bytes(checkpoint(after, 18))
                    run = subprocess.run([str(args.exe.resolve()), '--compare-pass-cancel', str(entry),
                                          str(before_path), str(expected), str(mask)], capture_output=True, text=True)
                    if run.returncode:
                        raise RuntimeError(f'{source.name}, move={move}, mask={mask}: {run.stderr[-500:]}')
                    count += 1
    print(f'{count} controlled original pass cancellations matched represented menu state')


if __name__ == '__main__':
    main()

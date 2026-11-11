#!/usr/bin/env python3
"""Compare the multi-teammate chooser with constructed original CPU runs."""
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


def observe_chooser(prg, ready, count, mask, start_last=False):
    raw = bytearray(ready)
    # Construct teammates in the next target cell; keep all original data private.
    for player in range(6, 6 + count):
        raw[0x306 + 12 * player], raw[0x308 + 12 * player] = 116, 108
    _, raw = trace_recovery(prg, raw, control_input=(64, 0))
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
            raise RuntimeError(f'Original chooser left PRG at {cpu.pc:04x}')
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
        if bank == 30 and cpu.pc == 0xd67c:
            if stage == 2:
                held, pressed, stage = 0, 0, 3
            elif stage == 5:
                held, pressed, stage = 0, 0, 6
        if bank == 30 and cpu.pc == 0xd80a:
            if stage == 6:
                held, pressed, stage = 1, 0, 7
            elif stage == 7:
                held, pressed, stage = 0, 128, 8
            elif stage == 11:
                return before, bytes(memory.ram[:2048]), 'cancel'
        if bank == 30 and cpu.pc == 0xd868 and stage == 8:
            # Release the original target-confirm edge before the chooser waits.
            held = pressed = memory[0x1c] = memory[0x1e] = 0
            stage = 9
        if bank == 30 and cpu.pc == 0xd879:
            if stage == 9:
                if start_last:
                    held, pressed, stage = 0, 4, 10
                else:
                    before = bytes(memory.ram[:2048])
                    held, pressed, stage = 0, mask, 11
            elif stage == 10 and memory[0x625] == count - 1:
                before = bytes(memory.ram[:2048])
                held, pressed, stage = 0, mask, 11
            elif stage == 11:
                return before, bytes(memory.ram[:2048]), 'poll'
        if bank == 30 and cpu.pc == 0xd84b and stage == 11:
            return before, bytes(memory.ram[:2048]), 'confirm'
        if cpu.pc == 0xcaae:
            memory.instruction = None
            memory[0x1b] |= 128
            memory[0x515] = 0
            memory[0x1c], memory[0x1e] = held, pressed
            if memory[0x498]:
                memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc)
        cpu.step()
    raise RuntimeError('Original chooser instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'exe', 'private-temp'):
        parser.add_argument('--' + name, required=True, type=Path)
    parser.add_argument('--last-index', action='store_true', help='Start at the last teammate instead of the first')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Original RAM captures must stay outside the checkout')
    expected_sources = 2 if args.last_index else 4
    sources = sorted(args.ready_dir.glob('*-ready.bin'))[:expected_sources]
    if len(sources) != expected_sources:
        raise ValueError(f'Expected at least {expected_sources} original ready snapshots')
    total = 0
    with tempfile.TemporaryDirectory(prefix='pass-chooser-', dir=args.private_temp) as temp:
        temp = Path(temp)
        for source in sources:
            for count in (2, 3, 4):
                for mask in (0, 4, 8, 12, 64, 128, 132, 136, 192):
                    before, after, boundary = observe_chooser(rom[16:262160], source.read_bytes(), count, mask, args.last_index)
                    if before[0x430] != count:
                        raise RuntimeError(f'Expected {count} constructed teammates, got {before[0x430]}')
                    entry, before_path, after_path, expected = (temp/'entry.cmjsession', temp/'before.bin', temp/'after.bin', temp/'expected.cmjsession')
                    entry.write_bytes(checkpoint(before, 19))
                    before_path.write_bytes(before)
                    after_path.write_bytes(after)
                    expected.write_bytes(checkpoint(after, 19))
                    cmd = [str(args.exe.resolve()), '--compare-multi-chooser', str(entry), str(before_path),
                           str(after_path), str(expected), str(mask), boundary]
                    run = subprocess.run(cmd, capture_output=True, text=True)
                    if run.returncode:
                        raise RuntimeError(f'{source.name}, count={count}, mask={mask}, boundary={boundary}: {run.stderr[-500:]}')
                    total += 1
    print(f'{total} constructed original multi-teammate chooser polls matched')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare a constructed opponent-pass flight defender preparation and roll."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256
from verify_captured_control_chain import trace_data
from export_phase_session import export_session


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'trace-directory', 'pack', 'comparison', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--clock-pack', type=Path, help='CMJSHOTN pack to compare non-stopping results through their contact clock')
    parser.add_argument('--interception', action='store_true', help='Also compare result-three possession changes through field movement')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary files must remain outside the checkout')
    _, pcs, snapshots = trace_data(args.trace_directory)
    if pcs[24084] != '8845':
        raise ValueError('Unexpected captured opponent defender entry')
    original = snapshots(24084)
    if args.interception and not args.clock_pack:
        parser.error('--interception requires --clock-pack')
    cases = {}; clock_cases = 0; interception_cases = 0
    with tempfile.TemporaryDirectory(prefix='pass-flight-contact-', dir=args.private_temp) as temp:
        before_path, prep_path, after_path = (Path(temp) / name for name in ('before.bin', 'prep.bin', 'after.bin'))
        for kind in (1, 7, 23, 35, 70):
            for level in (0, 5):
                for first, second in ((122, 176), (1, 0), (90, 45), (255, 0), (0, 255)):
                    raw = bytearray(original)
                    defender = raw[0x601]
                    raw[0x300 + 12 * defender] = kind
                    raw[0x303 + 12 * defender] = level
                    raw[0xe2], raw[0xe3] = first, second
                    before_path.write_bytes(raw)
                    memory = ObservedCartridge(rom[16:262160], bytes(raw), None, set())
                    memory.select = memory[0x23]
                    memory.registers[6:8] = [26, 27]
                    cpu = MPU(memory=memory)
                    cpu.pc, cpu.sp, cpu.p, cpu.a, cpu.x, cpu.y = 0x8845, 0x4c, 0x31, 0x6c, 0x6c, 0xa4
                    found_prep = False
                    for _ in range(10000):
                        bank = memory.bank_at(cpu.pc)
                        if bank == 26 and cpu.pc == 0x886d:
                            prep_path.write_bytes(bytes(memory.ram[:2048])); found_prep = True
                        if bank == 26 and cpu.pc == 0x8870:
                            if not found_prep:
                                raise RuntimeError('Original comparison skipped preparation')
                            after_path.write_bytes(bytes(memory.ram[:2048]))
                            break
                        if cpu.pc < 0x8000:
                            raise RuntimeError(f'Original left PRG at {cpu.pc:04x}')
                        memory.instruction = (bank, cpu.pc)
                        cpu.step()
                    else:
                        raise RuntimeError('Original contact comparison exceeded instruction limit')
                    command = [str(args.exe.resolve()), '--compare-flight-contact', str(before_path), str(prep_path), str(after_path), str(args.pack.resolve()), str(cpu.a), str(cpu.y), str(args.comparison.resolve())]
                    result = subprocess.run(command, capture_output=True, text=True)
                    if result.returncode:
                        raise RuntimeError(f'profile={kind}, level={level}, random={(first, second)}: {result.stderr}')
                    cases[cpu.y] = cases.get(cpu.y, 0) + 1
                    if args.interception and cpu.y == 3 and memory[0x43e] == 0:
                        for _ in range(1500000):
                            bank = memory.bank_at(cpu.pc)
                            if bank == 30 and cpu.pc == 0xc60f:
                                break
                            if cpu.pc < 0x8000:
                                raise RuntimeError(f'Original interception left PRG at {cpu.pc:04x}')
                            if cpu.pc == 0xcaae:
                                memory.instruction = None
                                memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = 128
                                if memory[0x498]: memory[0x498] -= 1
                            memory.instruction = (bank, cpu.pc); cpu.step()
                        else:
                            raise RuntimeError('Original interception exceeded instruction limit')
                        before_session, after_session = Path(temp) / 'steal-before.cmjsession', Path(temp) / 'steal-after.cmjsession'
                        before_session.write_bytes(export_session(raw));after_session.write_bytes(export_session(bytes(memory.ram[:2048])))
                        result = subprocess.run([str(args.exe.resolve()), '--compare-flight-interception', str(before_session), str(after_session), str(args.clock_pack.resolve())], capture_output=True, text=True)
                        if result.returncode:
                            raise RuntimeError(f'interception profile={kind}, level={level}, random={(first, second)}: {result.stderr}')
                        interception_cases += 1
                        continue
                    if args.clock_pack and cpu.y in (0, 1) and memory[0x43e] == 0:
                        for _ in range(1500000):
                            bank = memory.bank_at(cpu.pc)
                            if bank == 26 and cpu.pc == 0x888a:
                                break
                            if cpu.pc < 0x8000:
                                raise RuntimeError(f'Original contact clock left PRG at {cpu.pc:04x}')
                            if cpu.pc == 0xcaae:
                                memory.instruction = None
                                memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = 128
                                if memory[0x498]: memory[0x498] -= 1
                            memory.instruction = (bank, cpu.pc)
                            cpu.step()
                        else:
                            raise RuntimeError('Original contact clock exceeded instruction limit')
                        before_session, after_session = Path(temp) / 'before.cmjsession', Path(temp) / 'after.cmjsession'
                        before_session.write_bytes(export_session(raw))
                        after_session.write_bytes(export_session(bytes(memory.ram[:2048])))
                        result = subprocess.run([str(args.exe.resolve()), '--compare-flight-contact-clock', str(before_session), str(after_session), str(args.clock_pack.resolve())], capture_output=True, text=True)
                        if result.returncode:
                            raise RuntimeError(f'clock profile={kind}, level={level}, random={(first, second)}: {result.stderr}')
                        clock_cases += 1
        if args.clock_pack:
            raw = bytearray(original)
            raw[0x600], raw[0x601], raw[0x602] = 2, 2, 2
            raw[0x61c], raw[0x61d] = 7, 0
            memory = ObservedCartridge(rom[16:262160], bytes(raw), None, set())
            memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
            cpu = MPU(memory=memory)
            cpu.pc, cpu.sp, cpu.p, cpu.a, cpu.x, cpu.y = 0x8845, 0x4c, 0x31, 0x6c, 0x6c, 0xa4
            for _ in range(3000000):
                bank = memory.bank_at(cpu.pc)
                if bank == 26 and cpu.pc == 0x888a:
                    break
                if cpu.pc < 0x8000:
                    raise RuntimeError(f'Original multi-contact left PRG at {cpu.pc:04x}')
                if cpu.pc == 0xcaae:
                    memory.instruction = None
                    memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = 128
                    if memory[0x498]: memory[0x498] -= 1
                memory.instruction = (bank, cpu.pc); cpu.step()
            else:
                raise RuntimeError('Original multi-contact exceeded instruction limit')
            before_session, after_session = Path(temp) / 'multi-before.cmjsession', Path(temp) / 'multi-after.cmjsession'
            before_session.write_bytes(export_session(raw)); after_session.write_bytes(export_session(bytes(memory.ram[:2048])))
            result = subprocess.run([str(args.exe.resolve()), '--compare-flight-contact-clock', str(before_session), str(after_session), str(args.clock_pack.resolve())], capture_output=True, text=True)
            if result.returncode:
                raise RuntimeError(f'duplicate multi-contact: {result.stderr}')
            clock_cases += 1
    print(f'{sum(cases.values())} constructed opponent pass defender comparisons matched; result counts {cases}')
    if args.clock_pack:
        print(f'{clock_cases} non-stopping cases matched through contact-clock cleanup, including one with two defenders')
    if args.interception:
        print(f'{interception_cases} result-three cases matched through possession change and field movement')


if __name__ == '__main__':
    main()

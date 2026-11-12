#!/usr/bin/env python3
"""Compare confirmed skip-defense metadata with the original field menu."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from check_ai_field_preparation_original import original_preparation
from export_phase_session import export_session
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def confirmed_skip(prg, raw):
    memory = ObservedCartridge(prg, raw, None, {0x43d, 0x442, 0x634, 0x636})
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1), (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
    memory[0x151:0x155] = [0x67, 0xd5, 0x46, 0x80]; memory[0x1c] = memory[0x1e] = 128
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0xd3ab, 0x50, cpu.UNUSED
    for _ in range(100_000):
        bank = memory.bank_at(cpu.pc)
        if bank == 30 and cpu.pc == 0xd3d4: return bytes(memory.ram[:2048]), memory.writes
        if cpu.pc < 0x8000 or cpu.pc == 0xcaae: raise RuntimeError(f'Unexpected original menu stop at {cpu.pc:04x}')
        memory.instruction = (bank, cpu.pc); cpu.step()
    raise RuntimeError('Original defensive confirmation instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'source-dir', 'pack', 'exe', 'private-temp'): parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args(); rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    prg = rom[16:262160]; total = returns = 0
    with tempfile.TemporaryDirectory(prefix='field-defense-metadata-', dir=args.private_temp) as temporary:
        source = Path(temporary) / 'source.cmjsession'; metadata = Path(temporary) / 'metadata.bin'; expected = Path(temporary) / 'expected.cmjsession'
        for file in sorted(args.source_dir.glob('*-register.bin'))[:12]:
            for prior in (0, 3, 6):
                for cursor in (0, 1, 2):
                    raw = bytearray(file.read_bytes()); raw[0x61e] = cursor; raw[0x43d] = prior; raw[0x634], raw[0x636] = 37, 173
                    raw[0x60b:0x60e] = bytes((6, 6, 6)); seed = 8
                    value = seed; value ^= (value << 13) & 0xffffffff; value ^= value >> 17; value ^= (value << 5) & 0xffffffff
                    original = bytearray(raw); original[0xe2], original[0xe3] = value & 255, (value >> 8) & 255
                    after, writes = confirmed_skip(prg, original)
                    if after[0x43d] != 6 or after[0x442] != raw[0x601 + cursor] or (after[0x634], after[0x636]) != (37, 173):
                        raise RuntimeError('Original skip confirmation did not preserve observed metadata')
                    if any(item['address'] in (0x634, 0x636) for item in writes): raise RuntimeError('Original skip wrote fractional ball coordinates')
                    source.write_bytes(export_session(raw, 15)); expected.write_bytes(export_session(after, 15)); metadata.write_bytes(after)
                    run = subprocess.run([str(args.exe.resolve()), '--compare-field-defense-skip', str(source), str(expected), str(metadata), str(args.pack.resolve()), str(seed)], capture_output=True, text=True)
                    if run.returncode: raise RuntimeError(f'{file.name} prior={prior} cursor={cursor}: {run.stderr}')
                    total += 1
            for xfrac, yfrac in ((37, 173), (255, 0)):
                start = bytearray(file.read_bytes()); start[0x634], start[0x636] = xfrac, yfrac
                if start[0x43d] != 6: raise RuntimeError('Expected an already confirmed skip choice')
                for seed in range(1, 200):
                    value = seed; value ^= (value << 13) & 0xffffffff; value ^= value >> 17; value ^= (value << 5) & 0xffffffff
                    start[0xe2], start[0xe3] = value & 255, (value >> 8) & 255
                    if original_candidate(prg, start)[2] == 2 and original_choice(prg, start)[0x43b] == 2: break
                else: raise RuntimeError('No supported original fallback found')
                for options in ({}, {'through_return': True}, {'through_return': True, 'through_control': True}):
                    after = original_preparation(prg, start, **options)
                    if (after[0x43d], after[0x634], after[0x636]) != (6, xfrac, yfrac): raise RuntimeError('Original fallback changed known metadata')
                returns += 1
    if not total: raise RuntimeError('No defensive registration fixtures found')
    print(f'{total} constructed original skip confirmations matched; {returns} original fallback-to-control cases preserved metadata')


if __name__ == '__main__': main()

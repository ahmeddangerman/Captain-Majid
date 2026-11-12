#!/usr/bin/env python3
"""Verify S pack compatibility and conservative shot gates against original code."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def original_options(prg, raw):
    memory = ObservedCartridge(prg, raw, None, set()); memory.registers[6:8] = [28, 29]
    memory[0x24], memory[0x25] = 28, 29; memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]
    memory[0x1fe], memory[0x1ff] = 255, 1
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p, cpu.a, cpu.x = 0x8c06, 0xfd, cpu.UNUSED, raw[0x441], 0
    for _ in range(10_000):
        if cpu.pc == 0x200: return memory[0x430]
        if cpu.pc < 0x8000 or cpu.pc in (0xcaae, 0xcb0f): raise RuntimeError(f'Unexpected option routine stop {cpu.pc:04x}')
        cpu.step()
    raise RuntimeError('Original shot option instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'old-pack', 'new-pack', 'ram', 'session', 'exe', 'private-temp'): parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args(); rom = args.rom.read_bytes(); old = args.old_pack.read_bytes(); new = args.new_pack.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    if old[:8] != b'CMJSHOTR' or new[:8] != b'CMJSHOTS' or old[8:] != new[8:-118]: raise RuntimeError('S pack changed its R payload')
    raw = args.ram.read_bytes(); prg = rom[16:262160]; cases = 0; safe = 0
    for kind, gate in enumerate(new[-118:]):
        if gate not in (0, 1): raise RuntimeError('Invalid S option gate')
        if not gate: continue
        safe += 1
        for offset in (0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110):
            sample = bytearray(raw); sample[0x441], sample[0x44e], sample[0x446] = 19, 0, 0
            sample[0x300 + 12 * 19] = kind
            for player in range(1, 11): sample[0x300 + 12 * player] = (offset + player) % 118
            if original_options(prg, sample): raise RuntimeError(f'Unsafe S shot gate for type {kind}, roster offset {offset}')
            cases += 1
    def launch(pack): return subprocess.run([str(args.exe.resolve()), str(pack), str(args.session.resolve()), '--demo', '1', '1'], capture_output=True, text=True)
    if launch(args.old_pack.resolve()).returncode or launch(args.new_pack.resolve()).returncode: raise RuntimeError('Valid R/S pack rejected')
    with tempfile.TemporaryDirectory(prefix='shot-ai-pack-', dir=args.private_temp) as temporary:
        for name, data in (('truncated', new[:-1]), ('trailing', new + b'\0'), ('bad-gate', new[:-1] + b'\2')):
            file = Path(temporary) / (name + '.cmjpack'); file.write_bytes(data)
            if launch(file).returncode == 0: raise RuntimeError(f'Malformed {name} pack accepted')
    print(f'R payload preserved; {safe} safe gates checked against original option routine in {cases} roster cases; malformed packs rejected')


if __name__ == '__main__': main()

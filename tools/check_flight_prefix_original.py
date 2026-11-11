#!/usr/bin/env python3
"""Compare constructed flight defender prefixes with original callback 87E1."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ram', 'exe', 'private-temp'):
        p.add_argument('--' + name, type=Path, required=True)
    a = p.parse_args()
    rom, base = a.rom.read_bytes(), a.ram.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256 or len(base) != 2048:
        p.error('Expected verified Japanese ROM and 2048-byte private RAM')
    if a.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private RAM must stay outside the checkout')
    count = 0
    with tempfile.TemporaryDirectory(prefix='flight-prefix-', dir=a.private_temp) as temp:
        before_path, after_path = Path(temp) / 'before.bin', Path(temp) / 'after.bin'
        for side in (0, 11):
            first, capacity = (12, 5) if side == 0 else (1, 4)
            for prior in range(capacity + 1):
                for random_first, random_second in ((1, 0), (0, 0), (0, 1)):
                    for layout in ('all', 'stunned', 'alternating'):
                        ram = bytearray(base)
                        ram[0x5fb], ram[0x5fe], ram[0x61a] = side, 101, 255
                        ram[0xe2], ram[0xe3], ram[0x600] = random_first, random_second, prior
                        ram[0x601:0x606] = bytes(first + i if i < prior else 0 for i in range(5))
                        for offset in range(10):
                            record = 0x300 + 12 * (first + offset)
                            ram[record + 6] = 116 if layout != 'alternating' or offset % 2 == 0 else 52
                            ram[record + 8] = 124
                            ram[record + 10] = int(layout == 'stunned' and offset % 2 == 0)
                        before_path.write_bytes(ram)
                        memory = ObservedCartridge(rom[16:262160], bytes(ram), None, set())
                        memory.registers[6:8] = [26, 27]
                        cpu = MPU(memory=memory)
                        cpu.pc, cpu.sp, cpu.p = 0x87e1, 0xfd, cpu.UNUSED
                        memory[0x1fe], memory[0x1ff] = 255, 1
                        for _ in range(10000):
                            if cpu.pc == 0x200:
                                break
                            cpu.step()
                        else:
                            raise RuntimeError('Original callback exceeded instruction limit')
                        after_path.write_bytes(bytes(memory.ram[:2048]))
                        result = subprocess.run([str(a.exe.resolve()), '--compare-flight-prefix', str(before_path), str(after_path)], capture_output=True, text=True)
                        if result.returncode:
                            raise RuntimeError(f'side={side} prior={prior} random={(random_first, random_second)} layout={layout}: {result.stderr}')
                        count += 1
    print(f'{count} constructed original flight-prefix callbacks matched on both sides')


if __name__ == '__main__':
    main()

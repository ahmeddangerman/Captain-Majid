#!/usr/bin/env python3
"""Compare the isolated defensive menu affordability branch with a private ROM.

Cost loading, move chooser, script execution and stamina deduction are outside
this check. No ROM data or observations are written into the checkout.
"""
import argparse
import hashlib
import random
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--binary', type=Path, default=Path('build/defense-affordability-tests'))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected approved Japanese ROM')
    rng = random.Random(117)
    boundary = (0, 1, 127, 128, 255, 256, 1023, 32767, 32768, 65535)
    cases = [(stamina, cost, subtype) for stamina in boundary for cost in boundary for subtype in (0, 1, 127, 255)]
    cases.extend((rng.randrange(65536), rng.randrange(65536), rng.choice((0, 1, 2, 127, 255))) for _ in range(10000))
    process = subprocess.run([str(args.binary.resolve()), '--batch'],
                             input=''.join(f'{s} {c} {t}\n' for s,c,t in cases),
                             text=True, capture_output=True, check=True, timeout=30)
    outputs = process.stdout.splitlines()
    if len(outputs) != len(cases): raise RuntimeError('Native output count mismatch')
    memory = ReferenceCartridge(rom[16:262160]);memory.registers[6:8] = [28, 29]
    cpu = MPU(memory=memory)
    for (stamina, cost, subtype), output in zip(cases, outputs):
        memory[0x34], memory[0x35] = 0, 3
        memory[0x301], memory[0x302] = stamina & 255, stamina >> 8
        memory[0x43f], memory[0x440] = cost & 255, cost >> 8
        memory[0x43e] = subtype
        # Host calls original D76B with synthetic return to its caller D75B.
        memory[0x1fe], memory[0x1ff] = 0x5a, 0xd7
        cpu.pc, cpu.sp, cpu.p = 0xd76b, 0xfd, cpu.UNUSED
        for _ in range(30):
            if cpu.pc in (0xd762, 0xd769): break
            cpu.step()
        else: raise RuntimeError('Original affordability instruction bound')
        if output not in ('0', '1') or (output == '1') != (cpu.pc == 0xd769):
            raise RuntimeError(f'Original mismatch: stamina={stamina}, cost={cost}, subtype={subtype}')
        if memory[0x301] | memory[0x302] << 8 != stamina:
            raise RuntimeError('Original changed stamina in isolated check')
    print(f'{len(cases)} original affordability outcomes match (numeric branch only)')


if __name__ == '__main__':
    main()

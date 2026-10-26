#!/usr/bin/env python3
"""Compare the native contact-loss arithmetic with a user-supplied Japanese ROM."""

from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU

PAYLOAD_SHA256 = "e36f43f90501bad2cb43407922bd2ed686a8a9bd5e2be6bc1e7e4b94f02933bd"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232:
        parser.error("Expected the 393232-byte Japanese reference ROM")
    if hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")

    prg = rom[16 : 16 + 262144]
    cpu = MPU()
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        cpu.memory[address : address + 8192] = prg[bank * 8192 : (bank + 1) * 8192]

    powers = (0, 1, 20, 255, 256, 257, 32767, 32768, 32769, 65535)
    cases = [(power, loss) for power in powers for loss in range(256)]
    rng = random.Random(0x435432)
    cases.extend((rng.randrange(65536), rng.randrange(256)) for _ in range(4096))
    expected = []
    for power, loss in cases:
        cpu.pc = 0x819F
        cpu.p = cpu.UNUSED  # decimal mode off; routine sets its own borrow state
        cpu.a = cpu.x = cpu.y = 0
        cpu.memory[0x619] = loss
        cpu.memory[0x61C] = power & 255
        cpu.memory[0x61D] = power >> 8
        for _ in range(32):
            if cpu.pc == 0x81B8:
                break
            cpu.step()
        else:
            raise RuntimeError(f"Reference routine did not reach its exit: ${cpu.pc:04X}")
        expected.append(cpu.memory[0x61C] | cpu.memory[0x61D] << 8)

    result = subprocess.run(
        [str(args.binary.resolve()), "--strength-batch"],
        input="".join(f"{power} {loss}\n" for power, loss in cases),
        text=True,
        capture_output=True,
        check=True,
        timeout=30,
    )
    actual = [int(line) for line in result.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f"Native result count {len(actual)} != {len(expected)}")
    for case, reference, native in zip(cases, expected, actual):
        if reference != native:
            raise RuntimeError(f"Mismatch: power/loss={case}, ROM={reference}, native={native}")
    print(f"ROM comparison passed: {len(cases)} cases, exact 16-bit outputs matched")


if __name__ == "__main__":
    main()

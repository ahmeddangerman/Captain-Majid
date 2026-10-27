#!/usr/bin/env python3
"""Compare nearby-defender selection with routine $E4D7 in the verified ROM."""
from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256


def original(prg: bytes, case: tuple) -> tuple[int, ...]:
    side, ball_x, ball_y, radius, players = case
    cpu = MPU()
    ram = cpu.memory
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        ram[address:address + 8192] = prg[bank * 8192:(bank + 1) * 8192]
    ram[0x05FB] = side
    ram[0x0635], ram[0x0637] = ball_x, ball_y
    for index, (x, y, status_nonzero) in enumerate(players):
        player_id = index + 1 if index < 10 else index + 2
        record = 0x0300 + player_id * 12
        ram[record + 6], ram[record + 8] = x, y
        ram[record + 10] = int(status_nonzero)

    # Enter the ROM routine directly with a synthetic RTS target at $0200.
    cpu.pc, cpu.sp, cpu.p, cpu.a = 0xE4D7, 0xFD, cpu.UNUSED, radius
    ram[0x01FE], ram[0x01FF] = 0xFF, 0x01
    for _ in range(2000):
        if cpu.pc == 0x0200:
            count = ram[0x0600]
            if count > 5:
                raise RuntimeError(f"ROM returned impossible selection count {count}")
            return (count, *ram[0x0601:0x0601 + count])
        cpu.step()
    raise RuntimeError(f"ROM selector did not return; stopped at ${cpu.pc:04X}")


def native_line(case: tuple) -> str:
    side, ball_x, ball_y, radius, players = case
    values = [side, ball_x, ball_y, radius]
    for x, y, status_nonzero in players:
        values.extend((x, y, int(status_nonzero)))
    return " ".join(map(str, values))


def make_cases(count: int) -> list[tuple]:
    rng = random.Random(0xE4D7)
    cases = []
    sides = (0, 11)
    for index, side in enumerate(sides):
        players = [(200, 200, True)] * 20
        selected_range = range(10, 20) if side == 0 else range(10)
        for offset, player_index in enumerate(selected_range):
            # Includes exact-radius and just-inside edges, with some status skips.
            dx = (9, 10, 1, 0, 8)[offset % 5]
            dy = (1, 0, 9, 10, 8)[offset % 5]
            players[player_index] = (120 + dx, 100 + dy, offset % 4 == 3)
        cases.append((side, 120, 100, 10, players))
    for index in range(max(0, count - len(cases))):
        side = rng.choice(sides)
        players = [(rng.randrange(256), rng.randrange(256), rng.randrange(5) == 0)
                   for _ in range(20)]
        if index % 4 == 0:
            start = 10 if side == 0 else 0
            for player_index in range(start, start + 6):
                players[player_index] = (120, 100, False)
        cases.append((side, rng.randrange(256), rng.randrange(256),
                      (0, 1, 7, 10, 255)[index % 5], players))
    return cases


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    parser.add_argument("--cases", type=int, default=1024)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if (rom[:4] != b"NES\x1a" or len(rom) != 393232 or
            hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256):
        parser.error("ROM payload does not match the verified Japanese reference")
    if args.cases < 2:
        parser.error("--cases must be at least 2")
    prg = rom[16:16 + 262144]
    cases = make_cases(args.cases)
    run = subprocess.run([str(args.binary.resolve()), "--defender-selection-batch"],
                         input="\n".join(map(native_line, cases)) + "\n", text=True,
                         capture_output=True, check=True, timeout=30)
    native = [tuple(map(int, line.split())) for line in run.stdout.splitlines()]
    if len(native) != len(cases):
        raise RuntimeError(f"Native returned {len(native)} outputs for {len(cases)} cases")
    for index, (case, actual) in enumerate(zip(cases, native)):
        expected = original(prg, case)
        if actual != expected:
            raise AssertionError(
                f"case {index}: ROM={expected}, native={actual}, input={case}")
    print(f"Defender selection matched {len(cases)} original-ROM cases "
          "(ordered IDs, proximity bounds, status skips, and team limits).")


if __name__ == "__main__":
    main()

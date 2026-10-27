#!/usr/bin/env python3
"""Compare interior-field loose-ball recovery with the verified Japanese ROM."""
from __future__ import annotations

import argparse
import hashlib
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU
from verify_action_strength import PAYLOAD_SHA256


def original(prg: bytes, fields: tuple) -> tuple[int, ...]:
    (ball_x, ball_y, owner, opponent, random_e2, random_e3, side,
     players, defenders) = fields
    cpu = MPU()
    ram = cpu.memory
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        ram[address:address + 8192] = prg[bank * 8192:(bank + 1) * 8192]
    ram[0x0635], ram[0x0637] = ball_x, ball_y
    ram[0x0441], ram[0x0442] = owner, opponent
    ram[0x00E2], ram[0x00E3] = random_e2, random_e3
    ram[0x05FB], ram[0x05FF] = side, 0
    ram[0x043C] = 77
    ram[0x0600] = len(defenders)
    for index, (player_id, action) in enumerate(defenders):
        ram[0x0601 + index] = player_id
        ram[0x060B + index] = action
    for player_id, player in enumerate(players):
        record = 0x0300 + player_id * 12
        ram[record + 6], ram[record + 8] = player[0], player[1]
        ram[record + 10] = 1 if player[2] else 0

    # Start where the $8BDF caller returns from its random-generator call.
    # The E2/E3 bytes are controlled inputs here; the subsequent recovery
    # code, including coordinate jitter and the ownership handoff, is original.
    cpu.pc, cpu.sp, cpu.p = 0x8BE2, 0xFD, cpu.UNUSED
    ram[0x01FE], ram[0x01FF] = 0xFF, 0x01  # RTS return sentinel at $0200.
    direct_capture = False
    for _ in range(5000):
        # $8C61 checks a candidate action; the actual selected-ID return is
        # only reached at $8C68 after the E2 threshold passes.
        direct_capture = direct_capture or cpu.pc == 0x8C68
        if cpu.pc == 0x8C2E:
            recovered_id = ram[0x0441]
            record = 0x0300 + recovered_id * 12
            return (ram[0x0635], ram[0x0637], recovered_id, ram[0x05FB],
                    ram[0x043C], int(direct_capture), ram[record + 6], ram[record + 8])
        if cpu.pc == 0x0200:
            raise RuntimeError("ROM recovery returned before reaching its ownership handoff")
        cpu.step()
    raise RuntimeError(f"ROM recovery did not reach its ordinary handoff: ${cpu.pc:04X}")


def make_native_line(fields: tuple) -> str:
    ball_x, ball_y, owner, opponent, random_e2, random_e3, _side, players, defenders = fields
    values = [ball_x, ball_y, owner, opponent, random_e2, random_e3, 0, len(defenders)]
    for x, y, unavailable in players[1:11] + players[12:22]:
        values.extend((x, y, int(unavailable)))
    for player_id, action in defenders:
        values.extend((player_id, action))
    return " ".join(map(str, values))


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    parser.add_argument("--cases", type=int, default=512)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if (rom[:4] != b"NES\x1a" or len(rom) != 393232 or
            hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256):
        parser.error("ROM payload does not match the verified Japanese reference")
    if args.cases < 1:
        parser.error("--cases must be at least 1")
    prg = rom[16:16 + 262144]
    rng = random.Random(0xBA11EC)
    cases = []
    for index in range(args.cases):
        players = [(0, 0, True)] * 22
        for player_id in (*range(1, 11), *range(12, 22)):
            players[player_id] = (rng.randrange(96, 161), rng.randrange(96, 161),
                                  rng.randrange(5) == 0)
        for player_id in (1, 2, 3, 12, 13, 14):
            players[player_id] = (120, 120, False)
        e2 = (0x20 if index % 4 == 0 else 0x80 if index % 4 == 1
              else rng.randrange(256))
        defenders = []
        for _ in range(rng.randrange(10)):
            defenders.append((rng.choice((*range(1, 11), *range(12, 22))),
                              rng.randrange(7)))
        if index % 4 == 0:
            defenders.insert(0, (7, 5))
        cases.append((rng.randrange(112, 145), rng.randrange(112, 145),
                      rng.randrange(22), rng.randrange(22), e2, rng.randrange(256),
                      rng.choice((0, 11)), players, defenders))

    native_lines = [make_native_line(case) for case in cases]
    run = subprocess.run([str(args.binary), "--ball-recovery-batch"],
                         input="\n".join(native_lines) + "\n", text=True,
                         capture_output=True, check=True)
    native = [tuple(map(int, line.split())) for line in run.stdout.splitlines()]
    if len(native) != len(cases):
        raise RuntimeError(f"Native returned {len(native)} outputs for {len(cases)} cases")

    for index, (case, result) in enumerate(zip(cases, native)):
        want = original(prg, case)
        observed = result + want[6:8]
        if observed != want:
            raise AssertionError(f"case {index}: original={want}, native={observed}, input={case}")
    print(f"Ball recovery matched {len(cases)} original-ROM cases (owner, side, ball jitter, subtype, and player placement).")


if __name__ == "__main__":
    main()

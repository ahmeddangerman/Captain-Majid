#!/usr/bin/env python3
"""Compare native energy, keeper state, and random steps with a private Japanese ROM."""
from __future__ import annotations

import argparse
import hashlib
import itertools
import random
import subprocess
from pathlib import Path

from py65.devices.mpu6502 import MPU

from verify_action_strength import PAYLOAD_SHA256


class MirroredMemory:
    """Mirror the NES CPU's internal 2 KiB RAM throughout $0000-$1FFF."""

    def __init__(self):
        self.data = bytearray(65536)

    @staticmethod
    def canonical(address):
        address &= 65535
        return address & 2047 if address < 8192 else address

    def __getitem__(self, address):
        if isinstance(address, slice):
            return [self[index] for index in range(*address.indices(65536))]
        return self.data[self.canonical(address)]

    def __setitem__(self, address, value):
        if isinstance(address, slice):
            indices = range(*address.indices(65536))
            if len(indices) != len(value):
                raise ValueError("Memory slice assignment must preserve its length")
            for index, byte in zip(indices, value):
                self[index] = byte
        else:
            self.data[self.canonical(address)] = value


def cpu_for(prg):
    cpu = MPU(memory=MirroredMemory())
    for bank, address in ((26, 0x8000), (27, 0xA000), (30, 0xC000), (31, 0xE000)):
        cpu.memory[address:address + 8192] = prg[bank * 8192:(bank + 1) * 8192]
    return cpu


def run(cpu, entry, *, carry=0, stop=0x0200, checkpoint=None):
    cpu.pc, cpu.sp = entry, 0xFD
    cpu.p = cpu.UNUSED | carry  # The NES executes binary arithmetic, including with D set.
    cpu.memory[0x1FE], cpu.memory[0x1FF] = 0xFF, 0x01
    seen = False
    for _ in range(512):
        if cpu.pc == stop:
            return seen
        seen = seen or cpu.pc == checkpoint
        cpu.step()
    raise RuntimeError(f"Original routine did not reach ${stop:04X}: ${cpu.pc:04X}")


def compare(binary, mode, inputs, expected):
    result = subprocess.run([str(binary.resolve()), mode],
                            input="".join(" ".join(map(str, fields)) + "\n" for fields in inputs),
                            text=True, capture_output=True, check=True, timeout=30)
    actual = [tuple(map(int, line.split())) for line in result.stdout.splitlines()]
    if len(actual) != len(expected):
        raise RuntimeError(f"{mode}: native result count {len(actual)} != {len(expected)}")
    for index, (reference, native) in enumerate(zip(expected, actual)):
        if reference != native:
            raise RuntimeError(f"{mode} case {index}: {inputs[index]}\nROM={reference}\nNative={native}")
    print(f"{mode}: {len(inputs)} original-ROM cases matched", flush=True)


def verify_energy(cpu, binary, rng):
    words = (0, 1, 2, 3, 127, 128, 255, 256, 257, 32767, 32768, 32769,
             43690, 43691, 65534, 65535)
    contexts = ((0, 0, 0, 0), (32, 0, 0, 2), (32, 0, 0, 3),
                (32, 0, 0, 128), (32, 0, 1, 3), (32, 11, 0, 3),
                (31, 11, 0, 3), (33, 0, 0, 0), (32, 255, 255, 255))
    inputs = [(stamina, cost, *context, wrapper, carry)
              for stamina, cost, context, wrapper, carry
              in itertools.product(words, words, contexts, (0, 1), (0, 1))]
    inputs += [(rng.randrange(65536), rng.randrange(65536),
                rng.choice((32, rng.randrange(256))), rng.randrange(256),
                rng.randrange(256), rng.randrange(256), rng.randrange(2), rng.randrange(2))
               for _ in range(512)]
    expected = []
    for stamina, cost, player, side, action, subtype, wrapper, carry in inputs:
        cpu.memory[0x34:0x36] = [0, 3]
        cpu.memory[0x300:0x303] = [player, stamina & 255, stamina >> 8]
        cpu.memory[0x43F:0x441] = [cost & 255, cost >> 8]
        for address, value in ((0x5FB, side), (0x43B, action), (0x43C, subtype), (0x3A, 255)):
            cpu.memory[address] = value
        run(cpu, 0x8FF3 if wrapper else 0x8FFB, carry=carry)
        expected.append((cpu.memory[0x301] | cpu.memory[0x302] << 8,
                         cpu.memory[0x43F] | cpu.memory[0x440] << 8,
                         int(bool(cpu.memory[0x3A] & 128)), cpu.p & cpu.CARRY))
    compare(binary, "--energy-batch", inputs, expected)


def verify_story(cpu, binary, rng):
    inputs = list(itertools.product((0, 1, 2, 3, 255), (0, 1, 11), (4, 5, 6),
                                   (0, 1, 3, 4, 255), (0, 1, 2, 3, 4, 131, 255),
                                   (0, 127, 128, 255)))
    inputs += [tuple(rng.randrange(256) for _ in range(6)) for _ in range(512)]
    expected = []
    for classification, side, opponent, story, subtype, first in inputs:
        cpu.y = classification
        for address, value in ((0x5FB, side), (0x2B, opponent), (0x446, story),
                               (0x43C, subtype), (0xE2, first), (0x612, 254), (0x616, 253)):
            cpu.memory[address] = value
        overridden = run(cpu, 0x856B, stop=0x8599, checkpoint=0x8589)
        expected.append((cpu.memory[0x612], int(overridden), cpu.memory[0x616]))
    compare(binary, "--keeper-story-batch", inputs, expected)


def verify_keeper_state(cpu, binary, rng):
    # Include both original keeper records without adding side to the value API.
    fixtures = [(timer, 49, 251, selector, 0, 1, 0, side)
                for timer, selector, side
                in itertools.product(range(256), (0, 1, 2, 255), (0, 11))]
    fixtures += [(249, penalty, penalty ^ first, 2, first, 0, 1, side)
                 for penalty, first, side
                 in itertools.product(range(256), (0, 63, 64, 255), (0, 11))]
    fixtures += [(timer, 49, 250, 2, 63, invoke_timer, invoke_penalty, timer % 2 * 11)
                 for timer, invoke_timer, invoke_penalty
                 in itertools.product(range(256), (0, 1), (0, 1))]
    fixtures += [(rng.randrange(256), rng.randrange(256), rng.randrange(256),
                  rng.randrange(256), rng.randrange(256), rng.randrange(2),
                  rng.randrange(2), rng.choice((0, 11))) for _ in range(512)]
    inputs, expected = [], []
    for timer, penalty, duration, selector, first, invoke_timer, invoke_penalty, side in fixtures:
        inputs.append((timer, penalty, duration, selector, first, invoke_timer, invoke_penalty))
        cpu.memory[0x5FB], cpu.memory[0x443], cpu.memory[0xE2] = side, selector, first
        other_slot = side ^ 11
        player = cpu.memory[0xCD89 + other_slot * 2] | cpu.memory[0xCD8A + other_slot * 2] << 8
        cpu.memory[player + 5:player + 8] = [timer, duration, penalty]
        if invoke_penalty:
            run(cpu, 0x86D3)
        if invoke_timer:
            run(cpu, 0x86BD)
        expected.append((cpu.memory[player + 5], cpu.memory[player + 7], cpu.memory[player + 6]))
    compare(binary, "--keeper-state-batch", inputs, expected)


def verify_random(cpu, binary, rng):
    memory = cpu.memory
    memory[0x7FF], memory[0], memory[0x7F] = 13, 27, 41
    assert memory[0x780 + 127] == 13
    assert memory[0x780 + 128] == 27
    assert memory[0x780 + 255] == 41
    memory[0x1800] = 53
    assert memory[0] == memory[0x800] == memory[0x1000] == 53
    patterns = ((0, 0, 0, 0, 0), (255, 255, 255, 255, 255),
                (127, 128, 1, 255, 0), (128, 127, 255, 1, 255),
                (1, 254, 128, 128, 127), (254, 1, 127, 127, 128))
    fixtures = [(index, *pattern, carry)
                for index, pattern, carry in itertools.product(range(256), patterns, (0, 1))]
    fixtures += [(*[rng.randrange(256) for _ in range(6)], rng.randrange(2)) for _ in range(512)]
    inputs, expected = [], []
    for index, first, second, player, sound, tail, carry in fixtures:
        memory[0x300 + index] = player
        memory[0x700 + index] = sound
        memory[0x780 + index] = tail
        memory[0xE1:0xE4] = [index, first, second]
        # Sample the canonical RAM used by the CPU, including the wrapped tail range.
        inputs.append((memory[0xE1], memory[0xE2], memory[0xE3],
                       memory[(0x300 + index) & 2047], memory[(0x700 + index) & 2047],
                       memory[(0x780 + index) & 2047], carry))
        run(cpu, 0xC9C5, carry=carry)
        expected.append((memory[0xE1], memory[0xE2], memory[0xE3], cpu.p & cpu.CARRY))
    assert {fields[0] for fields in inputs} == set(range(256))
    compare(binary, "--random-step-batch", inputs, expected)
    print("Random oracle: all 256 indices, both carries, and $0800 RAM mirroring covered", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b"NES\x1a" or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error("ROM payload does not match the verified Japanese reference")
    cpu = cpu_for(rom[16:16 + 262144])
    rng = random.Random(0x53484F54)
    verify_energy(cpu, args.binary, rng)
    verify_story(cpu, args.binary, rng)
    verify_keeper_state(cpu, args.binary, rng)
    verify_random(cpu, args.binary, rng)


if __name__ == "__main__":
    main()

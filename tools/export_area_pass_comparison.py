#!/usr/bin/env python3
"""Export private original tables for the queued area-pass contact."""
import argparse
import hashlib
from pathlib import Path

from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected the verified Japanese reference ROM')
    output = args.output.resolve()
    if output.is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private content must stay outside the checkout')
    memory = ObservedCartridge(rom[16:262160], bytes(2048), None, set())
    memory.registers[6:8] = [26, 27]
    pointer = memory[0x8D9D] | memory[0x8D9E] << 8
    values = [memory[pointer + 12], memory[pointer + 13], memory[0x88F1]]
    values.extend(memory[0x8294 + index] for index in range(4))
    with output.open('x', encoding='utf-8') as stream:
        stream.write('CMJPASS1\n' + ' '.join(map(str, values)) + '\n')
    print('Exported private area-pass comparison data.')


if __name__ == '__main__':
    main()

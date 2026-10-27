#!/usr/bin/env python3
"""Pack an explicit private 2048-byte RAM checkpoint into a CMJFIELD1 snapshot."""
import argparse
import struct
from pathlib import Path


def export_state(ram):
    if len(ram) != 2048:
        raise ValueError('Internal RAM checkpoint must be exactly2048bytes')
    def word(address):
        return ram[address] | ram[address + 1] << 8
    side, owner = ram[0x5FB], ram[0x441]
    if side not in (0, 11) or owner > 21:
        raise ValueError('Unsupported field side/owner')
    teams = ram[0x2C:0x30]
    if any(teams[i] >= 4 or teams[i + 1] >= 3 for i in (0, 2)):
        raise ValueError('Unsupported field formation/tactic')
    data = bytearray(b'CMJFIELD1')
    data.extend(ram[a] for a in (0x5FB, 0x441, 0x635, 0x637, 0x5FE, 0x5FF, 0xE2, 0xE3, 0x600, 0x62A))
    data.extend(teams)
    for slot in range(22):
        record = 0x300 + 12 * slot
        data.extend(struct.pack('<BBH', ram[record], ram[record + 3], word(record + 1)))
    for slot in (*range(1, 11), *range(12, 22)):
        record = 0x300 + 12 * slot
        data.extend(struct.pack('<HHBB', word(record + 5), word(record + 7), ram[record + 9], ram[record + 10]))
    for slot in (0, 11):
        record = 0x300 + 12 * slot
        data.extend(ram[record + offset] for offset in (5, 6, 7, 10))
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('ram', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.output.suffix != '.cmjstate':
        parser.error('Use a private .cmjstate output path (ignored by Git)')
    try:
        data = export_state(args.ram.read_bytes())
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError) as error:
        parser.exit(1, f'Field export stopped: {error}\n')
    print(f'Exported {len(data)} bytes; private field phase state, not a complete game save.')


if __name__ == '__main__':
    main()

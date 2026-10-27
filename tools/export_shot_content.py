#!/usr/bin/env python3
"""Export private Japanese development data to a local CMJSHOT4 pack."""
import argparse
import hashlib
import struct
from pathlib import Path
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def word(memory, address):
    return memory[address] | memory[address + 1] << 8


def export_data(rom):
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        raise ValueError('ROM does not match verified Japanese reference')
    prg = rom[16:262160]
    stats = ReferenceCartridge(prg)
    stats.registers[6:8] = [28, 29]
    gameplay = ReferenceCartridge(prg)
    data = bytearray(b'CMJSHOT4' + hashlib.sha256(rom[16:]).digest())

    def vector(values):
        data.extend(struct.pack('<H', len(values)))
        data.extend(values)

    def move_lists(pointers):
        for start, end in zip(pointers, pointers[1:]):
            size = end - start
            if not (0x8000 <= start <= end <= 0xC000) or size % 4 or size > 256:
                raise ValueError('Unsupported move table bounds')
            data.append(size // 4)
            data.extend(stats[start:end])

    data.extend(stats[0x95D6 + 4 * i] for i in range(35))
    vector([stats[0x9662 + 12 * i] for i in range(221)])
    data.extend(stats[0x818E:0x8199])
    vector([])  # Runtime generic-team selection is not yet imported.
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0x9FCE + 24 * profile:0x9FCE + 24 * profile + 31])
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0xAE86 + 8 * profile:0xAE86 + 8 * profile + 8])
    curve = stats[0x9E4E:0x9F0E]
    data.extend(stats[0x8206:0x8215])
    move_lists([word(stats, 0x9460 + 2 * i) for i in range(6)])
    data.extend(curve)
    data.extend(stats[0x824C:0x825B])
    move_lists([word(stats, 0x9554 + 2 * i) for i in range(5)] + [0x959E])
    data.extend(curve)
    data.extend(stats[0x82C0 + i] for i in (0, 1, 2, 5))
    move_lists([word(stats, 0x959E + 2 * i) for i in range(5)])
    data.extend(curve)
    data.extend(gameplay[0x83D7:0x83DC])
    data.extend(gameplay[0x83DD:0x83E2])
    vector(gameplay[0x83E1:0x84E1])
    data.extend(struct.pack('<H', 128))
    variation = word(gameplay, 0x8D95)
    data.extend(gameplay[variation:variation + 256])
    data.extend(gameplay[0x83F1:0x83F5])
    # Pointer table starts at8D93; keeper selector3 indexes its fourth pointer.
    keeper_variation = word(gameplay, 0x8D93 + 6)
    data.extend(gameplay[keeper_variation:keeper_variation + 2])
    data.extend(gameplay[0x86B9:0x86BD])
    data.extend(gameplay[0x86B1:0x86B9])
    data.extend(gameplay[0xDDCB:0xDDD9])
    data.extend(gameplay[0xDDD9:0xDDFD])
    # Only the two supported action0 flight variants; do not export adjacent
    # instruction bytes as hypothetical action1..4 eligibility/threshold rows.
    flight_actions = gameplay[0x888B:0x888D]
    data.extend(flight_actions)
    data.extend(gameplay[0x88EB + action] for action in flight_actions)
    flight_variation = word(gameplay, 0x8D93 + 10)
    for action in flight_actions:
        data.extend(gameplay[flight_variation + 2 * action:flight_variation + 2 * action + 2])
    for action in flight_actions:
        data.extend(gameplay[0x828C + 4 * action:0x828C + 4 * action + 4])
    data.extend(gameplay[0xFACC:0xFB4C])
    data.extend(gameplay[0xE8ED:0xE8F5])
    for player_type in range(35):
        data.extend(stats[0x95D6 + 4 * player_type + 1:0x95D6 + 4 * player_type + 4])
    data.extend(struct.pack('<H', 221))
    for alternate in range(221):
        data.extend(stats[0x9662 + 12 * alternate + 1:0x9662 + 12 * alternate + 4])
    data.extend(struct.pack('<H', 256))
    for profile in range(256):
        data.extend(stats[0xAFAE + 12 * profile + 4:0xAFAE + 12 * profile + 8])
    data.extend(gameplay[0xA1DC:0xA1EB])
    for table, count, lower, upper in ((0xA6AD, 4, 0xA6B5, 0xAB65),
                                      (0xAB65, 12, 0xAB7D, 0xB98D)):
        for index in range(count):
            start = word(gameplay, table + 2 * index)
            if not lower <= start <= upper - 300:
                raise ValueError('Unsupported field formation template bounds')
            data.extend(gameplay[start:start + 300])
    return bytes(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    if args.output.suffix != '.cmjpack':
        parser.error('Output must use the .cmjpack extension (ignored by Git)')
    try:
        data = export_data(args.rom.read_bytes())
        with args.output.open('xb') as output:
            output.write(data)
    except (OSError, ValueError) as error:
        parser.exit(1, f'Export stopped: {error}\n')
    print(f'Exported {len(data)} bytes to {args.output}; private data-only development pack.')
    print('Generic team selection, graphics, audio and text are not included.')


if __name__ == '__main__':
    main()

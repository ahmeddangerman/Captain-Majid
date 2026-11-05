#!/usr/bin/env python3
"""Export private action-6 destination pairs from the verified original ROM."""
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
    values = [memory[0xe6cf + i] for i in range(16)]
    with output.open('x', encoding='utf-8') as stream:
        stream.write('CMJAREA6\n' + ' '.join(map(str, values)) + '\n')
    print('Exported eight private area-action destination pairs.')


if __name__ == '__main__':
    main()

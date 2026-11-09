#!/usr/bin/env python3
"""Export a private, replaceable 20-byte pass-distance table from the verified ROM."""
import argparse
import hashlib
from pathlib import Path

from reference_identity import PAYLOAD_SHA256


def export_data(rom):
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        raise ValueError('ROM does not match the verified Japanese reference')
    offset = 16 + 30 * 8192 + (0xdfbd - 0xc000)
    return b'CMJPDST1' + bytes(rom[offset:offset + 20])


def write_private(output, data, checkout):
    output = output.resolve()
    checkout = checkout.resolve()
    if output == checkout or checkout in output.parents:
        raise ValueError('Private table must remain outside the Git checkout')
    with output.open('xb') as stream:
        stream.write(data)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('output', type=Path)
    args = parser.parse_args()
    try:
        data = export_data(args.rom.read_bytes())
        write_private(args.output, data, Path(__file__).resolve().parents[1])
    except (OSError, ValueError) as exc:
        parser.exit(1, f'Pass distance export stopped: {exc}\n')
    print(f'Exported private pass-distance table to {args.output}')


if __name__ == '__main__':
    main()

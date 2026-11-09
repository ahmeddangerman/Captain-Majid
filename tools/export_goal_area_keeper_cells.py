#!/usr/bin/env python3
"""Export private opponent goal-area keeper cell content from the approved ROM."""
import argparse
import hashlib
from pathlib import Path
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    parser.add_argument('output',type=Path)
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('Expected approved Japanese ROM')
    output=args.output.resolve()
    if output.is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private content must remain outside the checkout')
    offset=16+26*8192+(0x84ef-0x8000)
    cells=rom[offset:offset+9]
    if len(cells)!=9 or any(cell>=240 for cell in cells):
        parser.error('Invalid keeper-area cell content')
    with output.open('x',encoding='ascii') as stream:
        stream.write('CMJGKAREA1\n'+' '.join(map(str,cells))+'\n')
    print(f'Exported private keeper-area content to {output}')


if __name__=='__main__':
    main()

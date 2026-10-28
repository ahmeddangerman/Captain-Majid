#!/usr/bin/env python3
"""Export private replaceable capture-clearance content from the approved ROM."""
import argparse
import hashlib
from pathlib import Path

# Approved reference fingerprint, also recorded by verify_action_strength.py.
PAYLOAD_SHA256 = "e36f43f90501bad2cb43407922bd2ed686a8a9bd5e2be6bc1e7e4b94f02933bd"


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    parser.add_argument('output',type=Path)
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('ROM does not match the verified Japanese reference')
    if args.output.suffix!='.cmjclearance':
        parser.error('Use a private .cmjclearance output path')
    prg=rom[16:262160]
    offset=len(prg)-8192+(0xE6CF-0xE000)
    values=prg[offset:offset+16]
    distance=prg[len(prg)-8192+(0xE6C8-0xE000)]
    rows=[f'{values[i]} {values[i+1]}' for i in range(0,16,2)]+[str(distance)]
    with args.output.open('x') as output:output.write('\n'.join(rows)+'\n')
    print('Exported eight private destinations and distance adjustment')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Export private selector35/36 clearance-race speed pairs from the approved ROM."""
import argparse
import hashlib
import struct
from pathlib import Path

PAYLOAD_SHA256="e36f43f90501bad2cb43407922bd2ed686a8a9bd5e2be6bc1e7e4b94f02933bd"


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('rom',type=Path);p.add_argument('output',type=Path)
    a=p.parse_args();rom=a.rom.read_bytes()
    if len(rom)!=393232 or rom[:4]!=b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        p.error('ROM does not match verified Japanese reference')
    if a.output.suffix!='.cmjrace':p.error('Use a private .cmjrace output path')
    prg=rom[16:262160];rows=[]
    for profile in range(256):
        offset=29*8192+(0xAFAE-0xA000)+12*profile+8
        first,second=struct.unpack_from('<HH',prg,offset)
        rows.append(f'{first} {second}')
    with a.output.open('x') as output:output.write('\n'.join(rows)+'\n')
    print('Exported256 private clearance-race speed pairs')


if __name__=='__main__':main()

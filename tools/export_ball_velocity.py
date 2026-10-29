#!/usr/bin/env python3
"""Export private CMJBALL1 projection data from the approved Japanese ROM."""
import argparse
import hashlib
import struct
from pathlib import Path
from verify_action_strength import PAYLOAD_SHA256


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path);p.add_argument('output',type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 if a.output.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private projection data must remain outside checkout')
 offset=16+262144-8192+(0xFB4C-0xE000);words=struct.unpack('<64H',rom[offset:offset+128])
 with a.output.open('x') as f:f.write('CMJBALL1\n'+' '.join(map(str,words))+'\n')
 print(a.output)


if __name__=='__main__':main()

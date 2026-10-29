#!/usr/bin/env python3
"""Export private CMJKICKFORMATION1 data from the approved Japanese ROM."""
import argparse
import hashlib
from pathlib import Path
from verify_action_strength import PAYLOAD_SHA256


def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('rom',type=Path);p.add_argument('output',type=Path)
 a=p.parse_args();rom=a.rom.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 if a.output.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private formation data must remain outside checkout')
 offset=16+30*8192+(0xdc87-0xc000);cells=rom[offset:offset+88]
 with a.output.open('x') as stream:stream.write('CMJKICKFORMATION1\n'+' '.join(map(str,cells))+'\n')
 print(a.output)


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Verify private Q/R compatibility and reject malformed cell-AI option gates."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'old-pack', 'new-pack', 'session', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Temporary data must stay private')
    old=args.old_pack.read_bytes();new=args.new_pack.read_bytes()
    if old[:8]!=b'CMJSHOTQ' or new[:8]!=b'CMJSHOTR' or old[8:]!=new[8:-118]:raise RuntimeError('R pack changed its Q payload')
    stats=ReferenceCartridge(rom[16:262160]);stats.registers[6:8]=[28,29]
    for kind,gate in enumerate(new[-118:]):
        group=stats[0x8e1b+2*kind] | (stats[0x8e1c+2*kind]<<8)
        if gate!=int(stats[group+4]==0 and stats[group+5]==0):raise RuntimeError(f'Incorrect option gate for type {kind}')
    def launch(pack):
        return subprocess.run([str(args.exe.resolve()),str(pack),str(args.session.resolve()),'--demo','1','1'],capture_output=True,text=True)
    if launch(args.old_pack.resolve()).returncode or launch(args.new_pack.resolve()).returncode:raise RuntimeError('Valid Q/R pack rejected')
    with tempfile.TemporaryDirectory(prefix='cell-ai-pack-',dir=args.private_temp) as temporary:
        for name,data in (('truncated',new[:-1]),('trailing',new+b'\0'),('bad-gate',new[:-1]+b'\2')):
            path=Path(temporary)/(name+'.cmjpack');path.write_bytes(data)
            if launch(path).returncode==0:raise RuntimeError(f'Malformed {name} pack accepted')
    print('Q payload preserved; all 118 R option gates matched the ROM; truncated, trailing and invalid-gate packs rejected')


if __name__=='__main__':main()

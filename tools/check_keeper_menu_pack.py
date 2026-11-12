#!/usr/bin/env python3
"""Verify T keeper-menu tables and backward-compatible S payload."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'old-pack', 'new-pack', 'session', 'exe', 'private-temp'): parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args(); rom = args.rom.read_bytes(); old = args.old_pack.read_bytes(); new = args.new_pack.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    if old[:8] != b'CMJSHOTS' or new[:8] != b'CMJSHOTT' or old[8:] != new[8:-6]: raise RuntimeError('T pack changed its S payload')
    memory = ReferenceCartridge(rom[16:262160])
    if new[-6:] != bytes(memory[0xd366:0xd36a]) + bytes(memory[0xd548:0xd54a]): raise RuntimeError('T keeper menu differs from original')
    def launch(path): return subprocess.run([str(args.exe.resolve()), str(path), str(args.session.resolve()), '--demo', '1', '1'], capture_output=True, text=True)
    if launch(args.old_pack.resolve()).returncode or launch(args.new_pack.resolve()).returncode: raise RuntimeError('Valid S/T pack rejected')
    with tempfile.TemporaryDirectory(prefix='keeper-menu-pack-', dir=args.private_temp) as temporary:
        for name, data in (('truncated', new[:-1]), ('trailing', new + b'\0'), ('bad-action', new[:-6] + b'\x03' + new[-5:])):
            path = Path(temporary) / (name + '.cmjpack'); path.write_bytes(data)
            if launch(path).returncode == 0: raise RuntimeError(f'Malformed {name} pack accepted')
    print('S payload preserved; all T keeper menu bytes matched the ROM; malformed packs rejected')


if __name__ == '__main__': main()

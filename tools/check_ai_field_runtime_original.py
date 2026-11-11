#!/usr/bin/env python3
"""Compare runtime candidate2 fallback against constructed original CPU decisions."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from check_ai_field_candidate_original import original_candidate
from check_ai_field_plan_original import original_choice
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'pack', 'session-dir', 'exe', 'private-temp'):
        parser.add_argument('--' + name, required=True, type=Path)
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Temporary original RAM must stay outside the checkout')
    prg, count = rom[16:262160], 0
    with tempfile.TemporaryDirectory(prefix='field-ai-runtime-', dir=args.private_temp) as temp:
        for session in sorted(args.session_dir.glob('*-complete.cmjsession')):
            raw0 = session.with_suffix('.bin').read_bytes()
            for seed in range(1, 2000):
                value = seed
                value ^= (value << 13) & 0xffffffff
                value ^= value >> 17
                value ^= (value << 5) & 0xffffffff
                value &= 0xffffffff
                raw = bytearray(raw0)
                raw[0xe2], raw[0xe3] = value & 255, (value >> 8) & 255
                if original_candidate(prg, raw)[2] != 2:
                    continue
                final = original_choice(prg, raw)
                if final[0x43b] != 2:
                    continue
                output = Path(temp) / 'original.bin'
                output.write_bytes(final)
                command = [str(args.exe.resolve()), '--compare-field-ai-runtime', str(session.resolve()),
                           str(output), str(args.pack.resolve()), str(seed)]
                result = subprocess.run(command, capture_output=True, text=True)
                if result.returncode:
                    raise RuntimeError(f'{session.name} seed {seed}: {result.stderr[-500:]}')
                count += 1
                break
            else:
                raise RuntimeError(f'No original fallback found for {session.name}')
    if not count:
        raise RuntimeError('No matching input sessions found')
    print(f'{count} constructed runtime AI fallback cases matched original')


if __name__ == '__main__':
    main()

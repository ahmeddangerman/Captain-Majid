#!/usr/bin/env python3
"""Check original ball-fraction retention when an ordinary pass target opens."""
import argparse
import hashlib
from pathlib import Path

from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256
from verify_ordinary_pass_initialization import original_target


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', type=Path, required=True)
    parser.add_argument('--ready-dir', type=Path, required=True)
    args = parser.parse_args();rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    prg = rom[16:262160];count = 0
    for fixture in range(4):
        ready = (args.ready_dir / f'{fixture}-ready.bin').read_bytes()
        _, source = trace_recovery(prg, ready, control_input=(64, 0))
        _, menu = trace_menu(prg, source, 'open')
        for x, y in ((48, 80), (64, 96), (108, 108), (127, 132), (160, 140), (207, 175)):
            for xfrac, yfrac in ((0, 255), (1, 127), (127, 1), (255, 0)):
                for player_low in (0, 255):
                    seed = bytearray(menu);seed[0x634], seed[0x636] = xfrac, yfrac
                    result = original_target(prg, seed, x * 256 + player_low, y * 256 + player_low, True)
                    if result[:2] != (x, y) or result[-2:] != (xfrac, yfrac):
                        raise RuntimeError(f'Original target changed fractions: fixture={fixture}, position={x}/{y}')
                    count += 1
    print(f'{count} constructed original pass-target openings preserved ball fractions while refreshing coordinates')


if __name__ == '__main__':
    main()

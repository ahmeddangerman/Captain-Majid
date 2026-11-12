#!/usr/bin/env python3
"""Compare a constructed ordinary-pass flight defender with original CPU RAM.

The original trace injects the represented native field state at the start of
flight and acknowledges CPU waits; it is not an uninterrupted game or timing test.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_captured_pass_defenses import compare_flight_setup, compare_control_entry
from verify_player7_pass_end_to_end import compare_represented


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original',type=Path,required=True)
    p.add_argument('--native',type=Path,required=True)
    p.add_argument('--nonterminal-original',type=Path)
    p.add_argument('--nonterminal-native',type=Path)
    p.add_argument('--special-original',type=Path)
    p.add_argument('--special-native',type=Path)
    a=p.parse_args();o=a.original;n=a.native
    raw=(o/'e773-2.bin').read_bytes()
    if len(raw)!=2048 or (raw[0x600],raw[0x601])!=(1,16):
        raise ValueError('Expected one selected flight defender, player16')
    compare_flight_setup((n/'345-1.cmjsession').read_bytes(),raw,'flight',345,'selection')
    for i in range(1,4):
        compare_flight_setup((n/f'346-{i+1}.cmjsession').read_bytes(),
                             (o/f'e773-{i+2}.bin').read_bytes(),'flight',346,'next-cell')
    raw=(o/'88e5-1.bin').read_bytes()
    if (raw[0x5fb],raw[0x441],raw[0x600],raw[0x601])!=(11,16,1,16):
        raise ValueError('Expected defender16 possession after result3')
    native=(n/'1-1.cmjsession').read_bytes()
    compare_represented(native,export_session(raw,1),False,'flight','player-control')
    if bool(a.nonterminal_original)!=bool(a.nonterminal_native) or bool(a.special_original)!=bool(a.special_native):
        p.error('Original and native arguments must be supplied together')
    if a.nonterminal_original:
        o,n=a.nonterminal_original,a.nonterminal_native
        raw=(o/'888a-1.bin').read_bytes()
        if (raw[0x600],raw[0x442],raw[0x612])!=(0,16,0):raise ValueError('Expected completed nonterminal defender16 contact')
        compare_flight_setup((n/'347-2.cmjsession').read_bytes(),raw,'flight',347,'nonterminal')
        compare_control_entry((n/'350-1.cmjsession').read_bytes(),(o/'e0df-1.bin').read_bytes(),'flight')
    if a.special_original:
        o,n=a.special_original,a.special_native
        decision=(o/'8874-1.bin').read_bytes()
        if (decision[0x43e],decision[0x612])!=(128,3):raise ValueError('Expected observed special result3')
        raw=(o/'88e5-1.bin').read_bytes()
        if (raw[0x5fb],raw[0x441])!=(11,16):raise ValueError('Expected defender16 special-result possession')
        compare_represented((n/'1-1.cmjsession').read_bytes(),export_session(raw,1),False,'flight','special-control')
    print('Matched selected defender16, three flight cells and represented opponent control; optional nonterminal and special paths checked when supplied.')


if __name__=='__main__':main()

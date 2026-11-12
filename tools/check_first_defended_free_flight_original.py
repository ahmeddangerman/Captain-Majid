#!/usr/bin/env python3
"""Compare a constructed two-defender free-pass arrival to original CPU captures.

Original RAM was replaced at the flight boundary with represented native state;
nonrepresented CPU/presentation context and cooperative waits are retained.
The source captures and native sessions remain in a private directory.
"""
import argparse
from pathlib import Path
from verify_captured_pass_defenses import compare_flight_setup, compare_free_winner, compare_control_entry


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original',type=Path,required=True)
    p.add_argument('--native',type=Path,required=True)
    a=p.parse_args()
    def native(name):return (a.native/(name+'.cmjsession')).read_bytes()
    def original(name):
        raw=(a.original/(name+'.bin')).read_bytes()
        if len(raw)!=2048:raise ValueError('Expected private 2048-byte original capture')
        return raw
    first=original('8845-1')
    if (first[0x600],first[0x601],first[0x602],first[0x616],first[0x5fc])!=(2,16,18,0,255):
        raise ValueError('Unexpected defended free-pass arrival')
    for state,boundary in [('native-1-347','8845-1'),('native-2-347','8845-2'),('native-3-347','888a-1')]:
        compare_flight_setup(native(state),original(boundary),'defended free pass',347,boundary)
    compare_free_winner(native('native-4-351'),original('df0d-1'),'defended free pass','winner')
    compare_control_entry(native('native-5-350'),original('e0df-1'),'defended free pass')
    print('Matched arrival, both defender contacts, player2 race winner and control entry')

if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare a controlled one-defender pass replay with private original checkpoints.

The trace must document the defender-list and random interventions. This checks
represented state at fixed CPU boundaries; it does not emulate NMI or render.
"""
import argparse
from pathlib import Path
from verify_captured_pass_defenses import (
    compare_first_defender, compare_first_defender_result, compare_first_contact,
    compare_dispatch_clock, compare_movement, compare_flight_target,
    compare_flight_setup, compare_control_entry,
)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('original','native','ownership'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();o,n,ownership=args.original,args.native,args.ownership
    target=(o/'target-before-intervention.bin').read_bytes()
    def native(phase,index=1):return (n/f'{phase}-{index}.cmjsession').read_bytes()
    def original(name, normalize_cursor=False):
        raw=bytearray((o/(name+'.bin')).read_bytes())
        if len(raw)!=2048:raise ValueError('Expected 2048-byte original RAM')
        if normalize_cursor:
            if raw[0x616]!=1:raise ValueError('Original did not increment final defender cursor')
            raw[0x616]=0
        return raw
    comparison=original('80d6-1')
    if (target[0x600],target[0x601],target[0x638],target[0x5fc])!=(3,17,103,7):
        raise ValueError('Unexpected pre-intervention target fixture')
    prepared=original('80b6-1')
    if (prepared[0x600],prepared[0x601],prepared[0x442],prepared[0xe2],prepared[0xe3])!=(1,17,17,9,0):
        raise ValueError('Expected documented one-defender/random intervention')
    compare_first_defender(native(336),prepared,target,1)
    compare_first_defender_result(native(337),comparison,target,1,255,0,0)
    compare_first_contact(native(338),original('80dc-1'),target,1,255,0,0)
    compare_dispatch_clock(native(342),original('80ed-1',True),target,1,255,0,0,0)
    compare_movement(native(343),original('c612-1',True),comparison,target,1,255,0,0,0)
    compare_flight_target(native(344),original('de64-1',True),comparison,target,1,255,0,0,0)
    compare_flight_setup(native(345),original('e773-1'),'one-defender')
    for index in range(1,5):
        compare_flight_setup(native(346,index),original(f'e773-{index+1}'),'one-defender',346,'flight.cell')
    compare_flight_setup(native(348),(ownership/'owner-1.bin').read_bytes(),'one-defender',348,'reception.owner')
    balls=list(ownership.glob('ball-*.bin'))
    if not balls:raise ValueError('Missing observed ball-position writes')
    last=max(balls,key=lambda p:int(p.stem.split('-')[1]))
    compare_flight_setup(native(349),last.read_bytes(),'one-defender',349,'reception.position')
    compare_control_entry(native(350),original('e0df-1'),'one-defender')
    print('Matched 14 original/native boundaries through player7 control entry; defender cursor normalized after dispatch. Original CPU waits acknowledged; random values and one-defender list intervened.')

if __name__=='__main__':main()

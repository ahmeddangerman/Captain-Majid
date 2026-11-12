#!/usr/bin/env python3
"""Compare controlled action-0 pass-defense checkpoints with original CPU RAM.

The original defender list and random values were explicitly intervened and
presentation waits acknowledged. No live timing or rendering is simulated.
"""
import argparse
from pathlib import Path
from verify_captured_pass_defenses import (
    compare_first_defender, compare_first_defender_result, compare_first_contact,
    compare_dispatch_clock, compare_movement, compare_flight_target,
    compare_flight_setup, compare_control_entry,
)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('original','native-prefix','native-continuation'):
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();o=a.original;n=a.native_continuation
    target=(o/'target-before-intervention.bin').read_bytes()
    if (target[0x600],target[0x601],target[0x638],target[0x5fc])!=(3,17,103,7):
        raise ValueError('Unexpected pre-intervention fixture')
    def original(name,normalize=False):
        raw=bytearray((o/(name+'.bin')).read_bytes())
        if len(raw)!=2048:raise ValueError('Expected original RAM')
        if normalize:
            if raw[0x616]!=1:raise ValueError('Original cursor was not incremented')
            raw[0x616]=0
        return raw
    def block(path):
        raw=bytearray(path.read_bytes())
        if raw[401]!=0:raise ValueError('Native contact is not action 0')
        # Existing helper expects action 1 in this byte; all other bytes,
        # including the recorded action-0 choice, are compared unchanged.
        raw[401]=1
        return raw
    prepared=original('80b6-1');comparison=original('80d6-1')
    if (prepared[0x600],prepared[0x601],prepared[0x442],prepared[0x60b],prepared[0xe2],prepared[0xe3])!=(1,17,17,0,6,0):
        raise ValueError('Unexpected defender, action or random intervention')
    compare_first_defender(block(Path(str(a.native_prefix)+'-prepared.cmjsession')),prepared,target,1)
    compare_first_defender_result(block(Path(str(a.native_prefix)+'-compared.cmjsession')),comparison,target,1,255,0,0)
    def native(phase,index=1):return block(n/f'{phase}-{index}.cmjsession')
    compare_first_contact(native(338),original('80dc-1'),target,1,255,0,0)
    compare_dispatch_clock(native(342),original('80ed-1',True),target,1,255,0,0,0)
    compare_movement(native(343),original('c612-1',True),comparison,target,1,255,0,0,0)
    compare_flight_target(native(344),original('de64-1',True),comparison,target,1,255,0,0,0)
    compare_flight_setup(native(345),original('e773-1'),'block')
    for i in range(1,5):compare_flight_setup(native(346,i),original(f'e773-{i+1}'),'block',346,'flight.cell')
    compare_control_entry(native(350),original('e0df-1'),'block')
    print('Matched 12 action-0 pass-defense boundaries through player7 control entry; original cursor normalized after dispatch.')


if __name__=='__main__':main()

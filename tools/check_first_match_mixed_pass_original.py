#!/usr/bin/env python3
"""Compare first-fixture tackle/block pass boundaries with private original RAM.

Defender choice random bytes are deliberately set; preparation random bytes are
then reset. Later loop cursors are normalized only for the older helper API.
Presentation waits are acknowledged rather than emulated.
"""
import argparse
from pathlib import Path
from verify_captured_pass_defenses import (compare_defenses, compare_first_defender,
    compare_first_defender_result, compare_first_contact, compare_dispatch_clock)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--original',type=Path,required=True)
    parser.add_argument('--native',type=Path,required=True)
    args=parser.parse_args();original=args.original;native=args.native
    target=(original/'target-before-intervention.bin').read_bytes()
    def raw(name,cursor=None):
        data=bytearray((original/(name+'.bin')).read_bytes())
        if len(data)!=2048:raise ValueError('Expected private original RAM')
        data[0xe2]=data[0xe3]=0
        if cursor is not None:
            if data[0x616]!=cursor:raise ValueError('Unexpected original cursor')
            data[0x616]=0
        return data
    def state(name,block=False):
        data=bytearray((native/(name+'.cmjsession')).read_bytes())
        if block:
            if data[401]!=0:raise ValueError('Expected action0 contact')
            data[401]=1  # Adapt only the helper contact-action assertion.
        return data
    if (raw('8637-2')[0x60b],raw('8637-2')[0x60c])!=(1,0):raise ValueError('Expected tackle then block')
    for name,boundary,phase,count in [('first','8637-1',333,1),('second','8637-2',334,2),('attack','804f-1',335,2)]:
        compare_defenses(state(name),raw(boundary),target,phase,count)
    for index in (0,1):
        number=index+1;cursor=index if index else None
        compare_first_defender(state(f'prepared{number}',bool(index)),raw(f'80b6-{number}',cursor),target,2,index)
        compare_first_defender_result(state(f'compared{number}',bool(index)),raw(f'80d6-{number}',cursor),target,2,255,0,0,index)
        compare_first_contact(state(f'contact{number}',bool(index)),raw(f'80dc-{number}',cursor),target,2,255,0,0,index)
    compare_dispatch_clock(state('dispatch',True),raw('80ed-1',2),target,2,255,0,0,1)
    print('Matched two choices, attacker preparation, six defender boundaries and dispatch')

if __name__=='__main__':main()

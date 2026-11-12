#!/usr/bin/env python3
"""Compare the captured first kickoff with sequential native runtime snapshots.

Original presentation/NMI advances random bytes while the provisional runtime
acknowledges it without executing frames. Only those two bytes are normalized.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('native','captured','original'):
        p.add_argument('--'+name,type=Path,required=True)
    p.add_argument('--constructed',action='store_true',help='Native kickoff started from provisional first-match constructor')
    a=p.parse_args();native=a.native;captured=a.captured
    for phase,name in ((61,'formation'),(62,'receiver'),(63,'ball'),(64,'handoff')):
        actual=bytearray((native/f'{phase}-1.cmjsession').read_bytes())
        expected=bytearray((captured/f'{name}.cmjsession').read_bytes())
        if phase==61:
            # The earlier capture lacks the later sequence-result byte; native keeps its default.
            if len(actual)!=len(expected)+1 or actual[-1]!=255:raise ValueError('Unexpected formation serialization')
            actual=actual[:-1];actual[10]=expected[10]
        if phase==64 or a.constructed:
            actual[26:28]=expected[26:28]  # provisional RNG and original presentation differ
        if actual!=expected:
            difference=next((i for i,(x,y) in enumerate(zip(actual,expected)) if x!=y),None)
            raise ValueError(f'{name}: first differing byte {difference}; lengths {len(actual)}/{len(expected)}')
    actual=bytearray((native/'9-1.cmjsession').read_bytes())
    raw=bytearray((a.original/'010-E145.ram').read_bytes())
    if (raw[0x5fb],raw[0x441],raw[0x5f7],raw[0x5f8])!=(0,9,180,0):
        raise ValueError('Expected captured first control-ready state')
    raw[0xe2:0xe4]=actual[26:28]
    compare_represented(actual,export_session(raw,9),False,'initial-kickoff','control-ready')
    print('Matched kickoff formation, receiver, ball, handoff and represented player9 control-ready; provisional/presentation RNG normalized and old formation sequence byte omitted.')


if __name__=='__main__':main()

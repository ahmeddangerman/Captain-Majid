#!/usr/bin/env python3
"""Compare eight provisional fresh starts with original emulator kickoff captures.

The emulator traces intervene on the kickoff random byte. Native and original
random values differ during acknowledged presentation; only those bytes are
normalized. Each native directory contains sequential runtime snapshots.
"""
import argparse
from pathlib import Path
from check_kickoff_session_original import snapshot
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original-root',type=Path,required=True)
    p.add_argument('--native-root',type=Path,required=True)
    a=p.parse_args();receivers=[]
    for seed in range(1,9):
        original=a.original_root/f'built-first-kickoff-rng-{seed&7}-20261112'
        native=a.native_root/f'first-match-seed{seed}-native-20261112'
        def capture(pc):
            files=sorted(original.glob(f'*-{pc}.ram'))
            if not files:raise ValueError(f'Missing original {pc} for seed {seed}')
            raw=bytearray(files[0].read_bytes())
            if len(raw)!=2048:raise ValueError('Expected 2048-byte original RAM')
            return raw
        for phase,pc in ((61,'DAF5'),(62,'DB0B'),(63,'DB0E'),(64,'DB1E')):
            actual=(native/f'{phase}-1.cmjsession').read_bytes();raw=capture(pc)
            raw[0xe2:0xe4]=actual[26:28]
            expected=bytearray(snapshot(raw,61));expected[258]=phase
            if phase>=62:expected[368:370]=bytes((1,raw[0x5fc]))
            if len(actual)!=len(expected)+1 or actual[-1]!=255:
                raise ValueError(f'Unexpected native serialization for seed {seed}, phase {phase}')
            expected.append(255);expected[10]=actual[10]
            if actual!=expected:raise ValueError(f'Kickoff state mismatch for seed {seed}, phase {phase}')
        raw=capture('E145');actual=(native/'9-1.cmjsession').read_bytes()
        receivers.append(raw[0x441]);raw[0xe2:0xe4]=actual[26:28]
        compare_represented(actual,export_session(raw,9),False,str(seed),'first-control')
    if set(receivers)!={5,6,7,9,10}:raise ValueError('Not all five captured receivers were exercised')
    print('Eight controlled original kickoff choices matched native formation, handoff and represented control for all five receivers; random bytes normalized.')


if __name__=='__main__':main()

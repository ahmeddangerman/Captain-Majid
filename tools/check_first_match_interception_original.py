#!/usr/bin/env python3
"""Compare the first-fixture flight interception with private original captures.

The original flight boundary used an explicitly injected represented native
state; CPU and presentation waits are cooperative, not live hardware timing.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_checkpoint, compare_represented


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original',type=Path,required=True)
    p.add_argument('--native',type=Path,required=True)
    a=p.parse_args();raw=(a.original/'88e5-1.bin').read_bytes()
    if len(raw)!=2048 or (raw[0x5fb],raw[0x441],raw[0x600],raw[0x601],raw[0x602],raw[0x43e])!=(11,16,2,16,18,128):
        raise ValueError('Unexpected defended free-flight interception')
    state=(a.native/'native-1-1.cmjsession').read_bytes()
    compare_represented(state,export_session(raw,1),False,'first fixture','interception')
    ready=(a.native/'native-2-9.cmjsession').read_bytes();control=(a.original/'e145-1.bin').read_bytes()
    compare_checkpoint(ready,control,9,9,'first fixture','opponent control',None,False,False)
    print('Matched defender16 interception and opponent control-ready')

if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare a constructed first-fixture opponent shot and save to original CPU.

The native represented field state and defender skip are injected at the
original preparation boundary, with controlled RNG and acknowledged waits.
No live rendering, NMI, sound or hardware timing is claimed.
"""
import argparse
from pathlib import Path
from export_phase_session import export_session
from verify_player7_pass_end_to_end import compare_represented


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('shot','keeper','native'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    def compare(native,original,boundary,keeper_selected=False):
        state=native.read_bytes();raw=original.read_bytes()
        if len(raw)!=2048:raise ValueError('Expected private original RAM')
        r=bytearray(raw)
        if keeper_selected:r[0x600]=0
        expected=bytearray(export_session(r,0))
        if keeper_selected:expected[28]=raw[0x600]
        compare_represented(state,expected,False,'first fixture',boundary)
        return state,raw
    for name,boundary in [('prepared','804f-1'),('clock','80ed-1'),('moved','c618-1'),('target','dd08-1')]:
        state,raw=compare(a.native/('first-defended-shot-'+name+'-20261112.cmjsession'),a.shot/(boundary+'.bin'),name)
        if name=='target' and tuple(state[340:344])!=(raw[0x62b],raw[0x638],raw[0x61a],0):raise ValueError('Shot target differs')
    for state_name,boundary in [('native-0-47','e773-1'),('native-1-48','e773-2'),('native-3-48','e773-3'),('native-5-48','e773-4'),('native-7-48','e773-5'),('native-9-49','e7cc-1'),('native-11-53','8509-1')]:
        state,raw=compare(a.shot/(state_name+'.cmjsession'),a.shot/(boundary+'.bin'),boundary,boundary=='8509-1')
        wanted=bytes((1,raw[0x62b],raw[0x638],raw[0x61a],0,1))+raw[0x634:0x638]+raw[0x639:0x63d]+bytes((raw[0x62c],raw[0x61b]))
        if state[339:355]!=wanted:raise ValueError('Shot flight trajectory differs at '+boundary)
    for index,boundary in enumerate(('open','selected','compared','save','recovery','continued')):
        compare(a.keeper/(f'native-{index}.cmjsession'),a.keeper/(boundary+'.bin'),boundary,index<3)
    compared=(a.keeper/'compared.bin').read_bytes();recovered=(a.keeper/'recovery.bin').read_bytes()
    if (compared[0x43d],compared[0x612],recovered[0x441],recovered[0x5fb])!=(0,2,17,11):
        raise ValueError('Unexpected goalkeeper choice, result or recovery owner')
    print('Matched shot preparation, clock, movement, target, flight, goalkeeper menu, save and opponent17 recovery')

if __name__=='__main__':main()

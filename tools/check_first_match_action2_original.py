#!/usr/bin/env python3
"""Compare the first-fixture action-2 pass against private original captures.

The target RAM and neutral inputs are constructed from an original capture;
random bytes at target confirmation are intervened, and cooperative waits and
presentation-facing are acknowledged. Cursor normalization is limited to the
comparison helper's older single-cursor projection. This is not live NES timing.
"""
import argparse
from pathlib import Path
from verify_captured_pass_defenses import (compare_defenses, compare_first_defender,
    compare_first_defender_result, compare_first_contact, compare_dispatch_clock,
    compare_movement, compare_flight_target, compare_flight_setup, compare_free_winner,
    compare_control_entry)
from verify_player7_pass_end_to_end import compare_checkpoint


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--original',type=Path,required=True)
    p.add_argument('--native',type=Path,required=True)
    a=p.parse_args();o=a.original;n=a.native
    target=(o/'target-before-intervention.bin').read_bytes()
    if (target[0x441],target[0x600],*target[0x601:0x603],target[0x638],target[0x5fc])!=(9,2,19,20,130,255):
        raise ValueError('Unexpected first-fixture target')
    def raw(name,cursor=None):
        data=bytearray((o/(name+'.bin')).read_bytes())
        if len(data)!=2048:raise ValueError('Expected private original RAM')
        if cursor is not None:
            if data[0x616]!=cursor:raise ValueError('Unexpected original cursor')
            data[0x616]=0
        return data
    def native(name,action=False):
        data=bytearray((n/(name+'.cmjsession')).read_bytes())
        if action:
            if data[401]!=2:raise ValueError('Expected action2 contact')
            # Existing helper checks the separately represented contact action
            # against action1; recorded defense bytes remain action2.
            data[401]=1
        return data
    for name,original,phase,count in [('native-first','8637-1',333,1),('native-second','8637-2',334,2),('native-attack','804f-1',335,2)]:
        compare_defenses(native(name),raw(original),target,phase,count)
    for slot in (0,1):
        number=slot+1;normalized=slot if slot else None
        compare_first_defender(native(f'native-prepared{number}',True),raw(f'80b6-{number}',normalized),target,2,slot)
        compare_first_defender_result(native(f'native-compared{number}',True),raw(f'80d6-{number}',normalized),target,2,255,0,2,slot)
        compare_first_contact(native(f'native-contact{number}',True),raw(f'80dc-{number}',normalized),target,2,255,0,2,slot)
    compare_dispatch_clock(native('native-dispatch',True),raw('80ed-1',2),target,2,255,0,2,1)
    comparison=raw('80d6-2')
    compare_movement(native('runtime4-1-343',True),raw('c612-1',2),comparison,target,2,255,0,2,1)
    compare_flight_target(native('runtime4-2-344',True),raw('de64-1',2),comparison,target,2,255,0,2,1)
    for index,step in enumerate((3,4,6,8,10)):
        phase=345 if index==0 else 346
        compare_flight_setup(native(f'runtime4-{step}-{phase}'),raw(f'e773-{index+1}'),'first fixture',phase)
    compare_free_winner(native('runtime4-12-351'),raw('df0d-1'),'first fixture','free winner')
    compare_control_entry(native('runtime4-13-350'),raw('e0df-1'),'first fixture')
    for name,original,phase in [('runtime4-14-9','e145-1',9),('runtime4-25-10','e1e7-1',10),('runtime4-26-13','e1fa-1',13)]:
        state=native(name);original_ram=raw(original)
        # Provisional runtime PRNG and original host waits have different schedules.
        original_ram[0xe2],original_ram[0xe3]=state[26],state[27]
        compare_checkpoint(state,original_ram,phase,19,'first fixture',original,None,False,False)
    print('Matched two choices, attack, six defender boundaries, dispatch, movement, target, five flight boundaries, player2 winner, control entry and three control/clock boundaries; helper cursor and provisional RNG normalized as documented.')

if __name__=='__main__':main()

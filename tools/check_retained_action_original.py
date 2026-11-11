#!/usr/bin/env python3
"""Check preservation of the original defensive-action byte through bounded control paths."""
import argparse
import hashlib
from collections import Counter
from pathlib import Path

from trace_action_menu import trace_menu
from trace_ball_recovery_phase import trace_recovery
from trace_field_loop import trace_loop
from check_empty_pass_original import observe
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom',type=Path,required=True)
    parser.add_argument('--ready-dir',type=Path,action='append',required=True)
    parser.add_argument('--include-empty-pass',action='store_true',help='Also trace empty-target passes to resumed control')
    args=parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected verified Japanese ROM')
    prg=rom[16:262160];counts=Counter()
    for directory in args.ready_dir:
        for fixture in range(4):
            ready=(directory/f'{fixture}-ready.bin').read_bytes()
            for action in range(7):
                base=bytearray(ready);base[0x43d]=action
                for held,pressed in ((0,0),(1,0),(2,0),(4,0),(8,0),(64,0),(0,128)):
                    report,after=trace_recovery(prg,base,control_input=(held,pressed))
                    if after[0x43d]!=action:raise RuntimeError(f'Control poll changed action: {directory.name}/{fixture}/{action}/{held}/{pressed}')
                    counts['poll:'+report['boundary']]+=1
                report,clock=trace_recovery(prg,base,control_tick=True)
                if clock[0x43d]!=action:raise RuntimeError(f'Control clock changed action: {directory.name}/{fixture}/{action}')
                counts['clock:'+report['boundary']]+=1
                report,world=trace_recovery(prg,clock,world_inputs=[(0,0)]*22)
                if world[0x43d]!=action:raise RuntimeError(f'Field round changed action: {directory.name}/{fixture}/{action}')
                counts['field:'+report['boundary']]+=1
                if base[0x5fb]==0:
                    _,menu_input=trace_recovery(prg,base,control_input=(64,0))
                    _,opened=trace_menu(prg,menu_input,'open')
                    _,choice=trace_menu(prg,opened,'poll',129)
                    if opened[0x43d]!=action or choice[0x43d]!=action:raise RuntimeError('Human shot menu changed retained action')
                    counts['human-menu']+=1
                if args.include_empty_pass and base[0x5fb]==0:
                    for direction in (0,1,2,4,8):
                        same_cell=direction==0
                        handoff,target,arrival,final,boundary,_=observe(prg,base,104,100,same_cell=same_cell,direction=direction or 1)
                        if arrival[0x600] or boundary!=0xdf0d:
                            counts['empty-pass-skipped']+=1;continue
                        if any(snapshot[0x43d]!=action for snapshot in (handoff,target,arrival,final)):
                            raise RuntimeError(f'Empty pass changed action: {directory.name}/{fixture}/{action}/{direction}')
                        control=bytearray(2048)
                        trace_loop(prg,final,race_tail='winner',ack_ppu_transfers=True,final_ram=control)
                        _,ready_again=trace_recovery(prg,control,prepare_control=True)
                        if control[0x43d]!=action or ready_again[0x43d]!=action:
                            raise RuntimeError(f'Empty pass control changed action: {directory.name}/{fixture}/{action}/{direction}')
                        counts['empty-pass-to-control']+=1
    checked=sum(count for name,count in counts.items() if not name.endswith('-skipped'))
    print(f'Original action byte remained unchanged in {checked} constructed boundaries: {dict(sorted(counts.items()))}')


if __name__=='__main__':main()

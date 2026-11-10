#!/usr/bin/env python3
"""Replay the captured player2 pass confirmation through target initialization."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_captured_pass_target import compare_target
from verify_player7_pass_end_to_end import compare_checkpoint, prepare_output


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    start, end = 20547, 20637
    if [pcs[start],pcs[end]] != ['D67C','D80A']:
        raise ValueError('Unexpected captured player2 pass boundaries')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    compare_checkpoint(args.source.read_bytes(),ram(start),19,19,'player2','pass confirmation',None,False,False)
    rows, last = menu_wait_events(lines,pcs,ram,start,end)
    points = [i for i in range(start+1,end) if pcs[i] in ('C9C5','E3CA')]
    if last!=20636 or len(rows)!=len(points):
        raise ValueError('Ambiguous player2 pass presentation ordering')
    values={'G':ram(start)[0x443],'M':ram(start)[0x532]}; samples=[]; ordered=[]
    for row,point in zip(rows,points):
        observed=ram(point)
        for token,address in (('G',0x443),('M',0x532)):
            value=observed[address]
            if value!=values[token]:
                values[token]=value; samples.append((token,value)); ordered.append(f'{token} {value}')
        ordered.append(row)
    if samples!=[('G',0),('M',23),('M',0)]:
        raise ValueError(f'Unexpected player2 pass presentation samples: {samples}')
    events=output/'pass-wait-events.txt'; events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
    waiting=output/'pass-wait.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),
                        '--advance-observed-nmi-sequence',str(events),str(ram(start)[0xe1]),'--capture',str(waiting)],
                       capture_output=True,text=True,timeout=30)
    if run.returncode: raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);',run.stdout)!=[str(ram(last)[0xe1])]:
        raise ValueError('Player2 pass wait random index differs')
    compare_checkpoint(waiting.read_bytes(),ram(last),19,19,'player2','pass wait',None,False,False)
    target=output/'pass-target.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),
                        '--begin-ordinary-human-pass-target','--capture',str(target)],capture_output=True,text=True,timeout=30)
    if run.returncode: raise RuntimeError(run.stderr.strip())
    compare_target(target.read_bytes(),ram(end),expected_sequence=2)
    print(f'Matched player2 pass target initialization; random index {ram(end)[0xe1]}.')


if __name__=='__main__':
    try: main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Player2 pass target replay stopped: {error}',file=sys.stderr);sys.exit(1)

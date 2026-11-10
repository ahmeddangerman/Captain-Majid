#!/usr/bin/env python3
"""Compare captured player2 empty-pass defender choice and attack preparation."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_captured_pass_target import compare_confirmed_target
from verify_player7_pass_end_to_end import prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--comparison-content',nargs=7,type=int,required=True,
                        metavar=('RANGE','OFFSET','ELIGIBLE','T0','T1','T2','T3'))
    args=parser.parse_args()
    lines,pcs,ram=trace_data(args.trace_directory)
    if [pcs[i] for i in (20828,20869,20875,20877)]!=['D84B','D716','804C','804F']:
        raise ValueError('Unexpected player2 pass defender boundaries')
    output=prepare_output(args.output,Path(__file__).resolve().parents[1])
    source=args.source.resolve(); index=ram(20828)[0xe1]; operation=0
    compare_confirmed_target(source.read_bytes(),ram(20828),expected_sequence=2)

    def run(option,*values):
        nonlocal source,operation
        operation+=1; destination=output/f'{operation:03d}.cmjsession'
        result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),option,
                               *map(str,values),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if result.returncode: raise RuntimeError(result.stderr.strip())
        source=destination
        return result.stdout

    def wait(start,stop):
        nonlocal index
        rows,last=menu_wait_events(lines,pcs,ram,start,stop)
        events=output/f'{operation+1:03d}-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        result=run('--advance-observed-nmi-sequence',events,index)
        if re.findall(r'final random index (\d+);',result)!=[str(ram(last)[0xe1])]:
            raise ValueError('Player2 pass defender wait random index differs')
        index=ram(last)[0xe1]
        return last

    def compare_defenses(at,phase,count,data=None):
        actual=source.read_bytes() if data is None else data
        if len(actual)!=440 or actual[10]!=150 or int.from_bytes(actual[258:260],'little')!=phase:
            raise ValueError('Unexpected player2 pass defense snapshot')
        wanted=bytes((count,1 if count else 0,0,0,0,0,0,0,0,0,0))
        if actual[-11:]!=wanted:
            raise ValueError('Player2 recorded defender choice differs')
        normalized=bytearray(actual[:-11]);normalized[10]=149;normalized[258:260]=(332).to_bytes(2,'little')
        compare_confirmed_target(normalized,ram(at),expected_sequence=2)

    last=wait(20828,20869)
    compare_confirmed_target(source.read_bytes(),ram(last),expected_sequence=2)
    run('--begin-ordinary-pass-defenses')
    compare_defenses(20869,333,0)
    last=wait(20869,20875)
    compare_defenses(last,333,0)
    if (ram(20875)[0x442],ram(20875)[0x60b],ram(20875)[0x606])!=(21,1,0):
        raise ValueError('Unexpected captured defender choice')
    run('--record-ordinary-pass-defense',21,1,0)
    compare_defenses(20875,334,1)
    run('--prepare-ordinary-pass')
    compare_defenses(20877,335,1)
    rows,last=menu_wait_events(lines,pcs,ram,20877,21037)
    points=[i for i in range(20878,21037) if pcs[i] in ('C9C5','E3CA')]
    if last!=21035 or len(rows)!=len(points): raise ValueError('Ambiguous pass defender presentation ordering')
    addresses=(('L',0x617),('M',0x532),('C',0x616),('F',0x517))
    values={token:ram(20877)[address] for token,address in addresses};samples=[];ordered=[]
    for row,point in zip(rows,points):
        for token,address in addresses:
            value=ram(point)[address]
            if value!=values[token]: values[token]=value;samples.append((token,value));ordered.append(f'{token} {value}')
        ordered.append(row)
    if samples!=[('L',0),('M',0),('C',0),('F',64)]: raise ValueError(f'Unexpected pass defender samples: {samples}')
    events=output/f'{operation+1:03d}-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
    result=run('--advance-observed-nmi-sequence',events,index)
    if re.findall(r'final random index (\d+);',result)!=[str(ram(last)[0xe1])]: raise ValueError('Pass defender presentation random index differs')
    index=ram(last)[0xe1];compare_defenses(last,335,1)
    run('--prepare-first-ordinary-pass-defender',ram(21037)[0x44e])
    actual=source.read_bytes();observed=ram(21037)
    expected=bytes((1,0,1,0,observed[0x444],observed[0x32],observed[0x33],observed[0x43f],observed[0x440],0,0,0,0,0))
    if actual[399:413]!=expected or int.from_bytes(actual[258:260],'little')!=336 or actual[414]!=255:
        raise ValueError('Player2 pass defender preparation differs')
    normalized=bytearray(actual);normalized[399:413]=bytes(14);normalized[414]=2;normalized[258:260]=(335).to_bytes(2,'little')
    compare_defenses(21037,335,1,normalized)
    run('--compare-first-ordinary-pass-defender',*args.comparison_content)
    actual=source.read_bytes();original=ram(21040)
    registers=re.search(r'\bA=([0-9A-F]{2}).*\bY=([0-9A-F]{2})',lines[21040])
    if registers is None: raise ValueError('Missing pass defender original result registers')
    score,result=(int(value,16) for value in registers.groups())
    if int.from_bytes(actual[258:260],'little')!=337 or actual[408:413]!=bytes((original[0x71],original[0x619],score,result,0)) or actual[414]!=result:
        raise ValueError('Player2 pass defender comparison differs')
    normalized=bytearray(actual);normalized[399:413]=bytes(14);normalized[414]=2;normalized[258:260]=(335).to_bytes(2,'little')
    compare_defenses(21040,335,1,normalized)
    print(f'Matched player2 pass defender21 result {result}; random index {index}.')


if __name__=='__main__':
    try: main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Player2 pass defender replay stopped: {error}',file=sys.stderr);sys.exit(1)

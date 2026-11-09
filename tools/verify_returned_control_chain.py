#!/usr/bin/env python3
"""Replay captured control polls, clocks and field rounds after a pass.

All paths are caller-supplied. Original random inputs, travel speeds and event
ordering are evidence; this tool does not generate frames or presentation.
"""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import clock_events, trace_data
from verify_observed_nmi_world_round import read_nmi_trace
from verify_player7_pass_end_to_end import compare_checkpoint, prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--start-frame',type=int,required=True)
    parser.add_argument('--initial-index',type=int,required=True)
    parser.add_argument('--expected-rounds',type=int,required=True)
    parser.add_argument('--expected-polls',type=int,required=True)
    args=parser.parse_args()
    if not 0<=args.initial_index<=255 or args.expected_rounds<1 or args.expected_polls<1:
        raise ValueError('Random index and expected counts are invalid')
    lines,pcs,ram=trace_data(args.trace_directory)
    start=next(i for i,line in enumerate(lines) if pcs[i]=='E145' and line.split()[-1]==f'FRAME={args.start_frame}')
    source=args.source.resolve();index=args.initial_index
    if index!=ram(start)[0xe1]:
        raise ValueError(f'boundary returned.start, random.index: native {index}, original {ram(start)[0xe1]}')
    compare_checkpoint(source.read_bytes(),ram(start),9,9,'startup','returned.start',None,False,False)
    output=prepare_output(args.output,Path(__file__).resolve().parents[1])
    counts={'poll':0,'clock':0,'world':0};steps=0
    for at in range(start+1,len(pcs)):
        pc=pcs[at]
        if pc not in ('E14A','E1E7','E407'):
            continue
        if pc=='E14A':
            kind='poll';counts[kind]+=1
            entry=next(i for i in range(at-1,-1,-1) if pcs[i]=='E145')
            end=next(i for i in range(at+1,len(pcs)) if pcs[i] in ('E145','E1E7'))
            phase=352 if pcs[end]=='E1E7' else 9
            group=[i for i in range(entry+1,at) if pcs[i] in ('C9C5','C9E8','E3CA','E3DE','E406')]
            names=[pcs[i] for i in group]
            if names not in (['C9C5','C9E8','E3CA','E406'],['C9C5','C9E8','E3CA','E3DE','E406']):
                raise ValueError(f'boundary returned.poll[{counts[kind]}]: unsupported random/travel ordering')
            nmi,travel,poll=ram(group[0]),ram(group[2]),ram(at)
            active=travel[0x5fb]!=0 or bool(travel[0x1c]&15)
            if active!=(len(group)==5):
                raise ValueError(f'boundary returned.poll[{counts[kind]}]: inconsistent speed evidence')
            speed=0 if not active else ram(group[3])[0x32] | (ram(group[3])[0x33]<<8)
            flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[group[0]])
            if flags is None or nmi[0xe1]!=index:
                raise ValueError(f'boundary returned.poll[{counts[kind]}]: missing carry or random index mismatch')
            row=(poll[0x1c],poll[0x1e],nmi[0x300+index],nmi[0x700+index],nmi[(0x780+index)&2047],
                 int(flags.group(1),16)&1,1,travel[0x1c],speed&255,speed>>8)
            event_text='CMJPOLLNMI1\n'+' '.join(map(str,row))+'\n'
        elif pc=='E1E7':
            kind='clock';counts[kind]+=1;entry=at
            end=next(i for i in range(at+1,len(pcs)) if pcs[i]=='E407')
            event_text='CMJCLOCKNMI1\n'+'\n'.join(clock_events(lines,pcs,ram,at,end))+'\n'
            phase=353
        else:
            kind='world';counts[kind]+=1;entry=at
            _,_,rows,boundary=read_nmi_trace(args.trace_directory,counts[kind])
            end=next(i for i in range(at+1,len(pcs)) if pcs[i] in ('E145','E4BE','E38A'))
            if pcs[end]!=boundary or boundary!='E145':
                raise ValueError(f'boundary returned.world[{counts[kind]}]: unsupported field-round continuation')
            event_text='CMJWORLDNMI1\n'+''.join(' '.join(map(str,row))+'\n' for row in rows)
            phase=9
        name=f'returned.{kind}[{counts[kind]}]'
        if index!=ram(entry)[0xe1]:
            raise ValueError(f'boundary {name}, random.index: native {index}, original {ram(entry)[0xe1]}')
        steps+=1;events=output/f'{steps:03d}-{kind}.txt';events.write_text(event_text)
        destination=output/f'{steps:03d}-{kind}.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-ordinary-return',kind,
                            str(events),str(index),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary {name}: {run.stderr.strip()}')
        actual=destination.read_bytes()
        legacy={352:10,353:13}.get(phase,phase)
        if phase>=256:
            if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=phase:
                raise ValueError(f'boundary {name}: invalid native session format or phase')
            actual=bytearray(actual[:259]+actual[260:]);actual[10]=133;actual[258]=legacy
        compare_checkpoint(actual,ram(end),legacy,legacy,'startup',name,None,False,False)
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(ram(end)[0xe1])]:
            raise ValueError(f'boundary {name}, random.index: native {indices}, original {ram(end)[0xe1]}')
        index=int(indices[0]);source=destination
        if kind=='world' and counts['world']==args.expected_rounds:
            break
    if counts!={'poll':args.expected_polls,'clock':args.expected_rounds,'world':args.expected_rounds}:
        raise ValueError(f'Incomplete returned-control chain: {counts}')
    print(f'Matched {steps} returned-control boundaries {counts}; final random index {index}; checkpoint {source}.')


if __name__=='__main__':
    try:
        main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Returned control replay stopped: {error}',file=sys.stderr)
        sys.exit(1)

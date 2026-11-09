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

from verify_captured_control_chain import clock_events, trace_data, menu_wait_events
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
    parser.add_argument('--expect-encounter',action='store_true',help='require the last field round to end at the original encounter')
    parser.add_argument('--menu-index',type=int,help='original first menu-ready random boundary')
    parser.add_argument('--menu-wait-end-index',type=int,help='original last menu-wait travel boundary')
    args=parser.parse_args()
    if not 0<=args.initial_index<=255 or args.expected_rounds<1 or args.expected_polls<1:
        raise ValueError('Random index and expected counts are invalid')
    if (args.menu_index is not None and not args.expect_encounter) or (args.menu_wait_end_index is not None and args.menu_index is None):
        raise ValueError('Menu comparison requires the encounter and any menu wait requires menu entry')
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
            expected_boundary='E4BE' if args.expect_encounter and counts[kind]==args.expected_rounds else 'E145'
            if pcs[end]!=boundary or boundary!=expected_boundary:
                raise ValueError(f'boundary returned.world[{counts[kind]}]: unsupported field-round continuation')
            event_text='CMJWORLDNMI1\n'+''.join(' '.join(map(str,row))+'\n' for row in rows)
            phase=354 if boundary=='E4BE' else 9
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
        legacy={352:10,353:13,354:15}.get(phase,phase)
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
    if args.menu_index is not None:
        menu=args.menu_index
        if not end<menu<len(pcs) or pcs[menu]!='C9C5':
            raise ValueError('boundary returned.menu: unsupported original ordering')
        rows,last=menu_wait_events(lines,pcs,ram,end,menu)
        points=[i for i in range(end+1,menu) if pcs[i] in ('C9C5','E3CA')]
        if len(points)!=len(rows):
            raise ValueError('boundary returned.menu_wait: ambiguous original event ordering')
        raw_state=source.read_bytes();values=[raw_state[305],raw_state[299],raw_state[304]];ordered=[]
        for row,point in zip(rows,points):
            observed=ram(point)
            for slot,(address,token) in enumerate(((0x532,'M'),(0x517,'F'),(0x8e,'D'))):
                if observed[address]!=values[slot]:
                    values[slot]=observed[address];ordered.append(f'{token} {values[slot]}')
            ordered.append(row)
        events=output/'menu-wait-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
        waiting=output/'menu-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',
                            str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary returned.menu_wait: {run.stderr.strip()}')
        actual=waiting.read_bytes()
        if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=354:
            raise ValueError('boundary returned.menu_wait: invalid native encounter snapshot')
        normalized=bytearray(actual[:259]+actual[260:]);normalized[10]=133;normalized[258]=15
        compare_checkpoint(normalized,ram(last),15,15,'startup','returned.menu_wait',None,False,False)
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(ram(last)[0xe1])] or ram(last)[0xe1]!=ram(menu)[0xe1]:
            raise ValueError('boundary returned.menu_wait, random.index: original and native differ')
        index=int(indices[0]);destination=output/'menu.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--open-action-menu','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary returned.menu: {run.stderr.strip()}')
        actual=destination.read_bytes()
        if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=355:
            raise ValueError('boundary returned.menu: invalid native menu snapshot')
        normalized=bytearray(actual[:259]+actual[260:]);normalized[10]=133;normalized[258]=18
        compare_checkpoint(normalized,ram(menu),18,18,'startup','returned.menu',None,False,False)
        source=destination;steps+=2
        if args.menu_wait_end_index is not None:
            finish=args.menu_wait_end_index
            if not menu<finish<len(pcs) or pcs[finish]!='E406':
                raise ValueError('boundary returned.menu_neutral: unsupported original wait boundary')
            rows,last=menu_wait_events(lines,pcs,ram,menu-1,finish+1)
            if last!=finish:
                raise ValueError('boundary returned.menu_neutral: incomplete travel event')
            events=output/'menu-neutral-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
            destination=output/'menu-neutral.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
            if run.returncode:
                raise RuntimeError(f'boundary returned.menu_neutral: {run.stderr.strip()}')
            actual=destination.read_bytes()
            if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=355:
                raise ValueError('boundary returned.menu_neutral: invalid native menu snapshot')
            normalized=bytearray(actual[:259]+actual[260:]);normalized[10]=133;normalized[258]=18
            compare_checkpoint(normalized,ram(finish),18,18,'startup','returned.menu_neutral',None,False,False)
            indices=re.findall(r'final random index (\d+);',run.stdout)
            if indices!=[str(ram(finish)[0xe1])]:
                raise ValueError('boundary returned.menu_neutral, random.index: original and native differ')
            index=int(indices[0]);source=destination;steps+=1
    print(f'Matched {steps} returned-control boundaries {counts}; final random index {index}; checkpoint {source}.')


if __name__=='__main__':
    try:
        main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Returned control replay stopped: {error}',file=sys.stderr)
        sys.exit(1)

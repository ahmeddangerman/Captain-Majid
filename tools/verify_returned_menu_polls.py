#!/usr/bin/env python3
"""Replay original button edges at a returned encounter's saved action menu."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from verify_captured_control_chain import menu_wait_events, trace_data
from verify_player7_pass_end_to_end import compare_checkpoint, prepare_output


def compare_menu(actual,raw,phase,boundary,score=0,strength=0,contact_raw=None):
    if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=phase:
        raise ValueError(f'boundary {boundary}: invalid native session format or phase')
    legacy={355:18,356:19,357:19,358:20,359:20,360:20,361:20,362:20}[phase]
    if phase in (357,358,359,360,361,362):
        scratch=raw if contact_raw is None else contact_raw
        wanted=bytes((1,0,1,0,*( (0,0,0,0,0) if phase in (357,358) else
                       (scratch[0x444],scratch[0x32],scratch[0x33],scratch[0x43f],scratch[0x440]) ),
                      strength if phase>=360 else 0,strength//4 if phase>=360 else 0,score if phase>=360 else 0,0,0))
        labels=('present','cursor','action','subtype','effect','power.low','power.high','cost.low','cost.high','strength','loss','score','result','weakened')
        for label,value,expected in zip(labels,actual[399:413],wanted):
            if value!=expected:
                raise ValueError(f'boundary {boundary}, contact.{label}: native {value}, expected {expected}')
        actual=bytearray(actual);actual[399:413]=bytes(14)
        if phase>=360:
            if actual[414]!=0:raise ValueError(f'boundary {boundary}, sequence.result: expected zero')
            actual[414]=255
    normalized=bytearray(actual[:259]+actual[260:]);normalized[10]=133;normalized[258]=legacy
    compare_checkpoint(normalized,raw,legacy,legacy,'startup',boundary,None,False,False)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--start-index',type=int,required=True)
    parser.add_argument('--initial-index',type=int,required=True)
    parser.add_argument('--expected-polls',type=int,required=True)
    parser.add_argument('--defense-wait-index',type=int,help='last original travel boundary before the defense choice')
    parser.add_argument('--defense-index',type=int,help='original attacker-preparation entry after recorded defense')
    parser.add_argument('--prepared-index',type=int,help='original attacker-prepared boundary')
    parser.add_argument('--defender-index',type=int,help='original shot defender-prepared boundary')
    parser.add_argument('--comparison-index',type=int,help='original shot defender comparison boundary')
    parser.add_argument('--comparison-followup-index',type=int,help='original comparison classification boundary')
    parser.add_argument('--shot-comparison-content',nargs=7,type=int,metavar=('RANGE','OFFSET','ELIGIBLE','T0','T1','T2','T3'))
    parser.add_argument('--contact-before-index',type=int)
    parser.add_argument('--contact-index',type=int)
    parser.add_argument('--contact-end-index',type=int)
    parser.add_argument('--dispatch-index',type=int)
    parser.add_argument('--dispatch-end-index',type=int)
    args=parser.parse_args()
    if args.start_index<0 or not 0<=args.initial_index<=255 or args.expected_polls<1:
        raise ValueError('Start, random index or expected poll count is invalid')
    defense=(args.defense_wait_index,args.defense_index,args.prepared_index)
    if any(value is not None for value in defense) and any(value is None for value in defense):
        raise ValueError('Shot defense comparison requires all three original boundaries')
    comparison=(args.defender_index,args.comparison_index,args.comparison_followup_index)
    if any(value is not None for value in comparison) and (any(value is None for value in comparison) or args.prepared_index is None or args.shot_comparison_content is None or any(not 0<=value<=255 for value in args.shot_comparison_content) or args.shot_comparison_content[2]>1):
        raise ValueError('Shot defender comparison requires all boundaries and direct table content')
    clocks=(args.contact_before_index,args.contact_index,args.contact_end_index,args.dispatch_index,args.dispatch_end_index)
    if any(value is not None for value in clocks) and (any(value is None for value in clocks) or args.comparison_index is None):
        raise ValueError('Shot contact and dispatch need all five boundaries and the comparison')
    lines,pcs,ram=trace_data(args.trace_directory)
    if args.start_index>=len(pcs) or pcs[args.start_index]!='C9C5':
        raise ValueError('boundary menu.start: expected original random entry')
    source=args.source.resolve();index=args.initial_index
    compare_menu(source.read_bytes(),ram(args.start_index),355,'returned.menu.start')
    if index!=ram(args.start_index)[0xe1]:
        raise ValueError(f'boundary returned.menu.start, random.index: native {index}, original {ram(args.start_index)[0xe1]}')
    output=prepare_output(args.output,Path(__file__).resolve().parents[1]);operation=0;count=0

    def wait(source,index,start,stop,label):
        nonlocal operation
        if not any(pcs[j]=='C9C5' for j in range(start+1,stop)):
            return source,index
        rows,last=menu_wait_events(lines,pcs,ram,start,stop);operation+=1
        events=output/f'{operation:03d}-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        destination=output/f'{operation:03d}-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',
                            str(events),str(index),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary {label}: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(ram(last)[0xe1])]:
            raise ValueError(f'boundary {label}, random.index: native {indices}, original {ram(last)[0xe1]}')
        return destination,int(indices[0])

    previous=args.start_index-1;confirmed=False
    for at in range(args.start_index,len(pcs)):
        if pcs[at]!='D5EA' or at<=previous:
            continue
        name=f'returned.menu.poll[{count+1}]'
        source,index=wait(source,index,previous,at,name+'.wait')
        compare_menu(source.read_bytes(),ram(at),355,name+'.input')
        if index!=ram(at)[0xe1]:
            raise ValueError(f'boundary {name}.input, random.index: native {index}, original {ram(at)[0xe1]}')
        operation+=1;destination=output/f'{operation:03d}-poll.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--menu-input',str(ram(at)[0x1e]),
                            '--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary {name}: {run.stderr.strip()}')
        source=destination
        end=next(j for j in range(at+1,len(pcs)) if pcs[j] in ('D5E5','D67C','D5EA'))
        if pcs[end]=='D5EA':
            raise ValueError(f'boundary {name}: missing original post-poll boundary')
        source,index=wait(source,index,at,end,name+'.result_wait')
        phase=356 if pcs[end]=='D67C' else 355
        compare_menu(source.read_bytes(),ram(end),phase,name+'.result')
        if index!=ram(end)[0xe1]:
            raise ValueError(f'boundary {name}.result, random.index: native {index}, original {ram(end)[0xe1]}')
        count+=1;previous=end
        if phase==356:
            confirmed=True;break
    if not confirmed or count!=args.expected_polls:
        raise ValueError(f'Expected {args.expected_polls} polls through shot confirmation; got {count}, confirmed={confirmed}')
    if args.defense_index is not None:
        before,choice,prepared=defense
        if not previous<before<choice<prepared<len(pcs) or [pcs[i] for i in defense]!=['E406','804C','804F'] or ram(choice)[0x24]!=26 or ram(prepared)[0x24]!=26:
            raise ValueError('boundary returned.shot: unsupported original ordering or bank')
        if any(pcs[i]=='C9C5' for i in range(before+1,prepared)):
            raise ValueError('boundary returned.shot: unexpected random event after the defense wait')
        source,index=wait(source,index,previous,choice,'returned.shot.wait')
        compare_menu(source.read_bytes(),ram(before),356,'returned.shot.wait')
        observed=ram(choice)
        if observed[0x600]!=1 or observed[0x442]!=observed[0x601]:
            raise ValueError('boundary returned.shot.defense: unsupported original defender prefix')
        operation+=1;destination=output/f'{operation:03d}-defense.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--record-ordinary-return-shot-defense',
                            str(observed[0x442]),str(observed[0x60b]),str(observed[0x606]),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary returned.shot.defense: {run.stderr.strip()}')
        compare_menu(destination.read_bytes(),observed,357,'returned.shot.defense')
        source=destination;operation+=1;destination=output/f'{operation:03d}-prepared.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-ordinary-return-shot','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:
            raise RuntimeError(f'boundary returned.shot.prepared: {run.stderr.strip()}')
        compare_menu(destination.read_bytes(),ram(prepared),358,'returned.shot.prepared')
        if index!=ram(prepared)[0xe1]:
            raise ValueError(f'boundary returned.shot.prepared, random.index: native {index}, original {ram(prepared)[0xe1]}')
        source=destination
    if args.defender_index is not None:
        defender,comparison,followup=args.defender_index,args.comparison_index,args.comparison_followup_index
        if not args.prepared_index<defender<comparison<followup<len(pcs) or [pcs[i] for i in (defender,comparison,followup)]!=['80B6','80D6','80D9'] or any(ram(i)[0x24]!=26 for i in (defender,comparison,followup)):
            raise ValueError('boundary returned.shot.defender: unsupported original ordering or bank')
        last=next(i for i in range(defender-1,args.prepared_index,-1) if pcs[i]=='E406')
        rows,checked=menu_wait_events(lines,pcs,ram,args.prepared_index,last+1)
        points=[i for i in range(args.prepared_index+1,last+1) if pcs[i] in ('C9C5','E3CA')]
        if checked!=last or len(points)!=len(rows):
            raise ValueError('boundary returned.shot.defender_wait: ambiguous event ordering')
        state=source.read_bytes();values=[state[305],state[301],state[413]];ordered=[]
        for row,point in zip(rows,points):
            original=ram(point)
            for slot,(address,token) in enumerate(((0x532,'M'),(0x617,'L'),(0x616,'C'))):
                if original[address]!=values[slot]:values[slot]=original[address];ordered.append(f'{token} {values[slot]}')
            ordered.append(row)
        operation+=1;events=output/f'{operation:03d}-shot-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
        waiting=output/f'{operation:03d}-shot-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.defender_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(ram(last)[0xe1])]:raise ValueError('boundary returned.shot.defender_wait, random.index: original and native differ')
        index=int(indices[0]);compare_menu(waiting.read_bytes(),ram(last),358,'returned.shot.defender_wait')
        operation+=1;destination=output/f'{operation:03d}-shot-defender.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--prepare-ordinary-return-shot-defender',str(ram(defender)[0x44e]),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.defender: {run.stderr.strip()}')
        compare_menu(destination.read_bytes(),ram(defender),359,'returned.shot.defender')
        if any(pcs[i]=='C9C5' for i in range(defender+1,followup)):
            raise ValueError('boundary returned.shot.compare: unexpected interleaved random event')
        def register(line,name):
            value=re.search(r'\b'+name+r'=([0-9A-Fa-f]{2})\b',line)
            if value is None:raise ValueError(f'boundary returned.shot.compare: missing register {name}')
            return int(value.group(1),16)
        score=register(lines[comparison],'A');result=register(lines[comparison],'Y');strength=ram(comparison)[0x71]
        if result!=0 or ram(comparison)[0x619]!=strength//4:
            raise ValueError('boundary returned.shot.compare: unsupported original result or loss')
        operation+=1;compared=output/f'{operation:03d}-shot-compared.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--compare-ordinary-return-shot-defender',*map(str,args.shot_comparison_content),'--capture',str(compared)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.compare: {run.stderr.strip()}')
        compare_menu(compared.read_bytes(),ram(comparison),360,'returned.shot.compare',score,strength)
        if index!=ram(comparison)[0xe1]:raise ValueError('boundary returned.shot.compare, random.index: original and native differ')
        source=compared
    if args.contact_index is not None:
        before,contact,end,dispatch,final=clocks
        if not args.comparison_followup_index<before<contact<end<dispatch<final<len(pcs) or [pcs[i] for i in clocks]!=['E406','C9C5','80DC','C9C5','80ED'] or ram(end)[0x24]!=26 or ram(final)[0x24]!=26:
            raise ValueError('boundary returned.shot.contact: unsupported original ordering or bank')
        source,index=wait(source,index,args.comparison_followup_index,before+1,'returned.shot.contact_wait')
        compare_menu(source.read_bytes(),ram(before),360,'returned.shot.contact_wait',score,strength,ram(args.comparison_index))
        operation+=1;destination=output/f'{operation:03d}-shot-contact.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-ordinary-return-shot-contact','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.contact: {run.stderr.strip()}')
        compare_menu(destination.read_bytes(),ram(contact),361,'returned.shot.contact',score,strength,ram(args.comparison_index))
        source,index=wait(destination,index,contact-1,end,'returned.shot.contact_after')
        compare_menu(source.read_bytes(),ram(end),361,'returned.shot.contact_after',score,strength,ram(args.comparison_index))
        operation+=1;destination=output/f'{operation:03d}-shot-dispatch.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-ordinary-return-shot-dispatch','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.dispatch: {run.stderr.strip()}')
        compare_menu(destination.read_bytes(),ram(dispatch),362,'returned.shot.dispatch',score,strength,ram(args.comparison_index))
        source,index=wait(destination,index,dispatch-1,final,'returned.shot.dispatch_after')
        compare_menu(source.read_bytes(),ram(final),362,'returned.shot.dispatch_after',score,strength,ram(args.comparison_index))
        if index!=ram(final)[0xe1]:raise ValueError('boundary returned.shot.dispatch, random.index: original and native differ')
    print(f'Matched {count} returned menu polls through {"shot contact and dispatch" if args.contact_index is not None else "shot defender comparison" if args.defender_index is not None else "recorded defense and shot preparation" if args.defense_index is not None else "ordinary shot confirmation"}; final random index {index}; checkpoint {source}.')


if __name__=='__main__':
    try:
        main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Returned menu replay stopped: {error}',file=sys.stderr)
        sys.exit(1)

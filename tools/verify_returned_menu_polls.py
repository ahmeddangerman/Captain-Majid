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


def compare_shot_flight(actual,raw,phase,boundary):
    if len(actual)!=419 or actual[10]!=143 or int.from_bytes(actual[258:260],'little')!=phase:
        raise ValueError(f'boundary {boundary}: invalid native session format or phase')
    labels=('present','distance','destination','counter','loose_ball','trajectory_present',
            'x.low','x.high','y.low','y.high','vx.low','vx.high','vy.low','vy.high','direction','flags')
    expected=bytes((1,raw[0x62b],raw[0x638],raw[0x61a],0,1))+raw[0x634:0x638]+raw[0x639:0x63d]+bytes((raw[0x62c],raw[0x61b]))
    for name,value,wanted in zip(labels,actual[340:356],expected):
        if value!=wanted:raise ValueError(f'boundary {boundary}, flight.{name}: native {value}, original {wanted}')
    for name,value,wanted in (('defenders.cursor',actual[413],raw[0x616]),('sequence.result',actual[414],raw[0x612])):
        if value!=wanted:raise ValueError(f'boundary {boundary}, {name}: native {value}, original {wanted}')
    normalized=bytearray(actual[:259]+actual[260:]);normalized[10]=133;normalized[258]=20
    if phase in (388,389,390):
        for name,actual_value,expected_value in (('field.selected_defenders',normalized[28],raw[0x600]),('scheduling.selected_ids[0]',normalized[309],raw[0x601])):
            if actual_value!=expected_value:raise ValueError(f'boundary {boundary}, {name}: native {actual_value}, original {expected_value}')
        # The generic exporter accepts field defenders only. Compare the keeper
        # prefix explicitly, then normalize it solely for the remaining fields.
        normalized[28]=0;normalized[309]=0;raw=bytearray(raw);raw[0x600]=0
    compare_checkpoint(normalized,raw,20,20,'startup',boundary,None,False,False)


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
    parser.add_argument('--movement-end-index',type=int,help='original shot handoff after ordered player movement')
    parser.add_argument('--target-trace-directory',type=Path,help='private follow-up trace matching the movement handoff')
    parser.add_argument('--target-entry-index',type=int,help='follow-up original C618 handoff index')
    parser.add_argument('--target-index',type=int,help='follow-up original DD08 target index')
    parser.add_argument('--flight-setup-index',type=int,help='follow-up original E773 numeric setup index')
    parser.add_argument('--velocity',type=Path,help='private original-derived ball velocity content')
    parser.add_argument('--flight-cell-indices',nargs='+',type=int,help='ordered original E773 boundaries after cell callbacks')
    parser.add_argument('--flight-step-indices',nargs='+',type=int,help='matching original E7B4 boundaries before callbacks')
    parser.add_argument('--flight-arrival-index',type=int,help='original boundary after numeric arrival')
    parser.add_argument('--flight-defender-pending-index',type=int)
    parser.add_argument('--flight-defender-index',type=int)
    parser.add_argument('--flight-defender-result-index',type=int)
    parser.add_argument('--recovery-trace-directory',type=Path)
    parser.add_argument('--recovery-entry-index',type=int)
    parser.add_argument('--recovery-handler-index',type=int)
    parser.add_argument('--recovery-position-index',type=int)
    parser.add_argument('--recovery-clock-index',type=int)
    parser.add_argument('--recovery-before-index',type=int)
    parser.add_argument('--recovery-handoff-index',type=int)
    parser.add_argument('--owner-trace-directory',type=Path)
    parser.add_argument('--owner-entry-index',type=int)
    parser.add_argument('--owner-jitter-index',type=int)
    parser.add_argument('--owner-movement-index',type=int)
    parser.add_argument('--owner-positioned-index',type=int)
    parser.add_argument('--area-trace-directory',type=Path)
    parser.add_argument('--area-entry-index',type=int)
    parser.add_argument('--area-selection-index',type=int)
    parser.add_argument('--area-defense-index',type=int)
    parser.add_argument('--area-menu-index',type=int)
    parser.add_argument('--keeper-cells',type=Path)
    parser.add_argument('--area-poll-trace-directory',type=Path)
    parser.add_argument('--area-poll-entry-index',type=int)
    parser.add_argument('--expected-area-polls',type=int)
    parser.add_argument('--volley-trace-directory',type=Path)
    parser.add_argument('--volley-entry-index',type=int)
    parser.add_argument('--volley-prepare-index',type=int)
    parser.add_argument('--volley-prepared-index',type=int)
    parser.add_argument('--volley-wait-index',type=int)
    parser.add_argument('--volley-clock-entry-index',type=int)
    parser.add_argument('--volley-clock-index',type=int)
    parser.add_argument('--volley-clock-end-index',type=int)
    parser.add_argument('--volley-movement-index',type=int)
    parser.add_argument('--volley-dispatch-clock-before-index',type=int)
    parser.add_argument('--volley-dispatch-clock-index',type=int)
    parser.add_argument('--volley-dispatch-before-index',type=int)
    parser.add_argument('--volley-dispatch-index',type=int)
    parser.add_argument('--volley-handoff-index',type=int)
    parser.add_argument('--volley-target-index',type=int)
    parser.add_argument('--volley-keeper-arrival-index',type=int)
    parser.add_argument('--volley-keeper-ready-index',type=int)
    parser.add_argument('--volley-keeper-wait-index',type=int)
    parser.add_argument('--volley-keeper-precompare-index',type=int)
    parser.add_argument('--volley-keeper-result-ram',type=Path,help='private original $8599 RAM, independently continued from the comparison')
    parser.add_argument('--volley-keeper-presentation-index',type=int,help='subsequent original presentation entry')
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
    if args.flight_step_indices is not None and args.flight_cell_indices is None:
        raise ValueError('Shot flight step boundaries require matching callback boundaries')
    if args.flight_arrival_index is not None and args.flight_cell_indices is None:
        raise ValueError('Shot flight arrival requires preceding callback boundaries')
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
    if args.movement_end_index is not None:
        end=args.movement_end_index
        if args.dispatch_end_index is None or not args.dispatch_end_index<end<len(pcs) or pcs[end]!='C618':
            raise ValueError('boundary returned.shot.movement: unsupported original handoff order')
        rows,last=menu_wait_events(lines,pcs,ram,args.dispatch_end_index,end)
        if len(rows)!=44:
            raise ValueError(f'boundary returned.shot.movement: expected 22 random/travel pairs, got {len(rows)//2}')
        operation+=1;events=output/f'{operation:03d}-movement-events.txt'
        events.write_text('CMJNMISEQ1\n'+''.join(rows[2*slot]+'\n'+rows[2*slot+1]+'\nP '+str(slot)+'\n' for slot in range(22)))
        destination=output/f'{operation:03d}-movement.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-field-movement',str(events),str(index),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.movement: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(ram(end)[0xe1])]:raise ValueError(f'boundary returned.shot.movement, random.index: native {indices}, original {ram(end)[0xe1]}')
        adjusted=bytearray(ram(end));adjusted[0x32:0x34]=ram(args.comparison_index)[0x32:0x34]
        normalized=bytearray(destination.read_bytes());normalized[258:260]=(362).to_bytes(2,'little')
        compare_menu(normalized,adjusted,362,'returned.shot.movement',score,strength,ram(args.comparison_index))
        source=destination;index=int(indices[0])
    target_args=(args.target_trace_directory,args.target_entry_index,args.target_index)
    if any(value is not None for value in target_args):
        if any(value is None for value in target_args) or args.movement_end_index is None:
            raise ValueError('boundary returned.shot.target: target comparison requires movement and all follow-up trace inputs')
        target_lines,target_pcs,target_ram=trace_data(args.target_trace_directory)
        entry,target=args.target_entry_index,args.target_index
        if not 0<=entry<target<len(target_pcs) or target_pcs[entry]!='C618' or target_pcs[target]!='DD08' or any(target_pcs[i]=='C9C5' for i in range(entry+1,target)):
            raise ValueError('boundary returned.shot.target: unsupported original ordering')
        handoff=ram(args.movement_end_index);followup=target_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,handoff)):
            if value!=wanted:
                raise ValueError(f'boundary returned.shot.target_handoff, RAM[0x{address:03x}]: follow-up {value}, movement {wanted}')
        operation+=1;destination=output/f'{operation:03d}-target.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-shot-approach','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.target: {run.stderr.strip()}')
        actual=bytearray(destination.read_bytes());observed=target_ram(target)
        if len(actual)!=419:raise ValueError('boundary returned.shot.target: invalid native session size')
        if int.from_bytes(actual[258:260],'little')!=364:
            raise ValueError('boundary returned.shot.target, session.phase: expected 364')
        flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',target_lines[target])
        if flags is None:raise ValueError('boundary returned.shot.target: missing original carry')
        expected=(1,observed[0x62b],observed[0x638],observed[0x61a],int(flags.group(1),16)&1)
        for label,value,wanted in zip(('present','distance','destination','counter','loose_ball'),actual[340:345],expected):
            if value!=wanted:raise ValueError(f'boundary returned.shot.target, flight.{label}: native {value}, original {wanted}')
        adjusted=bytearray(observed);adjusted[0x32:0x34]=ram(args.comparison_index)[0x32:0x34]
        actual[258:260]=(362).to_bytes(2,'little')
        compare_menu(actual,adjusted,362,'returned.shot.target',score,strength,ram(args.comparison_index))
        if index!=observed[0xe1]:raise ValueError(f'boundary returned.shot.target, random.index: native {index}, original {observed[0xe1]}')
        source=destination
    if args.flight_setup_index is not None or args.velocity is not None:
        if args.flight_setup_index is None or args.velocity is None or args.target_index is None:
            raise ValueError('boundary returned.shot.flight_setup: target, setup and velocity are required')
        setup=args.flight_setup_index
        if not args.target_index<setup<len(target_pcs) or target_pcs[setup]!='E773' or any(target_pcs[i]=='C9C5' for i in range(args.target_index+1,setup)):
            raise ValueError('boundary returned.shot.flight_setup: unsupported original ordering')
        fractions=(target_ram(args.target_index)[0x634],target_ram(args.target_index)[0x636])
        operation+=1;destination=output/f'{operation:03d}-flight-setup.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--begin-ball-flight',str(args.velocity.resolve()),*map(str,fractions),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.flight_setup: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),target_ram(setup),365,'returned.shot.flight_setup')
        if index!=target_ram(setup)[0xe1]:raise ValueError(f'boundary returned.shot.flight_setup, random.index: native {index}, original {target_ram(setup)[0xe1]}')
        source=destination
    if args.flight_cell_indices is not None:
        if args.flight_setup_index is None:raise ValueError('boundary returned.shot.flight: setup is required')
        if args.flight_step_indices is not None and len(args.flight_step_indices)!=len(args.flight_cell_indices):
            raise ValueError('boundary returned.shot.flight: step and callback counts differ')
        previous=args.flight_setup_index
        for slot,cell in enumerate(args.flight_cell_indices,1):
            step=args.flight_step_indices[slot-1] if args.flight_step_indices is not None else cell
            label=f'returned.shot.flight.cell[{slot}]'
            if not previous<cell<len(target_pcs) or target_pcs[cell]!='E773':
                raise ValueError(f'boundary {label}: unsupported original ordering')
            if step!=cell and (not previous<step<cell or target_pcs[step]!='E7B4' or any(target_pcs[i]=='C9C5' for i in range(step+1,cell))):
                raise ValueError(f'boundary {label}.step: unsupported original order')
            rows,last=menu_wait_events(target_lines,target_pcs,target_ram,previous,step)
            if rows:
                points=[i for i in range(previous+1,step) if target_pcs[i] in ('C9C5','E3CA')]
                if len(points)!=len(rows):raise ValueError(f'boundary {label}.wait: ambiguous motion sample ordering')
                motion=source.read_bytes()[305];ordered=[]
                for row,point in zip(rows,points):
                    observed=target_ram(point)[0x532]
                    if observed!=motion:motion=observed;ordered.append(f'M {motion}')
                    ordered.append(row)
                operation+=1;events=output/f'{operation:03d}-flight-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
                waiting=output/f'{operation:03d}-flight-wait.cmjsession'
                run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'boundary {label}.wait: {run.stderr.strip()}')
                indices=re.findall(r'final random index (\d+);',run.stdout)
                if indices!=[str(target_ram(last)[0xe1])]:raise ValueError(f'boundary {label}.wait, random.index: native {indices}, original {target_ram(last)[0xe1]}')
                source=waiting;index=int(indices[0])
            operation+=1;stepped=output/f'{operation:03d}-flight-step.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-ball-cell','--capture',str(stepped)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary {label}.step: {run.stderr.strip()}')
            if step!=cell:compare_shot_flight(stepped.read_bytes(),target_ram(step),366,label+'.step')
            operation+=1;destination=output/f'{operation:03d}-flight-callback.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(stepped),'--resolve-ball-cell','--capture',str(destination)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary {label}.callback: {run.stderr.strip()}')
            compare_shot_flight(destination.read_bytes(),target_ram(cell),365,label)
            if index!=target_ram(cell)[0xe1]:raise ValueError(f'boundary {label}, random.index: native {index}, original {target_ram(cell)[0xe1]}')
            source=destination;previous=cell
    if args.flight_arrival_index is not None:
        arrival=args.flight_arrival_index
        if args.flight_cell_indices is None or not previous<arrival<len(target_pcs) or target_pcs[arrival]!='E7CC':
            raise ValueError('boundary returned.shot.flight.arrival: unsupported original ordering')
        rows,last=menu_wait_events(target_lines,target_pcs,target_ram,previous,arrival)
        points=[i for i in range(previous+1,arrival) if target_pcs[i] in ('C9C5','E3CA')]
        if len(points)!=len(rows):raise ValueError('boundary returned.shot.flight.arrival: ambiguous motion ordering')
        motion=source.read_bytes()[305];ordered=[]
        for row,point in zip(rows,points):
            observed=target_ram(point)[0x532]
            if observed!=motion:motion=observed;ordered.append(f'M {motion}')
            ordered.append(row)
        if ordered:
            operation+=1;events=output/f'{operation:03d}-arrival-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
            waiting=output/f'{operation:03d}-arrival-wait.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.arrival.wait: {run.stderr.strip()}')
            indices=re.findall(r'final random index (\d+);',run.stdout)
            if indices!=[str(target_ram(last)[0xe1])]:raise ValueError('boundary returned.shot.flight.arrival.wait, random.index: original and native differ')
            source=waiting;index=int(indices[0])
        operation+=1;destination=output/f'{operation:03d}-arrival.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-ball-cell','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.arrival: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),target_ram(arrival),367,'returned.shot.flight.arrival')
        if index!=target_ram(arrival)[0xe1]:raise ValueError('boundary returned.shot.flight.arrival, random.index: original and native differ')
        source=destination
    defender_indices=(args.flight_defender_pending_index,args.flight_defender_index,args.flight_defender_result_index)
    if any(value is not None for value in defender_indices):
        if any(value is None for value in defender_indices) or args.flight_arrival_index is None:
            raise ValueError('boundary returned.shot.flight.defender: arrival and all defender boundaries are required')
        pending,defender,result=defender_indices
        if not args.flight_arrival_index<pending<defender<result<len(target_pcs) or [target_pcs[i] for i in defender_indices]!=['80DC','888D','88A8'] or target_ram(defender)[0x24]!=26 or target_ram(result)[0x24]!=26:
            raise ValueError('boundary returned.shot.flight.defender: unsupported original order or bank')
        if any(target_pcs[i]=='C9C5' for i in range(args.flight_arrival_index+1,pending)) or any(target_pcs[i]=='C9C5' for i in range(defender+1,result)):
            raise ValueError('boundary returned.shot.flight.defender: unexpected random event')
        operation+=1;destination=output/f'{operation:03d}-flight-defenders-pending.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--continue-flight-arrival','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.defenders: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),target_ram(pending),368,'returned.shot.flight.defenders')
        source=destination
        rows,last=menu_wait_events(target_lines,target_pcs,target_ram,pending,defender)
        if rows:
            operation+=1;events=output/f'{operation:03d}-defender-wait-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
            waiting=output/f'{operation:03d}-defender-wait.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.defender_wait: {run.stderr.strip()}')
            indices=re.findall(r'final random index (\d+);',run.stdout)
            if indices!=[str(target_ram(last)[0xe1])]:raise ValueError('boundary returned.shot.flight.defender_wait, random.index: original and native differ')
            compare_shot_flight(waiting.read_bytes(),target_ram(last),368,'returned.shot.flight.defender_wait')
            source=waiting;index=int(indices[0])
        operation+=1;prepared=output/f'{operation:03d}-flight-defender-prepared.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-ordinary-return-flight-defender','--capture',str(prepared)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.defender_prepare: {run.stderr.strip()}')
        observed=target_ram(defender)
        def check_contact(actual,raw,phase,label):
            scratch=target_ram(defender)
            wanted=bytes((1,0,0,0,scratch[0x444],scratch[0x32],scratch[0x33],scratch[0x43f],scratch[0x440],
                raw[0x71] if phase==370 else 0,raw[0x619] if phase==370 else 0,raw[0x6f] if phase==370 else 0,raw[0x612] if phase==370 else 0,0))
            labels=('present','cursor','action','subtype','effect','power.low','power.high','cost.low','cost.high','strength','loss','score','result','weakened')
            for name,value,expected in zip(labels,actual[399:413],wanted):
                if value!=expected:raise ValueError(f'boundary {label}, contact.{name}: native {value}, original {expected}')
            compare_shot_flight(actual,raw,phase,label)
        check_contact(prepared.read_bytes(),observed,369,'returned.shot.flight.defender_prepare')
        operation+=1;compared=output/f'{operation:03d}-flight-defender-compared.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(prepared),'--compare-ordinary-return-flight-defender','--capture',str(compared)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.flight.defender_compare: {run.stderr.strip()}')
        check_contact(compared.read_bytes(),target_ram(result),370,'returned.shot.flight.defender_compare')
        if index!=target_ram(result)[0xe1]:raise ValueError('boundary returned.shot.flight.defender_compare, random.index: original and native differ')
        source=compared
    recovery_args=(args.recovery_trace_directory,args.recovery_entry_index,args.recovery_handler_index)
    if any(value is not None for value in recovery_args):
        if any(value is None for value in recovery_args) or args.flight_defender_result_index is None:
            raise ValueError('boundary returned.shot.recovery_wait: defender comparison and all follow-up trace inputs are required')
        recovery_lines,recovery_pcs,recovery_ram=trace_data(args.recovery_trace_directory)
        entry,handler=args.recovery_entry_index,args.recovery_handler_index
        if not 0<=entry<handler<len(recovery_pcs) or [recovery_pcs[entry],recovery_pcs[handler]]!=['88A8','88BB'] or recovery_ram(handler)[0x24]!=26:
            raise ValueError('boundary returned.shot.recovery_wait: unsupported original order or bank')
        original=target_ram(args.flight_defender_result_index);followup=recovery_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,original)):
            if value!=wanted:raise ValueError(f'boundary returned.shot.recovery_handoff, RAM[0x{address:03x}]: follow-up {value}, defender {wanted}')
        rows,last=menu_wait_events(recovery_lines,recovery_pcs,recovery_ram,entry,handler)
        points=[i for i in range(entry+1,handler) if recovery_pcs[i] in ('C9C5','E3CA')]
        if len(points)!=len(rows):raise ValueError('boundary returned.shot.recovery_wait: ambiguous facing ordering')
        facing=source.read_bytes()[299];ordered=[]
        for row,point in zip(rows,points):
            observed=recovery_ram(point)[0x517]
            if observed!=facing:facing=observed;ordered.append(f'F {facing}')
            ordered.append(row)
        operation+=1;events=output/f'{operation:03d}-recovery-wait-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
        waiting=output/f'{operation:03d}-recovery-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(recovery_ram(handler)[0xe1])]:raise ValueError('boundary returned.shot.recovery_wait, random.index: original and native differ')
        check_contact(waiting.read_bytes(),recovery_ram(handler),370,'returned.shot.recovery_wait')
        source=waiting;index=int(indices[0])
    recovery_boundaries=(args.recovery_position_index,args.recovery_clock_index,args.recovery_before_index,args.recovery_handoff_index)
    if any(value is not None for value in recovery_boundaries):
        if any(value is None for value in recovery_boundaries) or args.recovery_handler_index is None:
            raise ValueError('boundary returned.shot.recovery: all recovery boundaries and handler are required')
        position,clock,before,handoff=recovery_boundaries
        if not args.recovery_handler_index<position<clock<before<handoff<len(recovery_pcs) or [recovery_pcs[i] for i in recovery_boundaries]!=['81BC','C9C5','E406','8BDF'] or recovery_ram(position)[0x24]!=26 or recovery_ram(handoff)[0x24]!=26:
            raise ValueError('boundary returned.shot.recovery: unsupported original order or bank')
        if any(recovery_pcs[i]=='C9C5' for i in range(args.recovery_handler_index+1,clock)) or any(recovery_pcs[i]=='C9C5' for i in range(before+1,handoff)):
            raise ValueError('boundary returned.shot.recovery: unexpected interleaved random event')
        operation+=1;positioned=output/f'{operation:03d}-blocked-position.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--position-ordinary-return-blocked-shot','--capture',str(positioned)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.position: {run.stderr.strip()}')
        compare_shot_flight(positioned.read_bytes(),recovery_ram(position),371,'returned.shot.recovery.position')
        operation+=1;clocked=output/f'{operation:03d}-blocked-clock.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(positioned),'--advance-ordinary-return-blocked-shot-clock','--capture',str(clocked)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.clock: {run.stderr.strip()}')
        compare_shot_flight(clocked.read_bytes(),recovery_ram(clock),372,'returned.shot.recovery.clock')
        rows,last=menu_wait_events(recovery_lines,recovery_pcs,recovery_ram,clock-1,before+1)
        if last!=before:raise ValueError('boundary returned.shot.recovery.clock_wait: incomplete event sequence')
        operation+=1;events=output/f'{operation:03d}-blocked-clock-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        waiting=output/f'{operation:03d}-blocked-clock-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(clocked),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.clock_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(recovery_ram(before)[0xe1])]:raise ValueError('boundary returned.shot.recovery.clock_wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),recovery_ram(before),372,'returned.shot.recovery.clock_wait')
        operation+=1;destination=output/f'{operation:03d}-blocked-recovery.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--enter-area-pass-blocked-recovery','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.handoff: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),recovery_ram(handoff),373,'returned.shot.recovery.handoff')
        if index!=recovery_ram(handoff)[0xe1]:raise ValueError('boundary returned.shot.recovery.handoff, random.index: original and native differ')
        source=destination
    owner_args=(args.owner_trace_directory,args.owner_entry_index,args.owner_jitter_index,args.owner_movement_index,args.owner_positioned_index)
    if any(value is not None for value in owner_args):
        if any(value is None for value in owner_args) or args.recovery_handoff_index is None:
            raise ValueError('boundary returned.shot.recovery.owner: recovery handoff and all owner boundaries are required')
        owner_lines,owner_pcs,owner_ram=trace_data(args.owner_trace_directory)
        entry,jitter,movement,positioned=owner_args[1:]
        if not 0<=entry<jitter<movement<positioned<len(owner_pcs) or [owner_pcs[i] for i in (entry,jitter,movement,positioned)]!=['8BDF','8BE5','8C10','8C31'] or any(owner_ram(i)[0x24]!=26 for i in (entry,jitter,movement,positioned)):
            raise ValueError('boundary returned.shot.recovery.owner: unsupported original ordering or bank')
        original=recovery_ram(args.recovery_handoff_index);followup=owner_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,original)):
            if value!=wanted:raise ValueError(f'boundary returned.shot.recovery.owner_handoff, RAM[0x{address:03x}]: follow-up {value}, recovery {wanted}')
        if any(owner_pcs[i]=='C9C5' for i in range(entry+1,jitter)) or any(owner_pcs[i]=='C9C5' for i in range(movement+1,positioned)):
            raise ValueError('boundary returned.shot.recovery.owner: unexpected random event')
        operation+=1;destination=output/f'{operation:03d}-blocked-jitter.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--jitter-ordinary-return-blocked-recovery','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.jitter: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),owner_ram(jitter),374,'returned.shot.recovery.jitter')
        rows,last=menu_wait_events(owner_lines,owner_pcs,owner_ram,jitter,movement)
        if len(rows)!=44:raise ValueError(f'boundary returned.shot.recovery.movement: expected 22 random/travel pairs, got {len(rows)//2}')
        operation+=1;events=output/f'{operation:03d}-recovery-movement-events.txt'
        events.write_text('CMJNMISEQ1\n'+''.join(rows[2*slot]+'\n'+rows[2*slot+1]+'\nP '+str(slot)+'\n' for slot in range(22)))
        moved=output/f'{operation:03d}-recovery-field-moved.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--advance-observed-field-movement',str(events),str(index),'--capture',str(moved)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.movement: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(owner_ram(movement)[0xe1])]:raise ValueError('boundary returned.shot.recovery.movement, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(moved.read_bytes(),owner_ram(movement),375,'returned.shot.recovery.movement')
        operation+=1;destination=output/f'{operation:03d}-recovered-owner.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(moved),'--choose-ordinary-return-blocked-owner','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.shot.recovery.owner: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),owner_ram(positioned),376,'returned.shot.recovery.owner')
        if index!=owner_ram(positioned)[0xe1]:raise ValueError('boundary returned.shot.recovery.owner, random.index: original and native differ')
        source=destination
    area_args=(args.area_trace_directory,args.area_entry_index,args.area_selection_index,args.area_defense_index,args.area_menu_index,args.keeper_cells)
    if any(value is not None for value in area_args):
        if any(value is None for value in area_args) or args.owner_positioned_index is None:
            raise ValueError('boundary returned.goal_area: owner and all area trace inputs are required')
        area_lines,area_pcs,area_ram=trace_data(args.area_trace_directory)
        entry,selection,defense,menu=area_args[1:5]
        if not 0<=entry<selection<defense<menu<len(area_pcs) or [area_pcs[i] for i in (entry,selection,defense,menu)]!=['8C31','8B6D','8B86','D5E5'] or any(area_ram(i)[0x24]!=26 for i in (entry,selection,defense,menu)):
            raise ValueError('boundary returned.goal_area: unsupported original ordering or bank')
        original=owner_ram(args.owner_positioned_index);followup=area_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,original)):
            if value!=wanted:raise ValueError(f'boundary returned.goal_area.handoff, RAM[0x{address:03x}]: follow-up {value}, owner {wanted}')
        if any(area_pcs[i]=='C9C5' for i in range(entry+1,defense)):
            raise ValueError('boundary returned.goal_area: unexpected interleaved random event')
        operation+=1;selected=output/f'{operation:03d}-goal-area-selection.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-ordinary-return-goal-area','--capture',str(selected)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.selection: {run.stderr.strip()}')
        compare_shot_flight(selected.read_bytes(),area_ram(selection),377,'returned.goal_area.selection')
        operation+=1;defended=output/f'{operation:03d}-goal-area-defenders.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--resolve-ordinary-return-goal-area',str(args.keeper_cells.resolve()),'--capture',str(defended)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.defenders: {run.stderr.strip()}')
        compare_shot_flight(defended.read_bytes(),area_ram(defense),378,'returned.goal_area.defenders')
        rows,last=menu_wait_events(area_lines,area_pcs,area_ram,defense,menu)
        operation+=1;events=output/f'{operation:03d}-goal-area-wait-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        waiting=output/f'{operation:03d}-goal-area-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(defended),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(area_ram(last)[0xe1])]:raise ValueError('boundary returned.goal_area.wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),area_ram(last),378,'returned.goal_area.wait')
        operation+=1;destination=output/f'{operation:03d}-goal-area-menu.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--open-ordinary-return-goal-area-menu','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.menu: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),area_ram(menu),379,'returned.goal_area.menu')
        if index!=area_ram(menu)[0xe1]:raise ValueError('boundary returned.goal_area.menu, random.index: original and native differ')
        source=destination
    area_poll_args=(args.area_poll_trace_directory,args.area_poll_entry_index,args.expected_area_polls)
    if any(value is not None for value in area_poll_args):
        if any(value is None for value in area_poll_args) or args.area_menu_index is None or args.expected_area_polls<1:
            raise ValueError('boundary returned.goal_area.poll: menu and all polling trace inputs are required')
        poll_lines,poll_pcs,poll_ram=trace_data(args.area_poll_trace_directory)
        entry=args.area_poll_entry_index
        if not 0<=entry<len(poll_pcs) or poll_pcs[entry]!='D5E5':raise ValueError('boundary returned.goal_area.poll: expected menu entry')
        original=area_ram(args.area_menu_index);followup=poll_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,original)):
            if value!=wanted:raise ValueError(f'boundary returned.goal_area.poll_handoff, RAM[0x{address:03x}]: follow-up {value}, menu {wanted}')
        previous=entry;area_count=0;confirmed=False
        for at in range(entry+1,len(poll_pcs)):
            if poll_pcs[at]!='D5EA' or at<=previous:continue
            label=f'returned.goal_area.poll[{area_count+1}]'
            rows,last=menu_wait_events(poll_lines,poll_pcs,poll_ram,previous,at)
            if rows:
                operation+=1;events=output/f'{operation:03d}-goal-poll-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
                waiting=output/f'{operation:03d}-goal-poll-wait.cmjsession'
                run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'boundary {label}.wait: {run.stderr.strip()}')
                indices=re.findall(r'final random index (\d+);',run.stdout)
                if indices!=[str(poll_ram(last)[0xe1])]:raise ValueError(f'boundary {label}.wait, random.index: original and native differ')
                source=waiting;index=int(indices[0])
            compare_shot_flight(source.read_bytes(),poll_ram(at),379,label+'.input')
            operation+=1;destination=output/f'{operation:03d}-goal-poll.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--menu-input',str(poll_ram(at)[0x1e]),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary {label}: {run.stderr.strip()}')
            end=next(j for j in range(at+1,len(poll_pcs)) if poll_pcs[j] in ('D5E5','D67C','D5EA'))
            if poll_pcs[end]=='D5EA':raise ValueError(f'boundary {label}: missing post-poll boundary')
            rows,last=menu_wait_events(poll_lines,poll_pcs,poll_ram,at,end) if any(poll_pcs[j]=='C9C5' for j in range(at+1,end)) else ([],at)
            if rows:
                operation+=1;events=output/f'{operation:03d}-goal-poll-after-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
                waiting=output/f'{operation:03d}-goal-poll-after.cmjsession'
                run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
                if run.returncode:raise RuntimeError(f'boundary {label}.after: {run.stderr.strip()}')
                indices=re.findall(r'final random index (\d+);',run.stdout)
                if indices!=[str(poll_ram(last)[0xe1])]:raise ValueError(f'boundary {label}.after, random.index: original and native differ')
                destination=waiting;index=int(indices[0])
            phase=380 if poll_pcs[end]=='D67C' else 379
            compare_shot_flight(destination.read_bytes(),poll_ram(end),phase,label+'.result')
            if index!=poll_ram(end)[0xe1]:raise ValueError(f'boundary {label}.result, random.index: original and native differ')
            source=destination;previous=end;area_count+=1
            if phase==380:confirmed=True;break
        if not confirmed or area_count!=args.expected_area_polls:
            raise ValueError(f'Expected {args.expected_area_polls} goal-area polls through shot confirmation; got {area_count}, confirmed={confirmed}')
    volley_args=(args.volley_trace_directory,args.volley_entry_index,args.volley_prepare_index,args.volley_prepared_index)
    if any(value is not None for value in volley_args):
        if any(value is None for value in volley_args) or args.expected_area_polls is None:
            raise ValueError('boundary returned.goal_area.volley: confirmed menu and all volley trace inputs are required')
        volley_lines,volley_pcs,volley_ram=trace_data(args.volley_trace_directory)
        entry,prepare,prepared=volley_args[1:]
        if not 0<=entry<prepare<prepared<len(volley_pcs) or [volley_pcs[i] for i in (entry,prepare,prepared)]!=['D67C','8298','82A0'] or any(volley_ram(i)[0x24]!=26 for i in (entry,prepare,prepared)):
            raise ValueError('boundary returned.goal_area.volley: unsupported original ordering or bank')
        original=poll_ram(previous);followup=volley_ram(entry)
        for address,(value,wanted) in enumerate(zip(followup,original)):
            if value!=wanted:raise ValueError(f'boundary returned.goal_area.volley_handoff, RAM[0x{address:03x}]: follow-up {value}, menu {wanted}')
        if any(volley_pcs[i]=='C9C5' for i in range(prepare+1,prepared)):
            raise ValueError('boundary returned.goal_area.volley: unexpected random event during preparation')
        rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,entry,prepare)
        points=[i for i in range(entry+1,prepare) if volley_pcs[i] in ('C9C5','E3CA')]
        if len(points)!=len(rows):raise ValueError('boundary returned.goal_area.volley_wait: ambiguous subtype ordering')
        subtype=source.read_bytes()[292];ordered=[]
        for row,point in zip(rows,points):
            observed=volley_ram(point)[0x43c]
            if observed!=subtype:subtype=observed;ordered.append(f'U {subtype}')
            ordered.append(row)
        operation+=1;events=output/f'{operation:03d}-volley-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
        waiting=output/f'{operation:03d}-volley-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(volley_ram(prepare)[0xe1])]:raise ValueError('boundary returned.goal_area.volley_wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),volley_ram(prepare),380,'returned.goal_area.volley_wait')
        operation+=1;destination=output/f'{operation:03d}-volley-prepared.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--prepare-basic-shot','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_prepare: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),volley_ram(prepared),381,'returned.goal_area.volley_prepare')
        if index!=volley_ram(prepared)[0xe1]:raise ValueError('boundary returned.goal_area.volley_prepare, random.index: original and native differ')
        source=destination
    volley_clock_args=(args.volley_wait_index,args.volley_clock_entry_index,args.volley_clock_index,args.volley_clock_end_index)
    if any(value is not None for value in volley_clock_args):
        if any(value is None for value in volley_clock_args) or args.volley_prepared_index is None:
            raise ValueError('boundary returned.goal_area.volley_clock: prepared shot and all clock boundaries are required')
        wait_end,entry,clock,clock_end=volley_clock_args
        if not args.volley_prepared_index<wait_end<entry<clock<clock_end<len(volley_pcs) or [volley_pcs[i] for i in volley_clock_args]!=['82AA','90DD','C9C5','83F5'] or any(volley_ram(i)[0x24]!=26 for i in (wait_end,entry,clock_end)):
            raise ValueError('boundary returned.goal_area.volley_clock: unsupported original ordering or bank')
        rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,args.volley_prepared_index,wait_end)
        points=[i for i in range(args.volley_prepared_index+1,wait_end) if volley_pcs[i] in ('C9C5','E3CA')]
        if len(points)!=len(rows):raise ValueError('boundary returned.goal_area.volley_wait: ambiguous presentation sample ordering')
        values=[source.read_bytes()[299],source.read_bytes()[305]];ordered=[]
        for row,point in zip(rows,points):
            raw=volley_ram(point)
            for slot,(address,token) in enumerate(((0x517,'F'),(0x532,'M'))):
                if raw[address]!=values[slot]:values[slot]=raw[address];ordered.append(f'{token} {values[slot]}')
            ordered.append(row)
        operation+=1;events=output/f'{operation:03d}-volley-presentation-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
        waiting=output/f'{operation:03d}-volley-presentation-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(volley_ram(wait_end)[0xe1])]:raise ValueError('boundary returned.goal_area.volley_wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),volley_ram(wait_end),381,'returned.goal_area.volley_wait')
        operation+=1;entered=output/f'{operation:03d}-volley-clock-entry.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--begin-ordinary-return-goal-shot-clock','--capture',str(entered)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_clock_entry: {run.stderr.strip()}')
        compare_shot_flight(entered.read_bytes(),volley_ram(entry),382,'returned.goal_area.volley_clock_entry')
        operation+=1;clocked=output/f'{operation:03d}-volley-first-clock.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(entered),'--advance-ordinary-return-goal-shot-clock','--capture',str(clocked)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_clock: {run.stderr.strip()}')
        compare_shot_flight(clocked.read_bytes(),volley_ram(clock),383,'returned.goal_area.volley_clock')
        rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,clock-1,clock_end)
        operation+=1;events=output/f'{operation:03d}-volley-clock-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        waiting=output/f'{operation:03d}-volley-clock-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(clocked),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_clock_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(volley_ram(clock_end)[0xe1])]:raise ValueError('boundary returned.goal_area.volley_clock_wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),volley_ram(clock_end),383,'returned.goal_area.volley_clock_wait');source=waiting
    if args.volley_movement_index is not None:
        movement=args.volley_movement_index
        if args.volley_clock_end_index is None or not args.volley_clock_end_index<movement<len(volley_pcs) or volley_pcs[movement]!='8408' or volley_ram(movement)[0x24]!=26:
            raise ValueError('boundary returned.goal_area.volley_movement: unsupported original ordering or bank')
        rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,args.volley_clock_end_index,movement)
        if len(rows)!=44:raise ValueError(f'boundary returned.goal_area.volley_movement: expected 22 random/travel pairs, got {len(rows)//2}')
        operation+=1;events=output/f'{operation:03d}-volley-field-events.txt'
        events.write_text('CMJNMISEQ1\n'+''.join(rows[2*slot]+'\n'+rows[2*slot+1]+'\nP '+str(slot)+'\n' for slot in range(22)))
        moved=output/f'{operation:03d}-volley-field-moved.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-field-movement',str(events),str(index),'--capture',str(moved)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_movement: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(volley_ram(movement)[0xe1])]:raise ValueError('boundary returned.goal_area.volley_movement, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(moved.read_bytes(),volley_ram(movement),384,'returned.goal_area.volley_movement');source=moved
    dispatch_args=(args.volley_dispatch_clock_before_index,args.volley_dispatch_clock_index,args.volley_dispatch_before_index,args.volley_dispatch_index,args.volley_handoff_index)
    if any(value is not None for value in dispatch_args):
        if any(value is None for value in dispatch_args) or args.volley_movement_index is None:
            raise ValueError('boundary returned.goal_area.volley_dispatch: movement and all dispatch boundaries are required')
        clock_before,clock, before,dispatch,handoff=dispatch_args
        if not args.volley_movement_index<clock_before<clock<before<dispatch<handoff<len(volley_pcs) or [volley_pcs[i] for i in dispatch_args]!=['E406','800C','E406','847F','C618'] or volley_ram(dispatch)[0x24]!=26:
            raise ValueError('boundary returned.goal_area.volley_dispatch: unsupported original ordering or bank')
        def replay_dispatch_wait(source,start,end,phase,label):
            nonlocal operation,index
            rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,start,end+1)
            points=[i for i in range(start+1,end+1) if volley_pcs[i] in ('C9C5','E3CA')]
            if len(points)!=len(rows):raise ValueError(f'boundary {label}: ambiguous presentation sample ordering')
            values=[source.read_bytes()[299],source.read_bytes()[305]];ordered=[]
            for row,point in zip(rows,points):
                raw=volley_ram(point)
                for slot,(address,token) in enumerate(((0x517,'F'),(0x532,'M'))):
                    if raw[address]!=values[slot]:values[slot]=raw[address];ordered.append(f'{token} {values[slot]}')
                ordered.append(row)
            operation+=1;events=output/f'{operation:03d}-volley-dispatch-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(ordered)+'\n')
            waiting=output/f'{operation:03d}-volley-dispatch-wait.cmjsession'
            run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
            if run.returncode:raise RuntimeError(f'boundary {label}: {run.stderr.strip()}')
            indices=re.findall(r'final random index (\d+);',run.stdout)
            if indices!=[str(volley_ram(end)[0xe1])]:raise ValueError(f'boundary {label}, random.index: original and native differ')
            index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),volley_ram(end),phase,label)
            return waiting
        waiting=replay_dispatch_wait(source,args.volley_movement_index,clock_before,384,'returned.goal_area.volley_before_dispatch_clock')
        operation+=1;dispatched=output/f'{operation:03d}-volley-dispatched.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--advance-ordinary-return-goal-shot-dispatch','--capture',str(dispatched)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_dispatch: {run.stderr.strip()}')
        compare_shot_flight(dispatched.read_bytes(),volley_ram(clock),385,'returned.goal_area.volley_dispatch_clock')
        waiting=replay_dispatch_wait(dispatched,clock,before,385,'returned.goal_area.volley_dispatch_wait')
        compare_shot_flight(waiting.read_bytes(),volley_ram(dispatch),385,'returned.goal_area.volley_dispatch_ready')
        if any(volley_pcs[i]=='C9C5' for i in range(before+1,handoff)):
            raise ValueError('boundary returned.goal_area.volley_dispatch: unexpected random event after wait')
        operation+=1;destination=output/f'{operation:03d}-volley-handoff.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(waiting),'--handoff-ordinary-return-goal-shot','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_handoff: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),volley_ram(handoff),386,'returned.goal_area.volley_handoff');source=destination
    if args.volley_target_index is not None:
        target=args.volley_target_index
        if args.volley_handoff_index is None or not args.volley_handoff_index<target<len(volley_pcs) or volley_pcs[target]!='DD08' or volley_ram(target)[0x24]!=26:
            raise ValueError('boundary returned.goal_area.volley_target: unsupported original ordering or bank')
        if any(volley_pcs[i]=='C9C5' for i in range(args.volley_handoff_index+1,target+1)):
            raise ValueError('boundary returned.goal_area.volley_target: unexpected random event')
        operation+=1;destination=output/f'{operation:03d}-volley-target.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-shot-approach','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.volley_target: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),volley_ram(target),387,'returned.goal_area.volley_target');source=destination
    if args.volley_keeper_arrival_index is not None:
        arrival=args.volley_keeper_arrival_index
        if args.volley_target_index is None or not args.volley_target_index<arrival<len(volley_pcs) or volley_pcs[arrival]!='8506' or volley_ram(arrival)[0x24]!=26:
            raise ValueError('boundary returned.goal_area.keeper_arrival: unsupported original ordering or bank')
        if any(volley_pcs[i]=='C9C5' for i in range(args.volley_target_index+1,arrival+1)):
            raise ValueError('boundary returned.goal_area.keeper_arrival: unexpected random event')
        operation+=1;destination=output/f'{operation:03d}-volley-keeper-arrival.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-keeper-entry','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.keeper_arrival: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),volley_ram(arrival),388,'returned.goal_area.keeper_arrival');source=destination
    keeper_ready_args=(args.volley_keeper_ready_index,args.volley_keeper_wait_index,args.volley_keeper_precompare_index)
    if any(value is not None for value in keeper_ready_args):
        if any(value is None for value in keeper_ready_args) or args.volley_keeper_arrival_index is None:
            raise ValueError('boundary returned.goal_area.keeper_ready: arrival and all keeper boundaries are required')
        ready,wait,precompare=keeper_ready_args
        if not args.volley_keeper_arrival_index<ready<wait<precompare<len(volley_pcs) or [volley_pcs[i] for i in keeper_ready_args]!=['80D9','E406','856B'] or volley_ram(precompare)[0x24]!=26:
            raise ValueError('boundary returned.goal_area.keeper_ready: unsupported original ordering or bank')
        operation+=1;destination=output/f'{operation:03d}-keeper-ready.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--prepare-keeper-entry','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.keeper_ready: {run.stderr.strip()}')
        compare_shot_flight(destination.read_bytes(),volley_ram(ready),389,'returned.goal_area.keeper_ready')
        rows,last=menu_wait_events(volley_lines,volley_pcs,volley_ram,ready,wait+1)
        operation+=1;events=output/f'{operation:03d}-keeper-wait-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        waiting=output/f'{operation:03d}-keeper-wait.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--advance-observed-nmi-sequence',str(events),str(index),'--capture',str(waiting)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.keeper_wait: {run.stderr.strip()}')
        indices=re.findall(r'final random index (\d+);',run.stdout)
        if indices!=[str(volley_ram(wait)[0xe1])]:raise ValueError('boundary returned.goal_area.keeper_wait, random.index: original and native differ')
        index=int(indices[0]);compare_shot_flight(waiting.read_bytes(),volley_ram(wait),389,'returned.goal_area.keeper_wait')
        if any(volley_pcs[i]=='C9C5' for i in range(wait+1,precompare+1)):
            raise ValueError('boundary returned.goal_area.keeper_precompare: unexpected random event')
        compare_shot_flight(waiting.read_bytes(),volley_ram(precompare),389,'returned.goal_area.keeper_precompare');source=waiting
    result_args=(args.volley_keeper_result_ram,args.volley_keeper_presentation_index)
    if any(value is not None for value in result_args):
        if any(value is None for value in result_args) or args.volley_keeper_precompare_index is None:
            raise ValueError('boundary returned.goal_area.keeper_result: comparison, result RAM and presentation entry are required')
        presentation=args.volley_keeper_presentation_index
        if not args.volley_keeper_precompare_index<presentation<len(volley_pcs) or volley_pcs[presentation]!='800C' or volley_ram(presentation)[0x24]!=16:
            raise ValueError('boundary returned.goal_area.keeper_result: unsupported presentation ordering')
        result=args.volley_keeper_result_ram.read_bytes()
        before=volley_ram(args.volley_keeper_precompare_index)
        if len(result)!=2048 or (before[0x2b],before[0x43d],before[0x43e],result[0x612],result[0x616])!=(3,0,0,0,0):
            raise ValueError('boundary returned.goal_area.keeper_result: unsupported original choice or result')
        if any(before[i]!=result[i] for i in range(2048) if not 0x100<=i<0x200):
            raise ValueError('boundary returned.goal_area.keeper_result: original handoff differs outside CPU stack')
        operation+=1;destination=output/f'{operation:03d}-keeper-result.cmjsession'
        run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--resolve-prepared-keeper','0','0','3','--capture',str(destination)],capture_output=True,text=True,timeout=30)
        if run.returncode:raise RuntimeError(f'boundary returned.goal_area.keeper_result: {run.stderr.strip()}')
        actual=destination.read_bytes()
        compare_shot_flight(actual,result,390,'returned.goal_area.keeper_result')
        expected=bytes((1,0,result[0x612],result[0x43d],result[0x43e],result[0x445]))
        if actual[356:362]!=expected:raise ValueError('boundary returned.goal_area.keeper_result: keeper decision differs')
        compare_shot_flight(actual,volley_ram(presentation),390,'returned.goal_area.keeper_presentation_entry')
        source=destination
    print(f'Matched {count} returned menu polls through {"opponent goal-area goalkeeper result before presentation" if args.volley_keeper_result_ram is not None else "opponent goal-area goalkeeper timing boundary" if args.volley_keeper_precompare_index is not None else "opponent goal-area goalkeeper arrival" if args.volley_keeper_arrival_index is not None else "opponent goal-area shot target" if args.volley_target_index is not None else "opponent goal-area shot handoff" if args.volley_handoff_index is not None else "opponent goal-area field movement" if args.volley_movement_index is not None else "opponent goal-area first attack clock" if args.volley_clock_end_index is not None else "opponent goal-area volley preparation" if args.volley_prepared_index is not None else "opponent goal-area shot confirmation" if args.expected_area_polls is not None else "opponent goal-area menu" if args.area_menu_index is not None else "blocked-shot recovered owner" if args.owner_positioned_index is not None else "blocked-shot recovery handoff" if args.recovery_handoff_index is not None else "blocked-shot handler entry" if args.recovery_handler_index is not None else "shot flight defender comparison" if args.flight_defender_result_index is not None else "shot flight arrival" if args.flight_arrival_index is not None else "shot flight cells" if args.flight_cell_indices is not None else "shot flight setup" if args.flight_setup_index is not None else "shot target" if args.target_index is not None else "shot field movement" if args.movement_end_index is not None else "shot contact and dispatch" if args.contact_index is not None else "shot defender comparison" if args.defender_index is not None else "recorded defense and shot preparation" if args.defense_index is not None else "ordinary shot confirmation"}; final random index {index}; checkpoint {source}.')


if __name__=='__main__':
    try:
        main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Returned menu replay stopped: {error}',file=sys.stderr)
        sys.exit(1)

#!/usr/bin/env python3
"""Compare opponent20's first observed control polls with the original trace."""
import argparse
import re
import subprocess
import sys
from pathlib import Path

from export_phase_session import export_session
from verify_captured_control_chain import trace_data, clock_events
from verify_player7_pass_end_to_end import compare_represented, prepare_output
from verify_observed_nmi_world_round import read_nmi_trace


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory', 'source', 'pack', 'exe', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--second-cycle-polls',action='store_true')
    parser.add_argument('--third-cycle-polls',action='store_true')
    args = parser.parse_args()
    lines, pcs, ram = trace_data(args.trace_directory)
    if args.second_cycle_polls and args.third_cycle_polls:raise ValueError('Choose one later cycle')
    later=args.second_cycle_polls or args.third_cycle_polls
    first = 22848 if args.third_cycle_polls else 22566 if args.second_cycle_polls else 22273
    if pcs[first] != 'E145':
        raise ValueError('Unexpected initial opponent control boundary')
    output = prepare_output(args.output, Path(__file__).resolve().parents[1])
    source = args.source.resolve(); previous = first; index = ram(previous)[0xe1]
    for count in range(1, 11 if later else 12):
        at = next(i for i in range(previous+1, len(pcs)) if pcs[i]=='E14A')
        end = next(i for i in range(at+1, len(pcs)) if pcs[i] in ('E145', 'E1E7'))
        points = {pcs[i]:i for i in range(previous+1, at) if pcs[i] in ('C9C5','C9E8','E3CA','E3DE','E406')}
        if not all(pc in points for pc in ('C9C5','C9E8','E3CA','E3DE','E406')):
            raise ValueError(f'Poll {count}: missing random or travel event')
        nmi, travel, poll = ram(points['C9C5']), ram(points['E3CA']), ram(at)
        flags = re.search(r'\bP=([0-9A-Fa-f]{2})\b', lines[points['C9C5']])
        if flags is None or nmi[0xe1] != index:
            raise ValueError(f'Poll {count}: random input differs')
        speed = ram(points['E3DE'])[0x32] | ram(points['E3DE'])[0x33] << 8
        row = (poll[0x1c], poll[0x1e], nmi[0x300+index], nmi[0x700+index],
               nmi[(0x780+index)&2047], int(flags.group(1),16)&1, 1, travel[0x1c], speed&255, speed>>8)
        previous_mode=ram(previous)[0x62d]; mode=travel[0x62d]
        if mode not in (128,192) or poll[0x62d]!=mode:
            raise ValueError(f'Poll {count}: unexpected ball mode')
        sample=mode if mode!=previous_mode else 256
        events=output/f'{count:02d}-events.txt';events.write_text('CMJPOLLNMI2\n'+' '.join(map(str,row))+f' {sample}\n')
        target=output/f'{count:02d}.cmjsession'
        result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),
                               '--advance-observed-nmi-poll',str(events),str(index),'--capture',str(target)],
                              capture_output=True,text=True,timeout=30)
        if result.returncode: raise RuntimeError(f'Poll {count}: {result.stderr.strip()}')
        index=ram(end)[0xe1]
        if re.findall(r'final random index (\d+);',result.stdout)!=[str(index)]:
            raise ValueError(f'Poll {count}: random index differs')
        data=target.read_bytes();expected_phase=441 if args.third_cycle_polls else 437 if args.second_cycle_polls else (435 if pcs[end]=='E1E7' else 434)
        if int.from_bytes(data[258:260],'little')!=expected_phase or data[413]!=ram(end)[0x616] or data[418]!=ram(end)[0x61b]:
            raise ValueError(f'Poll {count}: phase, cursor or flight flag differs')
        compare_represented(data,export_session(ram(end)),True,'opponent20',f'poll {count}')
        source,previous=target,end
    if later:
        if previous!=(22918 if args.third_cycle_polls else 22636): raise ValueError('Unexpected later-cycle decision boundary')
        if args.third_cycle_polls:
            rejected=output/'decision-rejected.cmjsession';before=source.read_bytes()
            attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-poll',str(events),str(index),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
            if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Unmodeled third-cycle decision did not reject atomically')
        print(f'Matched 10 opponent20 later-cycle polls before the captured AI decision; random index {index}.')
        return
    if pcs[previous]!='E1E7': raise ValueError('Expected control-clock entry')
    rows=clock_events(lines,pcs,ram,previous,22373)
    if pcs[22366]!='E3CA' or ram(22364)[0x62d]!=128 or ram(22366)[0x62d]!=192:
        raise ValueError('Unexpected control-clock ball-mode change')
    third_travel=[i for i,row in enumerate(rows) if row.startswith('T ')]
    if len(third_travel)!=3: raise ValueError('Unexpected control-clock travel count')
    rows.insert(third_travel[2],'B 192')
    events=output/'clock-events.txt'; events.write_text('CMJCLOCKNMI1\n'+'\n'.join(rows)+'\n')
    target=output/'clock.cmjsession'
    result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),
                           '--advance-observed-nmi-clock',str(events),str(index),'--capture',str(target)],
                          capture_output=True,text=True,timeout=30)
    if result.returncode: raise RuntimeError(f'Clock: {result.stderr.strip()}')
    index=ram(22373)[0xe1]
    if re.findall(r'final random index (\d+);',result.stdout)!=[str(index)]:
        raise ValueError('Clock random index differs')
    data=target.read_bytes()
    if int.from_bytes(data[258:260],'little')!=436 or data[413]!=ram(22373)[0x616] or data[418]!=ram(22373)[0x61b]:
        raise ValueError('Clock phase, cursor or flight flag differs')
    compare_represented(data,export_session(ram(22373)),True,'opponent20','control clock')
    entry,original,samples,boundary=read_nmi_trace(args.trace_directory,19)
    if entry!=ram(22373) or original!=ram(22566) or boundary!='E145':
        raise ValueError('Unexpected opponent20 field round boundaries')
    travels=[i for i in range(22374,22566) if pcs[i]=='E3CA']
    if len(travels)!=22: raise ValueError('Unexpected field round travel count')
    previous_mode=entry[0x62d]; extended=[]
    for row,point in zip(samples,travels):
        mode=ram(point)[0x62d]
        if mode not in (128,192): raise ValueError('Unexpected field round ball mode')
        extended.append((*row,mode if mode!=previous_mode else 256));previous_mode=mode
    events=output/'field-events.txt';events.write_text('CMJWORLDNMI2\n'+''.join(' '.join(map(str,row))+'\n' for row in extended))
    source=target;target=output/'field.cmjsession'
    result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),
                           '--advance-observed-nmi-world-file',str(events),str(index),'--capture',str(target)],
                          capture_output=True,text=True,timeout=30)
    if result.returncode: raise RuntimeError(f'Field round: {result.stderr.strip()}')
    index=original[0xe1]
    if re.findall(r'final random index (\d+);',result.stdout)!=[str(index)]:raise ValueError('Field random index differs')
    data=target.read_bytes()
    if int.from_bytes(data[258:260],'little')!=437 or data[413]!=original[0x616] or data[418]!=original[0x61b]:
        raise ValueError('Field phase, cursor or flight flag differs')
    compare_represented(data,export_session(original),True,'opponent20','field round')
    rejected=output/'rejected.cmjsession'
    replay=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),
                           '--advance-observed-nmi-world-file',str(events),str(index),'--capture',str(rejected)],
                          capture_output=True,text=True,timeout=30)
    if replay.returncode==0 or rejected.exists() or target.read_bytes()!=data:
        raise ValueError('Repeated field round did not reject atomically')
    roundtrip=output/'field-roundtrip.cmjsession'
    result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--capture',str(roundtrip)],
                          capture_output=True,text=True,timeout=30)
    if result.returncode or roundtrip.read_bytes()!=data:
        raise ValueError('Field round snapshot did not roundtrip')
    print(f'Matched 11 opponent20 control polls, clock and 22-player field round; random index {index}.')


if __name__=='__main__':
    try: main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 control replay stopped: {error}',file=sys.stderr);sys.exit(1)

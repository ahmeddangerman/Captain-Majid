#!/usr/bin/env python3
"""Compare an opponent field round with captured waits within a player update."""
import argparse
import re
import subprocess
import sys
from pathlib import Path
from export_phase_session import export_session
from verify_captured_control_chain import trace_data
from verify_player7_pass_end_to_end import compare_represented, prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    parser.add_argument('--next-round',action='store_true')
    parser.add_argument('--fourth-round',action='store_true')
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    if args.next_round and args.fourth_round:raise ValueError('Choose one later round')
    start,end=(23226,23380) if args.fourth_round else (22950,23104) if args.next_round else (22673,22848)
    if (pcs[start],pcs[end])!=('E407','E145'):raise ValueError('Unexpected original round boundaries')
    polls=[i for i in range(start+1,end) if pcs[i]=='E412']
    tails=[i for i in range(start+1,end) if pcs[i]=='E48F']
    if len(polls)!=22 or len(tails)!=22:raise ValueError('Expected 22 player polls and tails')
    previous=start;mode=ram(start)[0x62d];records=[];extra_counts=[]
    def captured_event(first,stop,allow_no_random=False):
        nonlocal mode
        candidates=[i for i in range(first,stop) if pcs[i]=='C9C5']
        if len(candidates)>1 or (not candidates and not allow_no_random):raise ValueError('Expected one random event')
        n=candidates[0] if candidates else None
        travel=next(i for i in range(n+1 if n is not None else first,stop) if pcs[i]=='E3CA')
        loader=next(i for i in range(travel+1,stop) if pcs[i]=='E3DE')
        t=ram(travel);loaded=ram(loader)
        if n is not None:
            r=ram(n);idx=r[0xe1];flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[n])
            if flags is None:raise ValueError('Missing random carry')
            random=(r[0x300+idx],r[0x700+idx],r[(0x780+idx)&2047],int(flags.group(1),16)&1)
        else:random=(0,0,0,0)
        value=t[0x62d];sample=value if value!=mode else 256;mode=value
        if value not in (128,192):raise ValueError('Unexpected ball mode')
        return (*random,t[0x1c],loaded[0x32],loaded[0x33],sample,int(n is not None))
    for slot,(poll,tail) in enumerate(zip(polls,tails)):
        if not previous<poll<tail:raise ValueError('Unexpected poll/tail order')
        base=captured_event(previous+1,poll,allow_no_random=slot==0);sample=ram(poll)
        starts=[i for i in range(poll+1,tail) if pcs[i]=='C9C5'];extras=[]
        for j,begin in enumerate(starts):extras.extend(captured_event(begin,starts[j+1] if j+1<len(starts) else tail)[:8])
        records.append((sample[0x1c],sample[0x1e],*base[:4],1,*base[4:7],base[7],base[8],len(starts),*extras))
        extra_counts.append(len(starts));previous=tail
    expected_counts=[0]*22 if args.next_round or args.fourth_round else [0]*7+[4]+[0]*14
    if extra_counts!=expected_counts:raise ValueError(f'Unexpected interleaved waits: {extra_counts}')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);events=out/'events.txt'
    events.write_text('CMJWORLDNMI3\n'+''.join(' '.join(map(str,row))+'\n' for row in records))
    target=out/'field.cmjsession';result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),
        '--advance-observed-nmi-world-file',str(events),str(ram(start)[0xe1]),'--capture',str(target)],capture_output=True,text=True,timeout=30)
    if result.returncode:raise RuntimeError(result.stderr.strip())
    if re.findall(r'final random index (\d+);',result.stdout)!=[str(ram(end)[0xe1])]:raise ValueError('Field random index differs')
    data=target.read_bytes()
    if int.from_bytes(data[258:260],'little')!=(449 if args.fourth_round else 445 if args.next_round else 441) or data[413]!=ram(end)[0x616] or data[418]!=ram(end)[0x61b]:raise ValueError('Field phase or retained flags differ')
    compare_represented(data,export_session(ram(end)),True,'opponent20','interleaved field round')
    rejected=out/'rejected.cmjsession'
    replay=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--advance-observed-nmi-world-file',str(events),str(ram(end)[0xe1]),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if replay.returncode==0 or rejected.exists() or target.read_bytes()!=data:raise ValueError('Repeated field round did not reject atomically')
    invalid_rows=[list(row) for row in records];invalid_rows[0][11]=1
    invalid=out/'invalid-events.txt';invalid.write_text('CMJWORLDNMI3\n'+''.join(' '.join(map(str,row))+'\n' for row in invalid_rows))
    source_bytes=args.source.read_bytes()
    replay=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-nmi-world-file',str(invalid),str(ram(start)[0xe1]),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if replay.returncode==0 or rejected.exists() or args.source.read_bytes()!=source_bytes:raise ValueError('Incorrect random ordering did not reject atomically')
    roundtrip=out/'field-roundtrip.cmjsession';replay=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if replay.returncode or roundtrip.read_bytes()!=data:raise ValueError('Field snapshot roundtrip differs')
    print(f'Matched 22 player updates and {sum(extra_counts)} interleaved waits; random index {ram(end)[0xe1]}.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 waited field replay stopped: {error}',file=sys.stderr);sys.exit(1)

#!/usr/bin/env python3
"""Compare opponent21 control polls against the captured original."""
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
    for name in ('trace-directory','source','pack','exe','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    if pcs[26335]!='E145':raise ValueError('Unexpected control entry')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);source=args.source.resolve();previous=26335;index=95
    for count in range(1,12):
        at=next(i for i in range(previous+1,len(pcs)) if pcs[i]=='E14A')
        end=next(i for i in range(at+1,len(pcs)) if pcs[i] in ('E145','E1E7'))
        points={pcs[i]:i for i in range(previous+1,at) if pcs[i] in ('C9C5','C9E8','E3CA','E3DE','E406')}
        if len(points)!=5 or any(pcs[i]=='C9C5' for i in range(at+1,end)):raise ValueError('Unexpected control poll events')
        nmi,travel,poll=ram(points['C9C5']),ram(points['E3CA']),ram(at)
        flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[points['C9C5']])
        if flags is None or nmi[0xe1]!=index:raise ValueError('Unexpected random input')
        speed=ram(points['E3DE'])[0x32] | ram(points['E3DE'])[0x33]<<8
        row=(poll[0x1c],poll[0x1e],nmi[0x300+index],nmi[0x700+index],nmi[(0x780+index)&2047],int(flags.group(1),16)&1,1,travel[0x1c],speed&255,speed>>8)
        mode=travel[0x62d];sample=mode if mode!=ram(previous)[0x62d] else 256
        events=out/f'{count:02d}.events.txt';events.write_text('CMJPOLLNMI2\n'+' '.join(map(str,row))+f' {sample}\n')
        if count==4:
            invalid=out/'invalid.events.txt';invalid.write_text('CMJPOLLNMI2\n'+' '.join(map(str,row))+' 192\n')
            rejected=out/'rejected.cmjsession';before=source.read_bytes()
            attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-poll',str(invalid),str(index),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
            if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Invalid mode did not reject atomically')
        target=out/f'{count:02d}.cmjsession'
        result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-observed-nmi-poll',str(events),str(index),'--capture',str(target)],capture_output=True,text=True,timeout=30)
        if result.returncode:raise RuntimeError(f'Poll {count}: {result.stderr.strip()}')
        index=ram(end)[0xe1]
        if re.findall(r'final random index (\d+);',result.stdout)!=[str(index)]:raise ValueError('Random index differs')
        data=target.read_bytes();expected=10 if pcs[end]=='E1E7' else 9
        if data[258]!=expected:raise ValueError('Poll phase differs')
        compare_represented(data,export_session(ram(end)),False,'opponent21',f'poll {count}')
        source,previous=target,end
    if previous!=26412:raise ValueError('Unexpected control clock boundary')
    roundtrip=out/'roundtrip.cmjsession';result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if result.returncode or roundtrip.read_bytes()!=source.read_bytes():raise ValueError('Poll snapshot roundtrip differs')
    print(f'Matched 11 opponent21 control polls; random index {index}.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent21 control replay stopped: {error}',file=sys.stderr);sys.exit(1)

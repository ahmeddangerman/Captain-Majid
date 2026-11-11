#!/usr/bin/env python3
"""Compare the captured opponent20 cell change and explicitly observed choice."""
import argparse
import re
import subprocess
import sys
from pathlib import Path
from export_phase_session import export_session
from verify_captured_control_chain import trace_data, clock_events
from verify_player7_pass_end_to_end import compare_represented, prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):
        parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    if [pcs[i] for i in (22636,22637,22639,22640,22642,22643,22644)]!=['E145','C9C5','E3CA','E3DE','E14A','E19B','E1B2']:
        raise ValueError('Unexpected original opponent decision ordering')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);nmi,travel,poll=ram(22637),ram(22639),ram(22642)
    index=nmi[0xe1];flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[22637]);speed=ram(22640)[0x32]|ram(22640)[0x33]<<8
    if flags is None or index!=26 or travel[0x62d]!=ram(22636)[0x62d]:raise ValueError('Unexpected decision NMI inputs')
    row=(poll[0x1c],poll[0x1e],nmi[0x300+index],nmi[0x700+index],nmi[(0x780+index)&2047],int(flags.group(1),16)&1,1,travel[0x1c],speed&255,speed>>8)
    events=out/'poll.events.txt';events.write_text('CMJPOLLNMI1\n'+' '.join(map(str,row))+'\n')
    decision=out/'cell-decision.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-nmi-poll',str(events),str(index),'--capture',str(decision)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);',run.stdout)!=[str(ram(22643)[0xe1])]:raise ValueError('Decision random index differs')
    data=decision.read_bytes()
    if int.from_bytes(data[258:260],'little')!=438:raise ValueError('Unexpected cell-decision phase')
    compare_represented(data,export_session(ram(22643)),True,'opponent20','cell decision')
    rejected=out/'rejected.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-action','3','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or decision.read_bytes()!=data:raise ValueError('Unsupported action was not rejected atomically')
    selected=out/'captured-choice.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-action','2','--capture',str(selected)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    actual=selected.read_bytes()
    if int.from_bytes(actual[258:260],'little')!=439:raise ValueError('Unexpected captured-choice phase')
    compare_represented(actual,export_session(ram(22644)),True,'opponent20','captured choice')
    if actual[413]!=ram(22644)[0x616] or actual[418]!=ram(22644)[0x61b]:raise ValueError('Cursor or flight flag differs')
    roundtrip=out/'choice-roundtrip.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or roundtrip.read_bytes()!=actual:raise ValueError('Captured choice snapshot roundtrip differs')
    if pcs[22645]!='E1E7' or pcs[22673]!='E407' or ram(22659)[0x62d]!=128:
        raise ValueError('Unexpected opponent action clock boundaries')
    rows=clock_events(lines,pcs,ram,22645,22673)
    travel=[i for i,row in enumerate(rows) if row.startswith('T ')]
    if len(travel)!=4 or ram(22657)[0x62d]!=192:raise ValueError('Unexpected opponent action clock event order')
    rows.insert(travel[2],'B 128')
    events=out/'clock.events.txt';events.write_text('CMJCLOCKNMI1\n'+'\n'.join(rows)+'\n')
    clock=out/'clock.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--advance-observed-nmi-clock',str(events),'27','--capture',str(clock)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    if re.findall(r'final random index (\d+);',run.stdout)!=['32']:raise ValueError('Opponent clock random index differs')
    data=clock.read_bytes()
    if int.from_bytes(data[258:260],'little')!=440 or data[413]!=ram(22673)[0x616] or data[418]!=ram(22673)[0x61b]:raise ValueError('Opponent clock phase or flags differ')
    compare_represented(data,export_session(ram(22673)),True,'opponent20','captured action clock')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(clock),'--advance-observed-nmi-clock',str(events),'32','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or clock.read_bytes()!=data:raise ValueError('Repeated clock did not reject atomically')
    roundtrip=out/'clock-roundtrip.cmjsession'
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(clock),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or roundtrip.read_bytes()!=data:raise ValueError('Clock snapshot roundtrip differs')
    print('Matched opponent20 cell refresh, explicitly captured action2 and clock; random index 32.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 decision replay stopped: {error}',file=sys.stderr);sys.exit(1)

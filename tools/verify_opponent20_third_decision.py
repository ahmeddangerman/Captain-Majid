#!/usr/bin/env python3
"""Compare the next captured opponent20 cell decision and control clock."""
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
    for name in ('trace-directory','source','pack','exe','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    indices=(22918,22919,22921,22922,22924,22925,22926,22927,22950)
    if [pcs[i] for i in indices]!=['E145','C9C5','E3CA','E3DE','E14A','E19B','E1B2','E1E7','E407']:
        raise ValueError('Unexpected opponent decision ordering')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);nmi,travel,poll=ram(22919),ram(22921),ram(22924)
    index=nmi[0xe1];flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[22919]);speed=ram(22922)[0x32]|ram(22922)[0x33]<<8
    if flags is None or index!=67 or travel[0x62d]!=ram(22918)[0x62d]:raise ValueError('Unexpected decision random input')
    row=(poll[0x1c],poll[0x1e],nmi[0x300+index],nmi[0x700+index],nmi[(0x780+index)&2047],int(flags.group(1),16)&1,1,travel[0x1c],speed&255,speed>>8)
    events=out/'poll.events.txt';events.write_text('CMJPOLLNMI1\n'+' '.join(map(str,row))+'\n')
    decision=out/'decision.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-nmi-poll',str(events),str(index),'--capture',str(decision)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=decision.read_bytes()
    if int.from_bytes(data[258:260],'little')!=442 or re.findall(r'final random index (\d+);',run.stdout)!=['68']:raise ValueError('Decision phase or random index differs')
    compare_represented(data,export_session(ram(22925)),True,'opponent20','third cell decision')
    rejected=out/'rejected.cmjsession';before=data
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-action','3','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or decision.read_bytes()!=before:raise ValueError('Unsupported choice did not reject atomically')
    selected=out/'choice.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-action','2','--capture',str(selected)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=selected.read_bytes()
    if int.from_bytes(data[258:260],'little')!=443:raise ValueError('Unexpected captured choice phase')
    compare_represented(data,export_session(ram(22926)),True,'opponent20','third captured choice')
    rows=clock_events(lines,pcs,ram,22927,22950);travel_rows=[i for i,row in enumerate(rows) if row.startswith('T ')]
    if len(travel_rows)!=3 or ram(22934)[0x62d]!=192 or ram(22936)[0x62d]!=128:raise ValueError('Unexpected clock ball-mode sample')
    rows.insert(travel_rows[1],'B 128');events=out/'clock.events.txt';events.write_text('CMJCLOCKNMI1\n'+'\n'.join(rows)+'\n')
    target=out/'clock.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--advance-observed-nmi-clock',str(events),'68','--capture',str(target)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=target.read_bytes()
    if int.from_bytes(data[258:260],'little')!=444 or re.findall(r'final random index (\d+);',run.stdout)!=['72']:raise ValueError('Clock phase or random index differs')
    compare_represented(data,export_session(ram(22950)),True,'opponent20','third control clock')
    if data[413]!=ram(22950)[0x616] or data[418]!=ram(22950)[0x61b]:raise ValueError('Cursor or flight flag differs')
    roundtrip=out/'roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or roundtrip.read_bytes()!=data:raise ValueError('Clock snapshot roundtrip differs')
    print('Matched opponent20 third cell decision, captured action2 and clock; random index 72.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 third decision stopped: {error}',file=sys.stderr);sys.exit(1)

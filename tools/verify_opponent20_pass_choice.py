#!/usr/bin/env python3
"""Compare an opponent20 cell refresh and explicitly captured pass target."""
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
    points=(23451,23452,23454,23455,23457,23458,23459,23461)
    if [pcs[i] for i in points]!=['E145','C9C5','E3CA','E3DE','E14A','E19B','E1B2','E1CB']:
        raise ValueError('Unexpected opponent pass choice ordering')
    nmi,travel,poll=ram(23452),ram(23454),ram(23457);index=nmi[0xe1]
    flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[23452]);speed=ram(23455)[0x32]|ram(23455)[0x33]<<8
    if flags is None or index!=143 or travel[0x62d]!=ram(23451)[0x62d]:raise ValueError('Unexpected pass decision NMI')
    row=(poll[0x1c],poll[0x1e],nmi[0x300+index],nmi[0x700+index],nmi[(0x780+index)&2047],int(flags.group(1),16)&1,1,travel[0x1c],speed&255,speed>>8)
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);events=out/'poll.events.txt';events.write_text('CMJPOLLNMI1\n'+' '.join(map(str,row))+'\n')
    decision=out/'decision.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-nmi-poll',str(events),str(index),'--capture',str(decision)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=decision.read_bytes()
    if int.from_bytes(data[258:260],'little')!=450 or re.findall(r'final random index (\d+);',run.stdout)!=['144']:raise ValueError('Pass decision phase or random index differs')
    compare_represented(data,export_session(ram(23458)),True,'opponent20','pass cell decision')
    selected=out/'choice.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-pass','21','94','--capture',str(selected)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=selected.read_bytes()
    if int.from_bytes(data[258:260],'little')!=451:raise ValueError('Unexpected pass choice phase')
    compare_represented(data,export_session(ram(23459)),True,'opponent20','pass choice')
    if data[394:399]!=bytes((1,ram(23459)[0x5fc],ram(23459)[0x638],0,0)) or data[413]!=ram(23459)[0x616] or data[418]!=ram(23459)[0x61b]:
        raise ValueError('Captured recipient, target scratch or retained flags differ')
    rejected=out/'rejected.cmjsession';before=decision.read_bytes()
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(decision),'--capture-player2-opponent-pass','20','94','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or decision.read_bytes()!=before:raise ValueError('Invalid pass choice did not reject atomically')
    roundtrip=out/'roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or roundtrip.read_bytes()!=data:raise ValueError('Pass choice snapshot roundtrip differs')
    prepared=out/'prepared.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(selected),'--prepare-player2-opponent-pass','--capture',str(prepared)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    actual=prepared.read_bytes()
    if int.from_bytes(actual[258:260],'little')!=452:raise ValueError('Unexpected pass preparation phase')
    compare_represented(actual,export_session(ram(23461)),True,'opponent20','pass preparation')
    original=ram(23461)
    if actual[394:399]!=bytes((1,original[0x5fc],original[0x638],original[0x43f],original[0x440])):raise ValueError('Prepared pass recipient, target or cost differs')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(prepared),'--prepare-player2-opponent-pass','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or prepared.read_bytes()!=actual:raise ValueError('Repeated preparation did not reject atomically')
    prepared_roundtrip=out/'prepared-roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(prepared),'--capture',str(prepared_roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or prepared_roundtrip.read_bytes()!=actual:raise ValueError('Prepared pass snapshot roundtrip differs')
    print('Matched opponent20 cell refresh, captured pass to player21 and pass preparation; random index 144.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 pass choice stopped: {error}',file=sys.stderr);sys.exit(1)

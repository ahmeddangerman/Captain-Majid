#!/usr/bin/env python3
"""Replay observed opponent20 pass crossings using private original event samples."""
import argparse
import re
import subprocess
import sys
from pathlib import Path
from export_phase_session import export_session
from verify_captured_control_chain import trace_data,menu_wait_events
from verify_player7_pass_end_to_end import compare_represented,prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    boundaries=((24041,24047,24049),(24049,24055,24057),(24057,24063,24065))
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);source=args.source.resolve();index=211
    rejected=out/'rejected.cmjsession';before=source.read_bytes()
    attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--advance-player2-opponent-pass-flight','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Early flight step did not reject atomically')
    attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--snap-player2-opponent-pass-arrival','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Early arrival snap did not reject atomically')
    def run(option,target,*values):
        nonlocal source
        result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),option,*map(str,values),'--capture',str(target)],capture_output=True,text=True,timeout=30)
        if result.returncode:raise RuntimeError(result.stderr.strip())
        source=target;return result.stdout
    def compare(at,phase,label):
        actual=source.read_bytes();raw=ram(at)
        if int.from_bytes(actual[258:260],'little')!=phase:raise ValueError(f'{label}: phase differs')
        compare_represented(actual,export_session(raw),True,'opponent20',label)
        trajectory=bytes((1,))+raw[0x634:0x638]+raw[0x639:0x63d]+bytes((raw[0x62c],raw[0x61b]))
        if actual[345:356]!=trajectory or actual[413]!=raw[0x616] or actual[418]!=raw[0x61b]:raise ValueError(f'{label}: trajectory or retained flags differ')
    for step,(start,cross,ready) in enumerate(boundaries,1):
        if [pcs[i] for i in (start,cross,ready)]!=['E773','E7B4','E773']:raise ValueError('Unexpected flight crossings')
        rows,last=menu_wait_events(lines,pcs,ram,start,cross)
        if len(rows)!=2:raise ValueError('Expected one crossing NMI/travel pair')
        events=out/f'{step:02d}-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
        result=run('--advance-observed-nmi-sequence',out/f'{step:02d}-wait.cmjsession',events,index)
        index=ram(last)[0xe1]
        if re.findall(r'final random index (\d+);',result)!=[str(index)]:raise ValueError('Flight random index differs')
        run('--advance-player2-opponent-pass-flight',out/f'{step:02d}-cross.cmjsession');compare(cross,457,f'cross {step}')
        run('--resolve-player2-opponent-pass-cell',out/f'{step:02d}-ready.cmjsession');compare(ready,456,f'callback {step}')
    if ram(24065)[0x600]!=1 or ram(24065)[0x601]!=2:raise ValueError('Unexpected selected original defender')
    rows,last=menu_wait_events(lines,pcs,ram,24065,24071)
    if len(rows)!=2 or pcs[24071]!='E7C0' or pcs[24074]!='E7CC':raise ValueError('Unexpected original arrival ordering')
    events=out/'arrival-events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
    result=run('--advance-observed-nmi-sequence',out/'arrival-wait.cmjsession',events,index);index=ram(last)[0xe1]
    if re.findall(r'final random index (\d+);',result)!=[str(index)]:raise ValueError('Arrival random index differs')
    run('--advance-player2-opponent-pass-flight',out/'arrival.cmjsession');compare(24071,458,'arrival before snap')
    run('--snap-player2-opponent-pass-arrival',out/'snapped.cmjsession');compare(24074,459,'arrival snap')
    before=source.read_bytes();attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--resolve-player2-opponent-pass-cell','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Repeated callback did not reject atomically')
    attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--snap-player2-opponent-pass-arrival','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError('Repeated arrival snap did not reject atomically')
    roundtrip=out/'roundtrip.cmjsession';attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if attempt.returncode or roundtrip.read_bytes()!=before:raise ValueError('Flight snapshot roundtrip differs')
    print(f'Matched three pass crossings, defender2 selection and destination snap; random index {index}.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 pass flight stopped: {error}',file=sys.stderr);sys.exit(1)

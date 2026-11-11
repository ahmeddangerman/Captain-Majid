#!/usr/bin/env python3
"""Compare the selected defender's original opponent-pass preparation."""
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
    for name in ('trace-directory','source','pack','exe','output','comparison'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory)
    if [pcs[i] for i in (24074,24078,24084,24090)]!=['E7CC','8840','8845','886D']:raise ValueError('Unexpected defender preparation ordering')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);source=args.source.resolve()
    rejected=out/'rejected.cmjsession'
    def reject(option,*values):
        before=source.read_bytes()
        attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),option,*map(str,values),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
        if attempt.returncode==0 or rejected.exists() or source.read_bytes()!=before:raise ValueError(f'{option}: invalid operation did not reject atomically')
    reject('--prepare-player2-opponent-pass-defender')
    def run(option,target,*values):
        nonlocal source
        result=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),option,*map(str,values),'--capture',str(target)],capture_output=True,text=True,timeout=30)
        if result.returncode:raise RuntimeError(result.stderr.strip())
        source=target;return result.stdout
    def compare(at,phase,label):
        data=source.read_bytes();raw=ram(at)
        if int.from_bytes(data[258:260],'little')!=phase or data[413]!=raw[0x616] or data[418]!=raw[0x61b]:raise ValueError(f'{label}: phase or retained flags differ')
        compare_represented(data,export_session(raw),True,'opponent20',label)
        if data[345:356]!=bytes((1,))+raw[0x634:0x638]+raw[0x639:0x63d]+bytes((raw[0x62c],raw[0x61b])):raise ValueError(f'{label}: trajectory differs')
    run('--begin-player2-opponent-pass-defender',out/'begin.cmjsession');compare(24078,460,'defender loop')
    rows,last=menu_wait_events(lines,pcs,ram,24078,24084)
    if len(rows)!=2:raise ValueError('Expected one defender announcement NMI/travel pair')
    events=out/'wait.events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
    result=run('--advance-observed-nmi-sequence',out/'wait.cmjsession',events,215)
    if re.findall(r'final random index (\d+);',result)!=[str(ram(last)[0xe1])]:raise ValueError('Defender wait random index differs')
    compare(24084,460,'defender wait')
    run('--prepare-player2-opponent-pass-defender',out/'prepared.cmjsession');compare(24090,461,'defender prepared')
    reject('--restore-player2-opponent-pass-variant')
    malformed=out/'malformed.cmjpass';malformed.write_bytes(b'CMJPASS1')
    reject('--compare-player2-opponent-pass-defender',malformed)
    data=source.read_bytes();raw=ram(24090)
    expected=bytes((1,0,2,0,raw[0x444],raw[0x32],raw[0x33],raw[0x43f],raw[0x440],0,0,0,0,0))
    if data[399:413]!=expected:raise ValueError('Defender preparation context differs')
    run('--compare-player2-opponent-pass-defender',out/'compared.cmjsession',args.comparison.resolve());compare(24092,462,'defender comparison')
    data=source.read_bytes();raw=ram(24092);registers=re.search(r'\bA=([0-9A-F]{2}).*\bY=([0-9A-F]{2})',lines[24092])
    if registers is None:raise ValueError('Missing original defender comparison registers')
    score,result=(int(value,16) for value in registers.groups())
    if data[408:413]!=bytes((raw[0x71],raw[0x619],score,result,0)) or data[414]!=result:
        raise ValueError('Defender comparison result differs')
    reject('--compare-player2-opponent-pass-defender',args.comparison.resolve())
    run('--restore-player2-opponent-pass-variant',out/'restored.cmjsession');compare(24094,463,'defender variant restored')
    reject('--restore-player2-opponent-pass-variant')
    before=source.read_bytes();roundtrip=out/'roundtrip.cmjsession'
    attempt=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(source),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if attempt.returncode or roundtrip.read_bytes()!=before:raise ValueError('Defender snapshot roundtrip differs')
    print(f'Matched opponent pass defender2 preparation and result {result}; random index 216.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 defender replay stopped: {error}',file=sys.stderr);sys.exit(1)

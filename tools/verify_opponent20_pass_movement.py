#!/usr/bin/env python3
"""Replay observed opponent20 pass field movement, including presentation samples."""
import argparse
import re
import subprocess
import sys
from pathlib import Path
from export_phase_session import export_session
from verify_captured_control_chain import trace_data
from verify_player7_pass_end_to_end import compare_represented,prepare_output


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('trace-directory','source','pack','exe','output','distance-table','velocity'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();lines,pcs,ram=trace_data(args.trace_directory);start,end=23515,24037
    if (pcs[start],pcs[end])!=('C606','C612'):raise ValueError('Unexpected pass field boundaries')
    mode={'F':ram(start)[0x517],'M':ram(start)[0x532],'D':ram(start)[0x8e]};samples=[];rows=[];slot=0
    for i in range(start+1,end):
        pc=pcs[i];observed=ram(i) if pc in ('C9C5','E3CA','E0D1') else None
        if observed is None:continue
        for key,address in (('F',0x517),('M',0x532),('D',0x8e)):
            value=observed[address]
            if value!=mode[key]:mode[key]=value;samples.append((key,value));rows.append(f'{key} {value}')
        if pc=='C9C5':
            index=observed[0xe1];flags=re.search(r'\bP=([0-9A-Fa-f]{2})\b',lines[i])
            if flags is None:raise ValueError('Missing random carry')
            rows.append('R '+' '.join(map(str,(observed[0x300+index],observed[0x700+index],observed[(0x780+index)&2047],int(flags.group(1),16)&1))))
        elif pc=='E3CA':
            loader=next(j for j in range(i+1,end) if pcs[j] in ('E3DE','E406'))
            if pcs[loader]!='E3DE':raise ValueError('Missing observed travel speed')
            speed=ram(loader)[0x32]|ram(loader)[0x33]<<8
            rows.append(f'T {observed[0x1c]} {speed&255} {speed>>8}')
        else:rows.append(f'P {slot}');slot+=1
    if slot!=22 or sum(row.startswith('R ') for row in rows)!=57 or samples!=[('M',0),('F',64),('D',0)]:
        raise ValueError(f'Unexpected movement events: slots={slot}, samples={samples}')
    if (ram(24036)[0x44e],ram(24036)[0x617],ram(end)[0x44e],ram(end)[0x617])!=(0,0,2,128):raise ValueError('Unexpected pass handoff variant/carrier sample')
    rows.append('V 2')
    out=prepare_output(args.output,Path(__file__).resolve().parents[1]);events=out/'movement.events.txt';events.write_text('CMJNMISEQ1\n'+'\n'.join(rows)+'\n')
    target=out/'movement.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-pass-movement',str(events),'154','--capture',str(target)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    data=target.read_bytes()
    if int.from_bytes(data[258:260],'little')!=454 or re.findall(r'final random index (\d+);',run.stdout)!=['211']:
        raise ValueError('Unexpected pass movement phase or random index')
    compare_represented(data,export_session(ram(end)),True,'opponent20','pass movement')
    if data[394:399]!=bytes((1,21,94,20,0)) or data[413]!=ram(end)[0x616] or data[418]!=ram(end)[0x61b]:raise ValueError('Pass context or flags differ')
    rejected=out/'rejected.cmjsession';before=args.source.read_bytes()
    incomplete=out/'incomplete.events.txt';incomplete.write_text('CMJNMISEQ1\n'+'\n'.join(row for row in rows if row!='D 0')+'\n')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(args.source.resolve()),'--advance-observed-pass-movement',str(incomplete),'154','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or args.source.read_bytes()!=before:raise ValueError('Incomplete presentation samples did not reject atomically')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--advance-observed-pass-movement',str(events),'211','--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or target.read_bytes()!=data:raise ValueError('Repeated movement did not reject atomically')
    roundtrip=out/'roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--capture',str(roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or roundtrip.read_bytes()!=data:raise ValueError('Movement snapshot roundtrip differs')
    destination=out/'target.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--resolve-player2-opponent-pass-target',str(args.distance_table.resolve()),'--capture',str(destination)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    resolved=destination.read_bytes();original=ram(24040)
    if int.from_bytes(resolved[258:260],'little')!=455 or resolved[340:345]!=bytes((1,original[0x62b],original[0x638],original[0x61a],0)):
        raise ValueError('Pass target or distance differs')
    compare_represented(resolved,export_session(original),True,'opponent20','pass target')
    malformed=out/'invalid-distance.bin';malformed.write_bytes(b'bad')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(target),'--resolve-player2-opponent-pass-target',str(malformed),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or target.read_bytes()!=data:raise ValueError('Malformed distance did not reject atomically')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--resolve-player2-opponent-pass-target',str(args.distance_table.resolve()),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or destination.read_bytes()!=resolved:raise ValueError('Repeated target did not reject atomically')
    target_roundtrip=out/'target-roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--capture',str(target_roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or target_roundtrip.read_bytes()!=resolved:raise ValueError('Target snapshot roundtrip differs')
    flight=out/'flight.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--begin-player2-opponent-pass-flight',str(args.velocity.resolve()),str(original[0x634]),str(original[0x636]),'--capture',str(flight)],capture_output=True,text=True,timeout=30)
    if run.returncode:raise RuntimeError(run.stderr.strip())
    actual=flight.read_bytes();flight_original=ram(24041)
    if int.from_bytes(actual[258:260],'little')!=456 or actual[345:356]!=bytes((1,))+flight_original[0x634:0x638]+flight_original[0x639:0x63d]+bytes((flight_original[0x62c],flight_original[0x61b])):
        raise ValueError('Opponent pass trajectory setup differs')
    compare_represented(actual,export_session(flight_original),True,'opponent20','pass flight setup')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(destination),'--begin-player2-opponent-pass-flight',str(args.velocity.resolve()),'0',str(original[0x636]),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or destination.read_bytes()!=resolved:raise ValueError('Unobserved flight fraction did not reject atomically')
    run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(flight),'--begin-player2-opponent-pass-flight',str(args.velocity.resolve()),str(original[0x634]),str(original[0x636]),'--capture',str(rejected)],capture_output=True,text=True,timeout=30)
    if run.returncode==0 or rejected.exists() or flight.read_bytes()!=actual:raise ValueError('Repeated flight setup did not reject atomically')
    flight_roundtrip=out/'flight-roundtrip.cmjsession';run=subprocess.run([str(args.exe.resolve()),str(args.pack.resolve()),str(flight),'--capture',str(flight_roundtrip)],capture_output=True,text=True,timeout=30)
    if run.returncode or flight_roundtrip.read_bytes()!=actual:raise ValueError('Flight snapshot roundtrip differs')
    print('Matched opponent20 pass field movement, calculated target and flight setup; random index 211.')


if __name__=='__main__':
    try:main()
    except (OSError,ValueError,RuntimeError,StopIteration,subprocess.SubprocessError) as error:
        print(f'Opponent20 pass movement stopped: {error}',file=sys.stderr);sys.exit(1)

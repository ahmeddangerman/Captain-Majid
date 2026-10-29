#!/usr/bin/env python3
"""Compare saved native shot approach with approved private original inputs."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from check_attack_clock_original import checkpoint
from trace_shot_approach import trace_approach
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected side0 ordinary shot RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must be outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());results=[]
    for name,x,y,random,zero in [('loose',120,100,101,False),('threshold127',127,100,100,False),
            ('threshold128',128,100,101,False),('keeper-even',160,96,100,False),
            ('keeper-odd',180,159,101,False),('unopposed',160,100,101,True),('subtype128',120,100,101,False)]:
        ram=bytearray(base);ram[0x635]=x;ram[0x637]=y;ram[0xE2]=random;ram[0x617]=0;ram[0x43C]=128 if name=="subtype128" else 0
        if zero:ram[0x600:0x606]=bytes(6);ram[0x612]=0
        entry=out/(name+'-entry.cmjsession');entry.write_bytes(checkpoint(ram,45));(out/(name+'-entry.bin')).write_bytes(ram)
        report,final=trace_approach(rom[16:262160],bytes(ram));
        if report['loose_ball']!=(x<128 and name!='subtype128'):raise RuntimeError('Fixture does not exercise its intended ground/aerial route')
        (out/(name+'-final.bin')).write_bytes(final)
        (out/(name+'.json')).write_text(json.dumps(report,indent=2)+'\n')
        expected=bytearray(checkpoint(final,46));expected[339:344]=bytes([1,report['distance'],report['destination'],report['counter'],int(report['loose_ball'])])
        dest=out/(name+'-native.cmjsession')
        subprocess.run([binary,pack,str(entry),'--prepare-shot-approach','--capture',str(dest)],capture_output=True,check=True,timeout=30)
        actual=dest.read_bytes()
        if actual!=expected:raise RuntimeError(f'{name}: mismatch{[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
        if subprocess.run([binary,pack,str(dest),'--prepare-shot-approach'],capture_output=True,timeout=30).returncode==0:raise RuntimeError('Replay accepted')
        results.append({'case':name,'matches':True,'distance':report['distance'],'destination':report['destination'],'loose_ball':report['loose_ball']})
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'324prior represented bytes plus062B/0638/061A and carry; native context presence excluded from ROM-memory claim.'},indent=2)+'\n')
    print(f'{len(results)} original/native shot approach cases passed: {out}')


if __name__=='__main__':main()

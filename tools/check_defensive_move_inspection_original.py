#!/usr/bin/env python3
"""Compare defensive move cost loading and affordability against the private ROM.

The observer executes the original loader at $8224, then the menu's wrapped
stamina check from $D76B. It excludes the menu, warning script and stamina debit.
"""
import argparse
import hashlib
import subprocess
from pathlib import Path
from verify_action_strength import PAYLOAD_SHA256
from verify_field_defender_preparation import cpu_for, original


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom',type=Path)
    parser.add_argument('pack',type=Path)
    parser.add_argument('--binary',type=Path,default=Path('build/defensive-move-inspection-tests'))
    args=parser.parse_args()
    rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:
        parser.error('Expected approved Japanese ROM')
    cpu=cpu_for(rom[16:262160]);ram=cpu.memory
    cases=[]
    stamina_values=(0,49,50,51,127,128,179,180,181,255,256,4096,0x237f,0x2380,65535)
    for kind in (23,24,48,70):
        for level in (0,15,127,255):
            for variant in (1,2):
                for subtype in (0,1):
                    for stamina in stamina_values:
                        # Existing profile reader intentionally rejects unrepresented
                        # alternate encodings; do not claim equivalence for them.
                        if kind>=35 and stamina&128 and stamina>>8<35:continue
                        actor=1+len(cases)%10
                        cases.append((actor,kind,level,2,variant,subtype,stamina))
    lines=''.join(' '.join(map(str,case))+'\n' for case in cases)
    run=subprocess.run([str(args.binary.resolve()),'--batch',str(args.pack.resolve())],
        input=lines,text=True,capture_output=True,timeout=60,check=True)
    outputs=run.stdout.splitlines()
    if len(outputs)!=len(cases):raise RuntimeError('Native result count mismatch')
    for case,line in zip(cases,outputs):
        actor,kind,level,action,variant,subtype,stamina=case
        _,_,_,effect,cost=original(cpu,case[:6],flags=stamina&255,alternate_type=stamina>>8)
        before=ram[0x301+12*actor] | ram[0x302+12*actor]<<8
        ram[0x1fe],ram[0x1ff]=0x5a,0xd7
        cpu.pc,cpu.sp,cpu.p=0xd76b,0xfd,cpu.UNUSED
        for _ in range(30):
            if cpu.pc in (0xd762,0xd769):break
            cpu.step()
        else:raise RuntimeError('Affordability instruction bound')
        expected=f'{cost} {int(cpu.pc==0xd769)} {effect}'
        after=ram[0x301+12*actor] | ram[0x302+12*actor]<<8
        if line!=expected or before!=stamina or after!=stamina:
            raise RuntimeError(f'Cost/decision mismatch for {case}: {line!r} != {expected!r}, stamina {before}/{after}')
    print(f'{len(cases)} original defensive move cost, effect and affordability cases match; no debit')


if __name__=='__main__':main()

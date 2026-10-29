#!/usr/bin/env python3
"""Compare unopposed flight return routing and prepared no-jitter recovery."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from trace_ball_recovery_phase import trace_recovery
from flight_checkpoint import flight_checkpoint
from export_phase_session import export_session
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected side0 ordinary flight RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());results=[]
    for name,x,loose in [('interior',156,True),('goal-area',196,True),('keeper-bound',204,False),('opponent',156,True)]:
        ram=bytearray(base);ram[0x600:0x606]=bytes(6);ram[0x5FF]=0;ram[0x635]=x;ram[0x637]=124
        ram[0x5FE]=((x-48)//8)*12+5;ram[0x638]=ram[0x5FE];ram[0x61B]=0;ram[0x43C]=0;ram[0xE2]=128 if name=="opponent" else 1;ram[0xE3]=0
        if name=="opponent":
            for slot in (*range(1,11),*range(12,22)):
                record=0x300+slot*12;ram[record+6]=156;ram[record+8]=124;ram[record+10]=0
        entry=out/(name+'-entry.cmjsession');entry.write_bytes(flight_checkpoint(ram,49,loose));(out/(name+'-entry.bin')).write_bytes(ram)
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xDD11;cpu.sp=0xFD;cpu.p=cpu.UNUSED;m[0x1FE]=cpu.UNUSED|int(loose)
        for step in range(10):
            if cpu.pc in (0xDD14,0xDD1C):break
            cpu.step()
        else:raise RuntimeError('Flight return branch bound')
        phase=51 if cpu.pc==0xDD14 else 52;routed=out/(name+'-routed.cmjsession')
        subprocess.run([binary,pack,str(entry),'--continue-flight-arrival','--capture',str(routed)],capture_output=True,check=True,timeout=30)
        if routed.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),phase,loose):raise RuntimeError('Flight return route mismatch')
        if subprocess.run([binary,pack,str(routed),'--continue-flight-arrival'],capture_output=True).returncode==0:raise RuntimeError('Arrival branch replay accepted')
        record={'case':name,'route_matches':True,'phase':phase}
        if loose:
            # Same explicit post-presentation numeric state for both native/ROM8BE5 entries.
            report,final=trace_recovery(rom[16:262160],bytes(ram),skip_jitter=True)
            (out/(name+'-recovery.json')).write_text(json.dumps(report,indent=2)+'\n');(out/(name+'-final.bin')).write_bytes(final)
            dest=out/(name+'-recovered.cmjsession')
            subprocess.run([binary,pack,str(routed),'--recover-shot-loose','--capture',str(dest)],capture_output=True,check=True,timeout=30)
            expected=bytearray(export_session(final,6));expected[10]=ord('W');expected+=bytes(46)
            actual=dest.read_bytes()
            if actual!=expected:raise RuntimeError(f'{name}recovery mismatch{[(i,x,y)for i,(x,y)in enumerate(zip(actual,expected))if x!=y]}')
            if subprocess.run([binary,pack,str(dest),'--recover-shot-loose'],capture_output=True).returncode==0:raise RuntimeError('Recovery replay accepted')
            continued=out/(name+'-continued.cmjsession');subprocess.run([binary,pack,str(dest),'--continue-recovery','--capture',str(continued)],capture_output=True,check=True,timeout=30)
            wanted_phase=7 if x>=176 else 1
            if continued.read_bytes()[258]!=wanted_phase:raise RuntimeError('Prepared recovery continuation route mismatch')
            record.update(recovery_matches=True,native_continuation_phase=wanted_phase,side=final[0x5FB],owner=final[0x441])
            if name=="opponent" and final[0x5FB]!=11:raise RuntimeError("Opposing tie case did not transfer possession")
            if wanted_phase==1:
                control_ready=out/(name+'-control-ready.cmjsession')
                subprocess.run([binary,pack,str(continued),'--setup-control','--capture',str(control_ready)],capture_output=True,check=True,timeout=30)
                if control_ready.read_bytes()[258]!=9:raise RuntimeError('Recovered owner did not reach native control-ready')
        results.append(record)
    (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'DD11 branch and explicitly prepared8BE5 recovery. Display45 is not executed or claimed equivalent. Native continuation route checked separately, no live presentation/full-match claim.'},indent=2)+'\n')
    print(f'{len(results)} original flight return branches and3no-jitter recoveries match: {out}')


if __name__=='__main__':main()

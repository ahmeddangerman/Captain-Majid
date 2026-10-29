#!/usr/bin/env python3
"""Compare full prepared numeric flight with native save/reload at every cell."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from flight_checkpoint import flight_checkpoint
from check_keeper_entry_original import keeper_checkpoint
from check_keeper_comparison_original import observe_keeper_comparison
from check_keeper_saves_original import observe_keeper_save, post_save_checkpoint
from check_scored_goal_original import observe_scored_goal
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for arg in ('rom','pack','ram','projection','output_dir'):p.add_argument(arg,type=Path)
    p.add_argument('--binary',type=Path,default=Path('build/captain-phase-session'))
    a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
    if len(base)!=2048 or base[0x5FB]!=0 or base[0x43B]!=0:p.error('Expected ordinary side0 shot RAM')
    out=a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private output must stay outside checkout')
    out.mkdir(parents=True,exist_ok=False);binary=str(a.binary.resolve());pack=str(a.pack.resolve());projection=str(a.projection.resolve())
    def original(ram):
        m=ObservedCartridge(rom[16:262160],bytes(ram),None,set());m.registers[6:8]=[26,27]
        cpu=MPU(memory=m);cpu.pc=0xE73E;cpu.sp=0xFC;cpu.p=cpu.UNUSED
        m[0x1FD],m[0x1FE],m[0x1FF]=0x10,0xDD,cpu.UNUSED|int(ram[0x43C]==0 and ram[0x635]<128)
        return m,cpu
    def until(cpu,targets):
        for count in range(2_000_000):
            if cpu.pc in targets:return count
            cpu.step()
        raise RuntimeError('Prepared flight original instruction bound')
    def native(source,operation,dest):
        subprocess.run([binary,pack,str(source),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
    cases=[]
    for name,x,y,dest,zero in [('loose-with-defenders',120,100,221,False),('keeper-bound',160,96,233,True),('loose-free',80,100,161,True),('keeper-ready-free',160,96,233,True),('keeper-stunned-free',160,96,233,True),('keeper-catch-free',160,96,233,True),('keeper-punch-free',160,96,233,True),('keeper-goal-free',160,96,233,True)]:
        ram=bytearray(base);ram[0x634:0x638]=((x<<8)|64).to_bytes(2,'little')+((y<<8)|192).to_bytes(2,'little')
        ram[0x5FE]=((x-48)//8)*12+(y-80)//8;ram[0x638]=dest;ram[0x61A]=255;ram[0x61B]=0;ram[0x617]=0;ram[0x43C]=0;ram[0xE2]=1;ram[0xE3]=0
        if zero:ram[0x600:0x606]=bytes(6);ram[0x612]=0
        if name.startswith("keeper-") and name.endswith("free"):ram[0x38E]=int(name=="keeper-stunned-free")
        if name in ("keeper-catch-free","keeper-punch-free"):ram[0x61C:0x61E]=bytes(2)
        if name=="keeper-goal-free":ram[0x61C:0x61E]=(65535).to_bytes(2,"little")
        # Freeze candidates in the first visited cell; positions are explicit input.
        probe,cp=original(ram);until(cp,{0xE773});cp.pc=0xE778;until(cp,{0xE7B4,0xE7CC})
        for slot in range(12,22):
            r=0x300+12*slot;ram[r+6]=200 if name.endswith("free") else probe[0x635];ram[r+8]=84 if name.endswith("free") else probe[0x637];ram[r+10]=int(slot==12)
        m,cpu=original(ram);until(cpu,{0xE773})
        source=out/(name+'-entry.cmjsession');source.write_bytes(flight_checkpoint(ram,46,x<128));(out/(name+'-entry.bin')).write_bytes(ram)
        ready=out/(name+'-ready.cmjsession');native(source,['--begin-ball-flight',projection,'64','192'],ready)
        if ready.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),47,x<128):raise RuntimeError('Setup mismatch')
        boundaries=[]
        for cell in range(100):
            cpu.pc=0xE778  # each frame wait acknowledged explicitly; no task/NMI update
            until(cpu,{0xE7B4,0xE7CC});phase=48 if cpu.pc==0xE7B4 else 49
            stepped=out/(name+f'-{cell}-step.cmjsession');native(ready,['--advance-ball-cell'],stepped)
            if stepped.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),phase,x<128):raise RuntimeError(f'{name}step{cell} mismatch')
            boundaries.append({'cell':m[0x5FE],'phase':phase,'selected':m[0x600]})
            if phase==49:break
            until(cpu,{0xE773})  # execute complete original800F/87E1 callback and return
            ready=out/(name+f'-{cell}-continued.cmjsession');native(stepped,['--resolve-ball-cell'],ready)
            if ready.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),47,x<128):raise RuntimeError(f'{name}callback{cell} mismatch')
        else:raise RuntimeError('Prepared flight did not arrive within100cell callbacks')
        (out/(name+'-final.bin')).write_bytes(m.ram[:2048])
        result={'case':name,'boundaries':boundaries,'final_selected':list(m.ram[0x601:0x601+m[0x600]]),'match':True}
        if name=='loose-free':
            until(cpu,{0xDD14,0xDD1C})  # empty8835 callback, return and saved carry branch
            routed=out/'loose-free-routed.cmjsession';native(stepped,['--continue-flight-arrival'],routed)
            if cpu.pc!=0xDD14 or routed.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),51,True):raise RuntimeError('Complete loose return mismatch')
            report,recovered=trace_recovery(rom[16:262160],bytes(m.ram[:2048]),skip_jitter=True)
            (out/'loose-free-recovery.json').write_text(json.dumps(report,indent=2)+'\n')
            dest=out/'loose-free-recovered.cmjsession';native(routed,['--recover-shot-loose'],dest)
            expected=bytearray(export_session(recovered,6));expected[10]=ord('i');expected+=bytes(46)
            if dest.read_bytes()!=expected:raise RuntimeError('Complete loose recovery mismatch')
            controlled=out/'loose-free-control.cmjsession';native(dest,['--continue-recovery'],controlled)
            ready_control=out/'loose-free-control-ready.cmjsession';native(controlled,['--setup-control'],ready_control)
            if ready_control.read_bytes()[258]!=9:raise RuntimeError('Loose flight did not rejoin native control-ready state')
            result['native_control_ready']=True;result['recovery_matches']=True
        if name.startswith('keeper-') and name.endswith('free'):
            until(cpu,{0xDD1C})  # empty arrival, saved loose carry clear
            routed=out/(name+'-routed.cmjsession');native(stepped,['--continue-flight-arrival'],routed)
            if routed.read_bytes()!=flight_checkpoint(bytes(m.ram[:2048]),52,False):raise RuntimeError('Complete keeper return mismatch')
            until(cpu,{0x8509,0x8900});phase=53 if cpu.pc==0x8509 else 54
            prepared=out/(name+'-prepared.cmjsession');native(routed,['--prepare-keeper-entry'],prepared)
            if prepared.read_bytes()!=keeper_checkpoint(bytes(m.ram[:2048]),phase):raise RuntimeError('Complete keeper setup mismatch')
            result['keeper_prepared_phase']=phase
            if phase==53:
                # Explicit prepared keeper choice boundary: no display2/menu effects.
                ram_for_keeper=bytearray(m.ram[:2048]);ram_for_keeper[0x43D]=int(name=="keeper-punch-free");ram_for_keeper[0x43E]=0
                keeper_final,instructions=observe_keeper_comparison(rom[16:262160],ram_for_keeper)
                keeper_saved=out/(name+'-keeper-decided.cmjsession')
                native(prepared,['--resolve-prepared-keeper',str(ram_for_keeper[0x43D]),'0',str(ram_for_keeper[0x2B])],keeper_saved)
                if keeper_saved.read_bytes()!=keeper_checkpoint(keeper_final,56):raise RuntimeError('Complete prepared keeper comparison mismatch')
                result['keeper_result']=keeper_final[0x612];result['keeper_comparison_instructions']=instructions
                if keeper_final[0x612]==0 or (keeper_final[0x612]==1 and (keeper_final[0x43C]&63)<3):
                    goal_final,goal_instructions=observe_scored_goal(rom[16:262160],keeper_final)
                    goal_save=out/(name+'-goal-scored.cmjsession')
                    native(keeper_saved,['--prepare-scored-goal',str(keeper_final[0x28]),str(keeper_final[0x29]),str(keeper_final[0x629])],goal_save)
                    if goal_save.read_bytes()!=keeper_checkpoint(goal_final,59):raise RuntimeError('Joined normal goal preparation mismatch')
                    result['goal_scored']=goal_final[0x28];result['goal_registration_instructions']=goal_instructions
                if keeper_final[0x612]>=2:
                    # Explicit post-display9 handler; actual rewards/status/ownership run.
                    save_final,save_phase,save_instructions=observe_keeper_save(rom[16:262160],keeper_final)
                    handoff=out/(name+'-save-handoff.cmjsession');native(keeper_saved,['--resolve-keeper-save'],handoff)
                    if handoff.read_bytes()!=post_save_checkpoint(save_final,save_phase):raise RuntimeError('Joined keeper save handoff mismatch')
                    result['save_handoff_phase']=save_phase;result['save_handler_instructions']=save_instructions
                    if save_phase==58:
                        recovery_report,recovered=trace_recovery(rom[16:262160],save_final,skip_jitter=True)
                        recovered_save=out/(name+'-punch-recovered.cmjsession');native(handoff,['--recover-keeper-punch'],recovered_save)
                        if recovered_save.read_bytes()!=post_save_checkpoint(recovered,6):raise RuntimeError('Joined punch recovery mismatch')
                        result['punch_recovery_matches']=True
                        continued=out/(name+'-punch-continued.cmjsession');native(recovered_save,['--continue-recovery'],continued)
                        result['native_recovery_continuation_phase']=continued.read_bytes()[258]
                        if continued.read_bytes()[258]==1:
                            controlled=out/(name+'-punch-control-ready.cmjsession');native(continued,['--setup-control'],controlled)
                            if controlled.read_bytes()[258]!=9:raise RuntimeError('Punch recovery did not rejoin native control-ready')
                            result['native_control_ready']=True
            if phase==54:
                until(cpu,{0x8917})
                thresholds=rom[16+26*8192+0x928:16+26*8192+0x92A]
                decided=out/(name+'-decided.cmjsession');native(prepared,['--resolve-stunned-keeper',*map(str,thresholds)],decided)
                if decided.read_bytes()!=keeper_checkpoint(bytes(m.ram[:2048]),55):raise RuntimeError('Complete stunned keeper decision mismatch')
                result['stunned_result']=m[0x612]
                if m[0x612]==0 or (m[0x612]==1 and (m[0x43C]&63)<3):
                    goal_final,goal_instructions=observe_scored_goal(rom[16:262160],bytes(m.ram[:2048]))
                    goal_save=out/(name+'-goal-scored.cmjsession')
                    native(decided,['--prepare-scored-goal',str(m[0x28]),str(m[0x29]),str(m[0x629])],goal_save)
                    if goal_save.read_bytes()!=keeper_checkpoint(goal_final,59):raise RuntimeError('Joined stunned goal preparation mismatch')
                    result['goal_scored']=goal_final[0x28];result['goal_registration_instructions']=goal_instructions
        cases.append(result)
    (out/'verification.json').write_text(json.dumps({'cases':cases,'scope':'Full numeric destination flight with original cell selection and native save/reload every step. Explicit frozen frame acknowledgements; no live NMI/task/input updates or queued arrival encounters. Free keeper cases also match keeper setup, stunned decision before presentation12, prepared normal choice/comparison before presentation9, and non-goal save/reward handlers after acknowledged display9, including punch recovery. Eligible normal/stunned cases also compare scorer reward and score registration before goal presentation1, with no celebration/kickoff claim. Free loose case additionally compares prepared post-display recovery and checks native control-ready. Display45 and native control setup are not claimed part of this original trajectory equality.'},indent=2)+'\n')
    print(f'{len(cases)} complete prepared numeric flights match at every saved cell/callback: {out}')


if __name__=='__main__':main()

#!/usr/bin/env python3
"""Compare post-goal kickoff reset, formation and control with the original CPU."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_keeper_ai_original import keeper_entry, observe as observe_keeper
from check_keeper_entry_original import observe as observe_keeper_entry
from check_kickoff_control_original import checkpoint
from check_scored_goal_original import observe_scored_goal
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def capture(ram, phase):
    data=bytearray(checkpoint(ram,phase))
    if phase in (62,63,64):data[368:370]=bytes((1,ram[0x5fc]))
    return bytes(data)


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','ready','pack','exe','private-temp'):parser.add_argument('--'+name,type=Path,required=True)
    args=parser.parse_args();rom=args.rom.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):parser.error('Temporary data must remain private')
    prg=rom[16:262160];base,sp,flags=keeper_entry(prg,args.ready.read_bytes())
    _,_,_,keeper=observe_keeper(prg,base,sp,flags,0,100,0,0,5,20,True)
    if keeper[0x612]!=0:raise RuntimeError('Expected a constructed original goal')
    goal,_=observe_scored_goal(prg,keeper);restart,_,_=observe_keeper_entry(prg,goal,0x85bf,{0x85dd})
    count=0
    with tempfile.TemporaryDirectory(prefix='goal-kickoff-',dir=args.private_temp) as tmp:
        before_file,after_file=Path(tmp)/'before.cmjsession',Path(tmp)/'after.cmjsession'
        before_raw,after_raw=Path(tmp)/'before.bin',Path(tmp)/'after.bin'
        for left in range(4):
            for right in range(4):
                for random in (0,2,5,7):
                    raw=bytearray(restart);raw[0x2c],raw[0x2e],raw[0xe2]=left,right,random
                    raw[0x634],raw[0x636]={0:(0,255),2:(1,127),5:(127,1),7:(255,0)}[random]
                    for player in range(22):raw[0x30a+12*player]=(player*3+1)&255
                    raw[0x307],raw[0x38b],raw[0x38e]=37,0,0
                    before_file.write_bytes(capture(raw,60));before_raw.write_bytes(raw)
                    memory=ObservedCartridge(prg,raw,None,set());memory.registers[6:8]=[26,27]
                    memory[0x24],memory[0x25]=26,27;memory[0x23]=memory[0x22]|7;memory.select=memory[0x23]
                    cpu=MPU(memory=memory);cpu.pc,cpu.sp,cpu.p=0x85dd,0xfd,cpu.UNUSED
                    def run_until(target):
                        for _ in range(2_000_000):
                            if cpu.pc==target:return
                            if cpu.pc<0x8000:raise RuntimeError(f'Kickoff left PRG at {cpu.pc:04x}')
                            if cpu.pc==0xcaae:
                                memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0
                                if memory[0x498]:memory[0x498]-=1
                            memory.instruction=(memory.bank_at(cpu.pc),cpu.pc);cpu.step()
                        raise RuntimeError(f'Original kickoff did not reach {target:04x}')
                    def compare(phase,steps,raw_after=None):
                        snapshot=bytes(memory.ram[:2048]) if raw_after is None else raw_after
                        after_file.write_bytes(capture(snapshot,phase));after_raw.write_bytes(snapshot)
                        result=subprocess.run([str(args.exe.resolve()),'--compare-goal-kickoff',str(before_file),str(after_file),str(args.pack.resolve()),str(steps),str(before_raw),str(after_raw)],capture_output=True,text=True)
                        if result.returncode:raise RuntimeError(f'formations={left}/{right}, random={random}, phase={phase}: {result.stderr}')
                    run_until(0xdae9);run_until(0xdaf5);compare(61,1)
                    run_until(0xdb0b);compare(62,2)
                    run_until(0xdb0e);compare(63,3)
                    cpu.pc=0xdb13;run_until(0xdb1e);compare(64,4)
                    _,ready=trace_recovery(prg,bytes(memory.ram[:2048]),prepare_control=True);compare(9,5,ready)
                    if ready[0x43d]!=raw[0x43d]:raise RuntimeError('Original kickoff changed retained defensive action')
                    count+=1
    print(f'{count} original post-goal kickoff sequences matched through control setup')


if __name__=='__main__':main()

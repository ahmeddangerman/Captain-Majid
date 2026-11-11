#!/usr/bin/env python3
"""Compare constructed human-pass defender outcomes against the original CPU."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path

from py65.devices.mpu6502 import MPU
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def entry(prg, ready):
    _, raw = trace_recovery(prg, ready, control_input=(64, 0))
    memory = ObservedCartridge(prg, raw, None, set())
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1),
                           (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb),
                           (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0xe38a, 0x50, cpu.UNUSED
    stage = held = pressed = 0; recipient_set = False
    for _ in range(3000000):
        bank = memory.bank_at(cpu.pc)
        if bank == 30 and cpu.pc == 0xd5e5:
            if stage == 0: held = pressed = 1; stage = 1
            elif stage == 1: held = pressed = 128; stage = 2
            elif stage == 3: held = pressed = 2; stage = 4
            elif stage == 4: held = pressed = 128; stage = 5
        if bank == 30 and cpu.pc == 0xd67c:
            if stage == 2: stage = 3; held = pressed = 0
            elif stage == 5: stage = 6; held = pressed = 0
        if bank == 30 and cpu.pc == 0xd80a and stage == 6: stage = 7; held = pressed = 128
        if bank == 30 and cpu.pc == 0xd84b and stage == 7: stage = 8; held = pressed = 0
        if stage == 8 and bank == 26 and cpu.pc == 0x804f and not recipient_set:
            memory[0x5fc] = 7; recipient_set = True
        if stage == 8 and bank == 31 and cpu.pc == 0xe7cc:
            memory[0x600] = 1; memory[0x601] = 12
        if recipient_set and bank == 26 and cpu.pc == 0x8845:
            return bytes(memory.ram[:2048]), (cpu.sp, cpu.p, cpu.a, cpu.x, cpu.y)
        if cpu.pc < 0x8000: raise RuntimeError(f'Original left PRG at {cpu.pc:04x}')
        if cpu.pc == 0xcaae:
            memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0
            memory[0x1c], memory[0x1e] = held, pressed
            if memory[0x498]: memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc); cpu.step()
    raise RuntimeError('Human pass entry exceeded instruction limit')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready', 'pack', 'exe', 'private-temp'):
        p.add_argument('--' + name, type=Path, required=True)
    p.add_argument('--later-defenders', action='store_true', help='Also compare terminal outcomes after an earlier defender')
    p.add_argument('--special', action='store_true', help='Compare special defender rolls instead of the ordinary sample')
    args = p.parse_args(); rom, ready = args.rom.read_bytes(), args.ready.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256 or len(ready) != 2048:
        p.error('Expected verified ROM and private 2048-byte ready RAM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private temporary data must stay outside the checkout')
    prg = rom[16:262160]; base, registers = entry(prg, ready); cases = {}; later_cases = {}
    with tempfile.TemporaryDirectory(prefix='human-pass-flight-', dir=args.private_temp) as temp:
        before_raw, before_session, after_session = (Path(temp) / name for name in ('before.bin', 'before.cmjsession', 'after.cmjsession'))
        for kind in (1, 23, 70):
            for power in ((4, 8, 21, 80) if args.special else (4, 8, 21)):
                for first, second in (((0, 255), (128, 255)) if args.special else ((1, 0), (122, 176), (255, 0))):
                    raw = bytearray(base); raw[0x390] = kind; raw[0x393] = 0
                    raw[0x396], raw[0x398], raw[0x39a] = (128 if args.special else 100), 124, 0
                    raw[0xe2], raw[0xe3], raw[0x61c], raw[0x61d] = first, second, power, 0
                    memory = ObservedCartridge(prg, bytes(raw), None, set())
                    memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
                    cpu = MPU(memory=memory); cpu.pc = 0x8845
                    cpu.sp, cpu.p, cpu.a, cpu.x, cpu.y = registers
                    for _ in range(10000):
                        bank = memory.bank_at(cpu.pc)
                        if bank == 26 and cpu.pc == 0x8870: break
                        if cpu.pc < 0x8000: raise RuntimeError(f'Original comparison left PRG at {cpu.pc:04x}')
                        memory.instruction = (bank, cpu.pc); cpu.step()
                    else: raise RuntimeError('Human comparison exceeded instruction limit')
                    code = cpu.y
                    if code > 3 or memory[0x43e] != (128 if args.special else 0): raise RuntimeError('Unexpected defender subtype or result')
                    for _ in range(1800000):
                        bank = memory.bank_at(cpu.pc)
                        if (code < 2 and bank == 26 and cpu.pc == 0x888a) or (code >= 2 and bank == 30 and cpu.pc == 0xc60f): break
                        if cpu.pc < 0x8000: raise RuntimeError(f'Original continuation left PRG at {cpu.pc:04x}')
                        if cpu.pc == 0xcaae:
                            memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0
                            memory[0x1c] = memory[0x1e] = 128
                            if memory[0x498]: memory[0x498] -= 1
                        memory.instruction = (bank, cpu.pc); cpu.step()
                    else: raise RuntimeError('Human contact continuation exceeded instruction limit')
                    before_raw.write_bytes(raw);before_session.write_bytes(export_session(raw))
                    after_session.write_bytes(export_session(bytes(memory.ram[:2048])))
                    command = [str(args.exe.resolve()), '--compare-human-flight-contact', str(before_session), str(after_session), str(args.pack.resolve()), str(before_raw), str(code)]
                    result = subprocess.run(command, capture_output=True, text=True)
                    if result.returncode:
                        raise RuntimeError(f'profile={kind}, power={power}, random={(first, second)}, result={code}: {result.stderr}')
                    cases[code] = cases.get(code, 0) + 1
        if args.later_defenders:
            for kind in ((1,70) if args.special else (1, 7, 23, 35, 70)):
                raw = bytearray(base);raw[0x600],raw[0x601],raw[0x602] = 2,12,13
                raw[0x300+12*12],raw[0x303+12*12] = 23,0
                raw[0x300+12*13],raw[0x303+12*13] = kind,0
                raw[0x306+12*12],raw[0x308+12*12],raw[0x30a+12*12] = 100,124,0
                raw[0x306+12*13],raw[0x308+12*13],raw[0x30a+12*13] = 104,124,0
                raw[0xe2],raw[0xe3],raw[0x61c],raw[0x61d] = (0,255,20,0) if args.special else (122,176,6,0)
                memory = ObservedCartridge(prg,bytes(raw),None,set());memory.select=memory[0x23];memory.registers[6:8]=[26,27]
                cpu = MPU(memory=memory);cpu.pc=0x8845;cpu.sp,cpu.p,cpu.a,cpu.x,cpu.y=registers;results=[]
                for _ in range(3000000):
                    bank = memory.bank_at(cpu.pc)
                    if bank==26 and cpu.pc==0x8870:
                        if memory[0x43e]!=(128 if args.special else 0):raise RuntimeError('Unexpected later human defender subtype')
                        results.append(cpu.y)
                    if bank==30 and cpu.pc==0xc60f:break
                    if cpu.pc<0x8000:raise RuntimeError(f'Original later human defender left PRG at {cpu.pc:04x}')
                    if cpu.pc==0xcaae:
                        memory.instruction=None;memory[0x1b]|=128;memory[0x515]=0;memory[0x1c]=memory[0x1e]=128
                        if memory[0x498]:memory[0x498]-=1
                    memory.instruction=(bank,cpu.pc);cpu.step()
                else:raise RuntimeError('Later human defender exceeded instruction limit')
                if len(results)!=2 or results[0]!=1 or results[1] not in (2,3):raise RuntimeError(f'Unexpected later human defender results {results}')
                before_raw.write_bytes(raw);before_session.write_bytes(export_session(raw));after_session.write_bytes(export_session(bytes(memory.ram[:2048])))
                command=[str(args.exe.resolve()),'--compare-human-flight-contact',str(before_session),str(after_session),str(args.pack.resolve()),str(before_raw),str(results[1])]
                result=subprocess.run(command,capture_output=True,text=True)
                if result.returncode:raise RuntimeError(f'later human defender profile={kind}, results={results}: {result.stderr}')
                later_cases[results[1]]=later_cases.get(results[1],0)+1
    print(f'{sum(cases.values())} constructed human-pass {"special" if args.special else "ordinary"} defender outcomes matched original; result counts {cases}')
    if args.later_defenders:
        print(f'{sum(later_cases.values())} later human-pass defender outcomes matched original; result counts {later_cases}')


if __name__ == '__main__':
    main()

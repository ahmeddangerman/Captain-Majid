#!/usr/bin/env python3
"""Compare a bounded ordinary opponent shot decision against the original CPU."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_kickoff_flight_original import until
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def observe(prg, raw):
    memory = ObservedCartridge(prg, raw, None, {0x43d})
    memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1), (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0xe19b, 0x50, cpu.UNUSED
    targets = {(31, 0xe1b2): 'choice', (31, 0xe1cb): 'prepared', (26, 0x80ed): 'clocked',
               (30, 0xc618): 'approach', (30, 0xdd08): 'target', (31, 0xe773): 'flight'}
    checkpoints = {}; loose = False
    for _ in range(2_000_000):
        bank = memory.bank_at(cpu.pc); name = targets.get((bank, cpu.pc))
        if name and name not in checkpoints:
            checkpoints[name] = bytes(memory.ram[:2048])
            if name == 'target': loose = bool(cpu.p & 1)
            if name == 'choice' and (memory[0x43b] != 0 or memory[0x43c] != 0): return 'other-shot', checkpoints
            if name == 'flight':
                records = bytearray()
                for _ in range(100):
                    cpu.pc = 0xe778; until(cpu, {0xe7b4, 0xe7cc})
                    records.append(0 if cpu.pc == 0xe7b4 else 1); records.extend(memory.ram[:2048])
                    if cpu.pc == 0xe7cc:
                        if not memory[0x600] and not loose and not memory[0x30a]:
                            until(cpu, {0xdd1c}); records.append(3); records.extend(memory.ram[:2048])
                            until(cpu, {0x8509, 0x8900})
                            if cpu.pc == 0x8509: records.append(4); records.extend(memory.ram[:2048])
                        checkpoints['flight-records'] = records
                        return 'flight', checkpoints
                    until(cpu, {0xe773}); records.append(2); records.extend(memory.ram[:2048])
                raise RuntimeError('Original opponent shot flight did not arrive')
        if cpu.pc < 0x8000: raise RuntimeError(f'Original shot left PRG at {cpu.pc:04x}')
        if cpu.pc == 0xcaae:
            memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = 0
            if memory[0x498]: memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc); cpu.step()
    raise RuntimeError('Original opponent shot instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'velocity', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--original-only', action='store_true')
    args = parser.parse_args(); rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    prg = rom[16:262160]; counts = Counter()
    with tempfile.TemporaryDirectory(prefix='unopposed-ai-shot-', dir=args.private_temp) as temporary:
        temp = Path(temporary); source = temp / 'source.cmjsession'; records = temp / 'records.bin'
        names = ('choice', 'prepared', 'clocked', 'approach', 'target', 'flight')
        for fixture in range(4):
            ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes()); ready[0x614] = ready[0x617] = 0
            report, decision = trace_recovery(prg, ready, control_input=(0, 0))
            if report['boundary'] != 'ai_cell_decision': raise RuntimeError('Expected original cell decision')
            for x in (81, 85, 95):
                for elapsed in (0, 1, 2):
                    for retained in (0, 3, 6):
                        base = bytearray(decision); owner = base[0x441]; cell = ((x - 48) // 8) * 12 + 7
                        base[0x635], base[0x637], base[0x5fe], base[0x617], base[0x5ff], base[0x43d] = x, 137, cell, cell, elapsed, retained
                        base[0x306 + 12 * owner], base[0x308 + 12 * owner] = x, 137
                        for seed in range(1, 500):
                            value = seed; value ^= (value << 13) & 0xffffffff; value ^= value >> 17; value ^= (value << 5) & 0xffffffff
                            original = bytearray(base); original[0xe2], original[0xe3] = value & 255, (value >> 8) & 255
                            if original_candidate(prg, original)[2] != 1: continue
                            outcome, states = observe(prg, original); counts[outcome] += 1
                            if outcome == 'flight':
                                events = states['flight-records'][::2049]
                                counts['flight-boundaries'] += len(events)
                                counts['normal-keeper-entry'] += int(bool(events) and events[-1] == 4)
                            if outcome == 'flight' and not args.original_only:
                                source.write_bytes(export_session(base, 12)); records.write_bytes(b''.join(states[name] for name in names)); (temp / 'flight-records.bin').write_bytes(states['flight-records'])
                                for name in names: (temp / (name + '.cmjsession')).write_bytes(export_session(states[name]))
                                result = subprocess.run([str(args.exe.resolve()), '--compare-unopposed-ai-shot', str(source), str(records), str(args.pack.resolve()), str(args.velocity.resolve()), str(seed), temporary], capture_output=True, text=True)
                                if result.returncode: raise RuntimeError(f'fixture={fixture}, x={x}, elapsed={elapsed}, retained={retained}, exit={result.returncode}: {result.stderr}')
                            break
                        else: counts['no-candidate'] += 1
    print(f'Constructed original unopposed opponent shot survey: {dict(sorted(counts.items()))}')


if __name__ == '__main__': main()

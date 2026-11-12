#!/usr/bin/env python3
"""Trace an intervened single field defender in an original opponent shot."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from check_unopposed_ai_shot_original import observe
from export_phase_session import export_session
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def defend(prg, raw, cell, defenders, stamina, randoms):
    memory = ObservedCartridge(prg, raw, None, {0x43d, 0x634, 0x636}); memory.ram[:0x1a] = bytes(0x1a)
    for address, value in ((0, 5), (1, 255), (2, 0x90), (3, 26), (4, 27), (5, 1), (6, 0x50), (7, 26), (8, 27), (0x191, 0x85), (0x192, 0xeb), (0x24, 26), (0x25, 27)):
        memory[address] = value
    memory[0x23] = memory[0x22] | 7; memory.select = memory[0x23]; memory.registers[6:8] = [26, 27]
    cpu = MPU(memory=memory); cpu.pc, cpu.sp, cpu.p = 0xe19b, 0x50, cpu.UNUSED
    checkpoints = {}; injected = False
    for _ in range(2_000_000):
        bank = memory.bank_at(cpu.pc)
        if (bank, cpu.pc) == (31, 0xe773) and not injected:
            for player in range(1, 11): memory[0x30a + 12 * player] = 1
            for defender in defenders:
                record = 0x300 + 12 * defender
                memory[record + 10] = 0; memory[record + 1] = stamina & 255; memory[record + 2] = stamina >> 8
                memory[record + 6] = 48 + (cell // 12) * 8 + 4; memory[record + 8] = 80 + (cell % 12) * 8 + 4
            memory[0xe2], memory[0xe3] = randoms; injected = True
        if (bank, cpu.pc) == (31, 0xe7cc) and 'arrival' not in checkpoints:
            if memory[0x600] != len(defenders) or tuple(memory.ram[0x601:0x601 + len(defenders)]) != defenders: return 'selection-other', checkpoints
            checkpoints['arrival'] = bytes(memory.ram[:2048])
        if (bank, cpu.pc) == (26, 0x886d): checkpoints['prepared'] = bytes(memory.ram[:2048])
        if (bank, cpu.pc) == (26, 0x8870):
            checkpoints['compared'] = bytes(memory.ram[:2048])
            if memory[0x612] >= 2: return 'terminal-' + str(memory[0x612]), checkpoints
        if bank == 30 and cpu.pc == 0xdd1c and 'compared' in checkpoints:
            checkpoints['keeper-entry'] = bytes(memory.ram[:2048]); return 'survived-' + str(memory[0x612]), checkpoints
        if cpu.pc < 0x8000: raise RuntimeError(f'Original shot defender left PRG at {cpu.pc:04x}')
        if cpu.pc == 0xcaae:
            memory.instruction = None; memory[0x1b] |= 128; memory[0x515] = 0; memory[0x1c] = memory[0x1e] = 0
            if memory[0x498]: memory[0x498] -= 1
        memory.instruction = (bank, cpu.pc); cpu.step()
    raise RuntimeError('Original shot defender instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'exe', 'private-temp'): parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--original-only', action='store_true')
    args = parser.parse_args(); rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256: parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]): parser.error('Temporary data must remain private')
    prg = rom[16:262160]; counts = Counter()
    with tempfile.TemporaryDirectory(prefix='opponent-shot-defender-', dir=args.private_temp) as temporary:
        temp = Path(temporary); source = temp / 'source.cmjsession'; expected = temp / 'expected.cmjsession'; records = temp / 'records.bin'
        for fixture in range(4):
            ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes()); ready[0x614] = ready[0x617] = 0
            report, decision = trace_recovery(prg, ready, control_input=(0, 0))
            if report['boundary'] != 'ai_cell_decision': raise RuntimeError('Expected original cell decision')
            base = bytearray(decision); owner = base[0x441]; x = 85; cell = ((x - 48) // 8) * 12 + 7
            base[0x635], base[0x637], base[0x5fe], base[0x617], base[0x5ff], base[0x43d] = x, 137, cell, cell, 0, 6
            base[0x306 + 12 * owner], base[0x308 + 12 * owner] = x, 137
            for seed in range(1, 500):
                value = seed; value ^= (value << 13) & 0xffffffff; value ^= value >> 17; value ^= (value << 5) & 0xffffffff
                base[0xe2], base[0xe3] = value & 255, (value >> 8) & 255
                if original_candidate(prg, base)[2] == 1: break
            else: raise RuntimeError('No original ordinary shot candidate')
            outcome, shot = observe(prg, base)
            if outcome != 'flight' or shot['flight-records'][0] != 0: raise RuntimeError('Shot needs an initial cell crossing')
            cell = shot['flight-records'][1 + 0x5fe]
            for defenders in ((1,), (3,), (5,), (7,), (1, 3), (3, 5)):
                for stamina in (20, 100, 256):
                    for randoms in ((1, 0), (32, 1), (128, 64), (255, 128)):
                        outcome, states = defend(prg, base, cell, defenders, stamina, randoms); counts[f'{len(defenders)}-{outcome}'] += 1
                        if (outcome.startswith('survived-') or outcome.startswith('terminal-')) and not args.original_only:
                            terminal = outcome.startswith('terminal-'); last = states['compared'] if terminal else states['keeper-entry']
                            source.write_bytes(export_session(states['arrival'])); expected.write_bytes(export_session(last))
                            records.write_bytes(b''.join((states['arrival'], states['prepared'], states['compared'], last)))
                            mode = '--compare-opponent-shot-defender-terminal' if terminal else '--compare-opponent-shot-defender'
                            run = subprocess.run([str(args.exe.resolve()), mode, str(source), str(expected), str(records), str(args.pack.resolve())], capture_output=True, text=True)
                            if run.returncode: raise RuntimeError(f'fixture={fixture} defenders={defenders} stamina={stamina} random={randoms}: {run.stderr}')
    print(f'Constructed original opponent shot defender survey: {dict(sorted(counts.items()))}')


if __name__ == '__main__': main()

#!/usr/bin/env python3
"""Compare bounded opponent cell-change fallback choices with original CPU execution."""
import argparse
import hashlib
import subprocess
import tempfile
from collections import Counter
from itertools import product
from pathlib import Path

from py65.devices.mpu6502 import MPU
from check_ai_field_candidate_original import original_candidate
from export_phase_session import export_session
from export_shot_content import no_variant_zero_shot_option
from reference_cartridge import ReferenceCartridge
from trace_ball_recovery_phase import trace_recovery
from trace_field_script import ObservedCartridge
from verify_action_strength import PAYLOAD_SHA256


def has_shot_gates(magic):
    return magic in (b'CMJSHOTS', b'CMJSHOTT', b'CMJSHOTU', b'CMJSHOTV', b'CMJSHOTW', b'CMJSHOTX', b'CMJSHOTY', b'CMJSHOTZ', b'CMJSHOTa')


def original_decision(prg, raw):
    memory = ObservedCartridge(prg, raw, None, {0x43d, 0x634, 0x636})
    memory.registers[6:8] = [26, 27]
    memory[0x24], memory[0x25] = 26, 27
    memory[0x23] = memory[0x22] | 7;memory.select = memory[0x23]
    cpu = MPU(memory=memory);cpu.pc, cpu.sp, cpu.p = 0xe19b, 0x50, cpu.UNUSED
    for _ in range(20000):
        if cpu.pc in (0xe1e7, 0xe1b9):
            return cpu.pc, bytes(memory.ram[:2048]), memory.writes
        if cpu.pc < 0x8000 or cpu.pc in (0xcaae, 0xcb0f):
            raise RuntimeError(f'Unexpected original AI scheduler/return at {cpu.pc:04x}')
        memory.instruction = (memory.bank_at(cpu.pc), cpu.pc);cpu.step()
    raise RuntimeError('Original cell AI exceeded instruction limit')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ready-dir', 'pack', 'exe', 'private-temp'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--legacy-pack',action='store_true',help='Expect Q packs to reject candidate3 when a variant-zero option gate is unavailable')
    args = parser.parse_args();rom = args.rom.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('Expected verified Japanese ROM')
    if args.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
        parser.error('Private temporary data must stay outside checkout')
    prg = rom[16:262160];counts = Counter();stats=ReferenceCartridge(prg);stats.registers[6:8]=[28,29];shot_gates=has_shot_gates(args.pack.read_bytes()[:8])
    with tempfile.TemporaryDirectory(prefix='ai-cell-', dir=args.private_temp) as temporary:
        source, expected, metadata = (Path(temporary) / name for name in ('source.cmjsession', 'expected.cmjsession', 'metadata.bin'))
        for fixture in range(4):
            ready = bytearray((args.ready_dir / f'{fixture}-ready.bin').read_bytes())
            ready[0x614] = 0;ready[0x617] = 0
            for action in range(7):
                seed=action+1;raw=bytearray(ready);raw[0x43d]=action;raw[0x634]=(action*29)&255;raw[0x636]=(action*47)&255
                value=seed;value^=(value<<13)&0xffffffff;value^=value>>17;value^=(value<<5)&0xffffffff
                original=bytearray(raw);original[0xe2],original[0xe3]=value&255,(value>>8)&255
                report, polled=trace_recovery(prg,original,control_input=(0,0))
                if report['boundary']!='ai_cell_decision' or any(polled[a]!=raw[a] for a in (0x43d,0x634,0x636)):
                    raise RuntimeError('Original control-to-AI poll changed known metadata or boundary')
                source.write_bytes(export_session(raw,9));expected.write_bytes(export_session(polled,12));metadata.write_bytes(polled)
                command=[str(args.exe.resolve()),'--compare-ai-cell-poll',str(source),str(expected),str(metadata),str(args.pack.resolve()),str(seed)]
                result=subprocess.run(command,capture_output=True,text=True)
                if result.returncode:raise RuntimeError(f'fixture={fixture}, poll action={action}: {result.stderr}')
                counts['control-to-ai']+=1
            report, decision = trace_recovery(prg, ready, control_input=(0, 0))
            if report['boundary'] != 'ai_cell_decision':raise RuntimeError('Expected original opponent cell-change boundary')
            if any(decision[address] != ready[address] for address in (0x43d, 0x634, 0x636)):
                raise RuntimeError('Original control-to-AI poll changed known metadata')
            for x,kind,variant,flags in product((80,112,160,200),(23,26,40),(0,1),(0,128)):
                for seed in range(1, 9):
                    raw = bytearray(decision);owner = raw[0x441];cell = ((x - 48) // 8) * 12 + (124 - 80) // 8
                    raw[0x635], raw[0x637], raw[0x5fe], raw[0x617] = x, 124, cell, cell
                    raw[0x306 + 12 * owner], raw[0x308 + 12 * owner] = x, 124
                    raw[0x300+12*owner],raw[0x44e]=kind,variant
                    raw[0x301+12*owner],raw[0x302+12*owner]=flags,35 if flags else 0
                    raw[0x43d], raw[0x634], raw[0x636] = seed % 7, (seed * 29) & 255, (seed * 47) & 255
                    value = seed;value ^= (value << 13) & 0xffffffff;value ^= value >> 17;value ^= (value << 5) & 0xffffffff
                    intervened = bytearray(raw);intervened[0xe2], intervened[0xe3] = value & 255, (value >> 8) & 255
                    candidate = original_candidate(prg, intervened)[2]
                    boundary, after, writes = original_decision(prg, intervened)
                    group=stats[0x8e1b+2*kind] | (stats[0x8e1c+2*kind]<<8)
                    empty_action2=stats[group+4] == 0 and stats[group+5] == 0
                    empty_pass=stats[group+2] == 0 and stats[group+3] == 0
                    fallback = boundary == 0xe1e7 and after[0x43b] == 2 and (candidate == 2 or (candidate == 3 and (variant or (empty_action2 and not args.legacy_pack))))
                    passing = boundary == 0xe1b9 and after[0x43b] == 1 and candidate == 2 and variant == 0 and empty_pass
                    safe_shot = no_variant_zero_shot_option(stats,stats[group] | (stats[group+1]<<8))
                    shooting = shot_gates and safe_shot and boundary == 0xe1b9 and after[0x43b] == 0 and after[0x43c] == 0 and candidate == 1 and variant == 0
                    if (fallback or passing or shooting) and (writes or any(after[address] != raw[address] for address in (0x43d, 0x634, 0x636))):
                        raise RuntimeError('Original fallback changed known metadata')
                    source.write_bytes(export_session(raw, 12));expected.write_bytes(export_session(after, 0 if passing or shooting else 10));metadata.write_bytes(after)
                    command = [str(args.exe.resolve()), '--compare-ai-cell' if fallback else '--compare-ai-cell-pass' if passing else '--compare-ai-cell-shot' if shooting else '--compare-ai-cell-unsupported',
                               str(source), str(expected), str(metadata), str(args.pack.resolve()), str(seed)]
                    result = subprocess.run(command, capture_output=True, text=True)
                    if result.returncode:raise RuntimeError(f'fixture={fixture}, x={x}, type={kind}, variant={variant}, flags={flags}, seed={seed}: {result.stderr}')
                    counts['fallback' if fallback else 'pass' if passing else 'shot' if shooting else 'unsupported'] += 1
    if not counts['fallback']:raise RuntimeError('Survey did not exercise a supported fallback')
    print(f'Constructed original opponent cell decisions: {dict(sorted(counts.items()))}')


if __name__ == '__main__':
    main()

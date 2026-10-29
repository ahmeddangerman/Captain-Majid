#!/usr/bin/env python3
"""Observe kickoff-to-control setup, using a prepared post-reset ROM boundary."""
import argparse
import hashlib
import json
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from trace_ball_recovery_phase import trace_recovery
from verify_action_strength import PAYLOAD_SHA256


def run_until(m, start, stop):
    cpu = MPU(memory=m)
    cpu.pc, cpu.sp, cpu.p = start, 0xfd, cpu.UNUSED
    for step in range(20000):
        if cpu.pc == stop:
            return step
        cpu.step()
    raise RuntimeError(f'Instruction bound between {start:04x} and {stop:04x}')


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for name in ('rom', 'ram', 'output_dir'):
        p.add_argument(name, type=Path)
    a = p.parse_args()
    rom, base = a.rom.read_bytes(), a.ram.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    if len(base) != 2048 or base[0x22] & 64:
        p.error('Expected compatible original internal RAM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private outputs must stay outside checkout')
    out.mkdir(parents=True, exist_ok=False)
    prg = rom[16:262160]
    results = []
    for side in (0, 11):
        for left in range(4):
            for right in range(4):
                ram = bytearray(base)
                for addr, value in ((0x5fb,side), (0x43b,0), (0x5ff,0), (0x62d,0),
                                    (0x600,1), (0x601,11), (0x442,11), (0x389,0),
                                    (0x38b,0), (0x38e,0), (0x28,3), (0x29,2),
                                    (0x629,4), (0x2c,left), (0x2e,right), (0xe2,2)):
                    ram[addr]=value
                ram[0x602:0x606] = bytes(4)
                m = ObservedCartridge(prg, ram, None, set())
                m.registers[6:8] = [26, 27]
                formation_steps = run_until(m, 0xdae9, 0xdb0e)
                # Display36 is explicitly acknowledged without simulating it.
                handoff_steps = run_until(m, 0xdb13, 0xdb1e)
                before = bytes(m.ram[:2048])
                report, after = trace_recovery(prg, before, prepare_control=True)
                if report['boundary'] != 'player_control_ready':
                    raise RuntimeError('Unexpected original control boundary')
                name = f's{side}-l{left}-r{right}'
                (out / f'{name}-before.bin').write_bytes(before)
                (out / f'{name}-after.bin').write_bytes(after)
                results.append({'side':side, 'left_formation':left, 'right_formation':right,
                                'owner':before[0x441], 'formation_and_ball_instructions':formation_steps,
                                'handoff_instructions':handoff_steps,
                                'control_instructions':report['original_instructions'],
                                'keeper_y_before':[before[0x308],before[0x38c]],
                                'keeper_y_after':[after[0x308],after[0x38c]],
                                'ball_before':[before[0x635],before[0x637]],
                                'ball_after':[after[0x635],after[0x637]],
                                'controlled_player':after[0x5fd],
                                'presentation_requests':report['presentation_requests']})
    report = {'cases':results, 'scope':'Prepared state after earlier reset and side selection. Original formation/receiver/ball and post-display handoff execute, with display36 assumed acknowledged. Original E0DF..E145 runs using disclosed frame acknowledgements; no NMI, PPU, audio, or real presentation effects. Research only: no native comparison.'}
    (out / 'observation.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'{len(results)} original kickoff-to-control observations: {out}')


if __name__ == '__main__':
    main()

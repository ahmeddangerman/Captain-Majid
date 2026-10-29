#!/usr/bin/env python3
"""Compare native nonterminal selected dispatch with the approved private ROM.

Inputs must be an explicit side0 ordinary selected-shot checkpoint at83F5,
a matching private native pack and the approved ROM. Output contains private RAM.
This constructs four CPU-entry cases, not full matches or live frame emulation.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from trace_field_loop import trace_loop
from export_phase_session import export_session
from verify_action_strength import PAYLOAD_SHA256


def checkpoint(ram, code, phase):
    data = bytearray(export_session(bytes(ram), 20))
    data[10], data[258] = ord('I'), phase
    count = ram[0x600]
    progress = [count, code, ram[0x600+count] if count else 0,
                ram[0x43D] if count else 0, ram[0x43E] if count else 0]
    data += bytes([1, *progress, 0, 0, 0, 0, 0, 0, 0, 0, 0])
    return bytes(data)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('rom', type=Path); p.add_argument('pack', type=Path)
    p.add_argument('ram', type=Path); p.add_argument('output_dir', type=Path)
    p.add_argument('--binary', type=Path, default=Path('build/captain-phase-session'))
    a = p.parse_args()
    rom, base = a.rom.read_bytes(), a.ram.read_bytes()
    if len(rom) != 393232 or rom[:4] != b'NES\x1a' or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    if len(base) != 2048 or base[0x5FB] != 0 or base[0x43B] != 0 or not 1 <= base[0x600] <= 5:
        p.error('Expected side0 ordinary selected-shot RAM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]):
        p.error('Private output must be outside the Git checkout')
    out.mkdir(parents=True, exist_ok=False)
    binary, pack = str(a.binary.resolve()), str(a.pack.resolve())
    results = []
    for name, code, count, elapsed in [('pass0', 0, base[0x600], 3),
            ('pass1', 1, base[0x600], 3), ('unopposed', 0, 0, 1),
            ('moving', 0, base[0x600], 7)]:
        ram = bytearray(base)
        ram[0x612], ram[0x600], ram[0x5FF] = code, count, elapsed
        if not count:
            ram[0x601:0x606] = bytes(5)
        entry = out/(name+'-entry.cmjsession')
        entry.write_bytes(checkpoint(ram, code, 27))
        (out/(name+'-entry.bin')).write_bytes(ram)
        final = bytearray()
        report = trace_loop(rom[16:262160], bytes(ram), dispatch_only=True, final_ram=final)
        (out/(name+'.json')).write_text(json.dumps(report, indent=2)+'\n')
        (out/(name+'-final.bin')).write_bytes(final)
        dest = out/(name+'-native.cmjsession')
        subprocess.run([binary, pack, str(entry), '--resolve-shot-result', '--capture', str(dest)],
                       capture_output=True, text=True, check=True, timeout=30)
        actual, expected = dest.read_bytes(), checkpoint(final, code, 43)
        different = [i for i in range(min(len(actual), len(expected))) if actual[i] != expected[i]]
        if actual != expected:
            raise RuntimeError(f'{name}: snapshot mismatch offsets{different}, lengths{len(actual)}/{len(expected)}')
        replay = subprocess.run([binary, pack, str(dest), '--resolve-shot-result'],
                                capture_output=True, text=True, timeout=30)
        if replay.returncode == 0:
            raise RuntimeError(f'{name}: repeated dispatch accepted')
        results.append({'case': name, 'matches': True, 'original_represented_bytes': 324,
                        'exit': '8408 before script8', 'replay_rejected': True})
    (out/'verification.json').write_text(json.dumps({'cases': results,
        'scope': 'Constructed CPU entries; no skipped calls or frames before8408. Native metadata is checked independently, not original RAM.'}, indent=2)+'\n')
    print(f'{len(results)} original/native comparisons passed. Private evidence: {out}')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Compare prepared8410 attack timing against original CPU observations.

Private inputs: approved ROM, matching native pack and phase43 entry RAM after
script8. Outputs contain private RAM and must be outside the source checkout.
"""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from export_phase_session import export_session
from trace_field_loop import trace_loop
from verify_action_strength import PAYLOAD_SHA256


def checkpoint(ram, phase):
    b = bytearray(export_session(bytes(ram), 20)); b[10], b[258] = ord('Z'), phase
    count = ram[0x600]
    progress = [count, ram[0x612], ram[0x600+count], ram[0x43D], ram[0x43E]] if count else [0]*5
    b += bytes([1, *progress, 0, 0, 0, 0, 0, 0, 0, 0, 0])
    b += bytes(31)
    return bytes(b)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    for arg in ('rom', 'pack', 'ram', 'output_dir'): p.add_argument(arg, type=Path)
    p.add_argument('--binary', type=Path, default=Path('build/captain-phase-session'))
    a = p.parse_args(); rom, base = a.rom.read_bytes(), a.ram.read_bytes()
    if len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        p.error('Expected approved Japanese ROM')
    if len(base) != 2048 or base[0x5FB] != 0 or base[0x43B] != 0 or not 1 <= base[0x600] <= 5:
        p.error('Expected side0 ordinary selected shot RAM')
    out = a.output_dir.resolve()
    if out.is_relative_to(Path(__file__).resolve().parents[1]): p.error('Private output must be outside Git checkout')
    out.mkdir(parents=True, exist_ok=False)
    binary, pack = str(a.binary.resolve()), str(a.pack.resolve())
    cases = [('ordinary',449,0,0,0,0,False), ('action2',449,0,1,2,0,False),
             ('added-time',1,5,0,0,0,False), ('period-end',0,0,0,0,0,False),
             ('elapsed-wrap',449,0,0,0,255,False), ('unopposed',449,0,0,2,0,True),
             ('unopposed-added',1,5,0,2,0,True)]
    results = []
    for name, remaining, extra, code, action, elapsed, unopposed in cases:
        ram = bytearray(base); ram[0x617] = 0; ram[0x612] = code; ram[0x43D] = action
        ram[0x5FF] = elapsed; ram[0x5F7:0x5F9] = remaining.to_bytes(2,'little'); ram[0x5F9] = extra
        if unopposed: ram[0x600:0x606] = bytes(6)
        entry, dest = out/(name+'-entry.cmjsession'), out/(name+'-native.cmjsession')
        entry.write_bytes(checkpoint(ram,43)); (out/(name+'-entry.bin')).write_bytes(ram)
        # The observer's prepared selected-entry guard is based on active prefix.
        # Unopposed timing executes the same8410 CPU entry; no defender scan occurs.
        final = bytearray(); report = trace_loop(rom[16:262160], bytes(ram), attack_tail=True, final_ram=final)
        (out/(name+'.json')).write_text(json.dumps(report,indent=2)+'\n'); (out/(name+'-final.bin')).write_bytes(final)
        phase = {'shot_approach_entry':45,'attack_added_time':44,'period_end':4}[report['boundary']]
        operation = ['--continue-unopposed-attack',str(action)] if unopposed else ['--continue-prepared-attack']
        subprocess.run([binary,pack,str(entry),*operation,'--capture',str(dest)],capture_output=True,check=True,timeout=30)
        actual, expected = dest.read_bytes(), checkpoint(final,phase)
        if actual != expected:
            different=[i for i,(x,y) in enumerate(zip(actual,expected)) if x!=y]
            raise RuntimeError(f'{name}: mismatch offsets{different}; lengths{len(actual)}/{len(expected)}')
        if subprocess.run([binary,pack,str(dest),*operation],capture_output=True,timeout=30).returncode==0:
            raise RuntimeError('Repeated attack clock accepted')
        if phase==44:
            resumed=out/(name+'-resumed.cmjsession')
            subprocess.run([binary,pack,str(dest),'--resume-attack-clock','--capture',str(resumed)],capture_output=True,check=True,timeout=30)
            wanted=bytearray(actual);wanted[258]=45
            if resumed.read_bytes()!=wanted: raise RuntimeError('Resume reapplied numeric state')
        results.append({'case':name,'match':True,'phase':phase,'host_frames':len(report['host_frames'])})
    (out/'verification.json').write_text(json.dumps({'cases':results,
        'scope':'324prior represented bytes plus native metadata; prepared after script8; disclosed scheduler/frame acknowledgements, no live rendering/audio.'},indent=2)+'\n')
    print(f'{len(results)} prepared attack clock comparisons passed: {out}')


if __name__ == '__main__': main()

#!/usr/bin/env python3
"""Compare composed profile selection/preparation to original ROM execution.

Fixtures deliberately construct records/team bytes, including byte-domain alternate
entries; they do not establish which alternate types occur in real matches.
"""
from __future__ import annotations
import argparse
import hashlib
import random
import subprocess
import tempfile
from pathlib import Path
from verify_field_defender_preparation import cpu_for, original, word
from verify_action_strength import PAYLOAD_SHA256


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--cxx', default='c++')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if (rom[:4] != b'NES\x1a' or len(rom) != 393232 or
            hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256):
        parser.error('ROM payload does not match verified Japanese reference')
    cpu = cpu_for(rom[16:16 + 262144])
    ram = cpu.memory
    ordinary = [ram[0x95D6 + i * 4] for i in range(35)]
    alternate = [ram[0x9662 + i * 12] for i in range(221)]
    skills = [ram[0x9FCE + i * 24:0x9FCE + i * 24 + 31] for i in range(256)]
    slots = ram[0x818E:0x8199]
    team = [0, 0, 1, 34, 35, 36]  # Synthetic team selection, not original team data.
    ram[0x38:0x3A] = [0,5]
    ram[0x500:0x500 + len(team)] = team
    selectors = ram[0x824C:0x825B]
    pointers = [word(ram, 0x9554 + 2 * i) for i in range(5)] + [0x959E]
    moves = [ram[a:b] for a,b in zip(pointers,pointers[1:])]
    counts = [len(row)//4 for row in moves]
    options = [(a,s,t) for a in range(5) for s in range(3)
               if selectors[3*a+s] != 255 for t in range(counts[a])]
    content = [*ordinary, len(alternate), *alternate, len(skills)]
    for row in skills: content.extend(row)
    content.extend([*slots,len(team),*team,*selectors])
    for n,row in zip(counts,moves): content.extend([n,*row])
    content.extend(ram[0x9E4E:0x9F0E])
    field_ids = [*range(1,11),*range(12,22)]
    cases = []
    # Every ordinary/alternate type and all level carry boundaries; exercise
    # both direct and override paths and every action/situation/move combination.
    for typ in range(1,256):
        for level in (0,127,128,255):
            for a,s,t in options:
                pid = field_ids[len(cases)%20]
                cases.append((pid,typ,0,0,level,a,s,t))
                if typ >= 35:
                    cases.append((pid,35,128,typ,level,a,s,t|192))
    for pid in range(12,22):
        for flags in (0,128):
            for level in (0,127,128,255):
                for a,s,t in options:
                    cases.append((pid,0,flags,37,level,a,s,t))
    rng = random.Random(0x803A)
    for _ in range(4096):
        a,s,t = rng.choice(options)
        cases.append((rng.choice(field_ids),rng.randrange(1,256),rng.randrange(256),
                      rng.randrange(35,256),rng.randrange(256),a,s,t|rng.choice((0,64,128,192))))
    expected = []
    for pid,typ,flags,alt,level,a,s,t in cases:
        expected.append(original(cpu,(pid,typ,level,a,s,t),flags=flags,alternate_type=alt))
    root = Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix='captain-profiles-') as temp:
        binary = Path(temp)/'compare'
        sources = ['engine/tests/profile_preparation_driver.cpp',
                   'engine/src/field_player_profile.cpp','engine/src/field_defender_preparation.cpp',
                   'engine/src/shot_power.cpp','engine/src/defender_contact.cpp',
                   'engine/src/action_strength.cpp']
        subprocess.run([args.cxx,'-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        '-I'+str(root/'engine/include'),*[str(root/p) for p in sources],
                        '-o',str(binary)],check=True,timeout=60)
        data = ' '.join(map(str,content))+'\n'+str(len(cases))+'\n'
        data += '\n'.join(' '.join(map(str,c)) for c in cases)+'\n'
        run = subprocess.run([str(binary)],input=data,text=True,capture_output=True,
                             check=True,timeout=60)
    actual = [tuple(map(int,line.split())) for line in run.stdout.splitlines()]
    if len(actual) != len(expected): raise AssertionError('Native output count mismatch')
    for i,(want,got) in enumerate(zip(expected,actual)):
        if want != got: raise AssertionError(f'Case {i} {cases[i]}: ROM={want}, native={got}')
    print(f'Profile selection plus defender preparation matched {len(cases)} original-ROM fixtures.')


if __name__ == '__main__':
    main()

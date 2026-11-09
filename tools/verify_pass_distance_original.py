#!/usr/bin/env python3
"""Compare bounded pass-distance calculations with original CPU routine $DF8B."""
import argparse
import hashlib
import itertools
import random
import subprocess
import tempfile
from pathlib import Path

from reference_cartridge import ReferenceCartridge
from reference_identity import PAYLOAD_SHA256


def iter_cases(exhaustive):
    if exhaustive:
        yield from itertools.product(range(48,208), range(80,176), range(240))
        return
    for x in (48,49,51,52,55,56,59,60,127,128,199,200,203,204,206,207):
        for y in (80,81,83,84,87,88,91,92,127,128,167,168,171,172,174,175):
            for cell in (0,1,11,12,119,120,227,228,238,239):
                yield x,y,cell
    rng = random.Random(0xdf8b)
    for _ in range(20000):
        yield rng.randrange(48,208),rng.randrange(80,176),rng.randrange(240)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('rom', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--exhaustive', action='store_true', help='check every valid ball coordinate and target cell')
    args = parser.parse_args()
    rom = args.rom.read_bytes()
    if rom[:4] != b'NES\x1a' or len(rom) != 393232 or hashlib.sha256(rom[16:]).hexdigest() != PAYLOAD_SHA256:
        parser.error('ROM payload does not match the verified Japanese reference')
    from py65.devices.mpu6502 import MPU
    memory = ReferenceCartridge(rom[16:262160])
    table = memory[0xdfbd:0xdfd1]
    cpu = MPU(memory=memory)
    root = Path(__file__).resolve().parents[1]
    cases = iter(iter_cases(args.exhaustive))
    total = 0
    with tempfile.TemporaryDirectory(prefix='captain-pass-distance-') as temp:
        binary = Path(temp) / 'compare'
        subprocess.run([args.cxx,'-std=c++17','-O2','-Wall','-Wextra','-Wpedantic',
                        '-I'+str(root/'engine/include'),str(root/'engine/tests/pass_distance_trace_driver.cpp'),
                        str(root/'engine/src/pass_distance.cpp'),'-o',str(binary)],check=True,timeout=60)
        while True:
            batch = list(itertools.islice(cases, 32768))
            if not batch:
                break
            expected = []
            for x,y,cell in batch:
                memory[0x635],memory[0x637],memory[0x638],memory[0x62b] = x,y,cell,255
                memory[0x1fe],memory[0x1ff] = 255,1
                cpu.pc,cpu.sp,cpu.p = 0xdf8b,0xfd,cpu.UNUSED
                cpu.a=cpu.x=cpu.y=0
                for _ in range(300):
                    if cpu.pc == 0x200:
                        break
                    cpu.step()
                else:
                    raise RuntimeError(f'Original pass distance did not return for case {total + len(expected)}')
                expected.append(memory[0x62b])
            data = ' '.join(map(str,table))+'\n'+str(len(batch))+'\n'
            data += '\n'.join(' '.join(map(str,case)) for case in batch)+'\n'
            result = subprocess.run([str(binary)],input=data,text=True,capture_output=True,check=True,timeout=60)
            actual = [int(line) for line in result.stdout.splitlines()]
            if len(actual) != len(expected):
                raise RuntimeError(f'Native output count {len(actual)} != {len(expected)} at case {total}')
            for index,(want,got) in enumerate(zip(expected,actual)):
                if want != got:
                    raise AssertionError(f'Case {total + index}, ball and target {batch[index]}: original {want}, native {got}')
            total += len(batch)
            if args.exhaustive and total % 262144 == 0:
                print(f'Compared {total} original/native pass-distance inputs', flush=True)
    print(f'Pass distance matched {total} controlled original-CPU cases; valid field coordinates and target cells only.')


if __name__ == '__main__':
    main()

#!/usr/bin/env python3
"""Check U pass-defense tables, T compatibility, and malformed-pack rejection."""
import argparse
import hashlib
import subprocess
import tempfile
from pathlib import Path
from reference_cartridge import ReferenceCartridge
from verify_action_strength import PAYLOAD_SHA256


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('rom','old-pack','new-pack','exe','private-temp'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();rom=a.rom.read_bytes();old=a.old_pack.read_bytes();new=a.new_pack.read_bytes()
    if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected verified Japanese ROM')
    if a.private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):p.error('Temporary files must stay private')
    m=ReferenceCartridge(rom[16:262160]);m.registers[6:8]=[28,29]
    profiles=bytes(m[0x9662+12*i+7] for i in range(83));rows=max(profiles)+1
    tail=bytes(m[0x8bbe:0x8cbe])+len(profiles).to_bytes(2,'little')+profiles+rows.to_bytes(2,'little')+bytes(m[0xb8ae:0xb8ae+48*rows])
    if old[:8]!=b'CMJSHOTT' or new[:8]!=b'CMJSHOTU' or old[8:]!=new[8:-len(tail)] or new[-len(tail):]!=tail:raise RuntimeError('U pack does not preserve T and match original tables')
    def launch(pack):return subprocess.run([str(a.exe.resolve()),'--field',str(pack)],input='',capture_output=True,text=True).returncode
    if launch(a.old_pack.resolve()) or launch(a.new_pack.resolve()):raise RuntimeError('Valid T/U pack rejected')
    with tempfile.TemporaryDirectory(prefix='pass-defense-pack-',dir=a.private_temp) as temp:
        for name,data in (('truncated',new[:-1]),('trailing',new+b'\0'),('bad-row',new[:-len(tail)]+tail[:258]+bytes([255])+tail[259:])):
            path=Path(temp)/(name+'.cmjpack');path.write_bytes(data)
            if launch(path)==0:raise RuntimeError(f'Malformed {name} pack accepted')
    print('T payload preserved; U tables matched original; malformed packs rejected')


if __name__=='__main__':main()

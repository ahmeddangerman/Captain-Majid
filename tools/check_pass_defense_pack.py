#!/usr/bin/env python3
"""Check V/W/X/Y/Z pass-defense tables, T compatibility, and malformed-pack rejection."""
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
    profiles=bytes(m[0x9662+12*((kind-35)&255)+7] for kind in range(1,118));rows=max(profiles)+1
    tail=bytes(m[0x8bbe:0x8cbe])+len(profiles).to_bytes(2,'little')+profiles+rows.to_bytes(2,'little')+bytes(m[0xb8ae:0xb8ae+48*rows])
    rules=bytearray((118).to_bytes(2,'little'))
    for kind in range(118):
        group=m[0x8e1b+2*kind]|m[0x8e1c+2*kind]<<8;pointer=m[group+10]|m[group+11]<<8
        rules.append(int(bool(pointer)))
        if pointer:
            value=m[pointer];conditional=not bool(value&128);rules.extend((value&127,int(conditional),m[pointer+1] if conditional else 0))
    tail+=rules
    version=new[:8]
    if version in (b'CMJSHOTW',b'CMJSHOTX',b'CMJSHOTY',b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb'):
        m.registers[6:8]=[26,27]
        direct=m[0x8d93]|m[0x8d94]<<8
        tail+=bytes((m[direct+10],m[direct+11],int(bool(m[0x827c+5]&128)),*m[0x8290:0x8294]))
    if version in (b'CMJSHOTX',b'CMJSHOTY',b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb'):
        m.registers[6:8]=[28,29]
        tail+=(118).to_bytes(2,'little')
        for kind in range(118):
            group=m[0x8e1b+2*kind]|m[0x8e1c+2*kind]<<8;pointer=m[group+8]|m[group+9]<<8
            tail+=bytes((int(bool(pointer)),))
            if pointer:
                value=m[pointer];conditional=not bool(value&128)
                tail+=bytes((value&127,int(conditional),m[pointer+1] if conditional else 0))
        m.registers[6:8]=[26,27]
        tail+=bytes((m[direct+8],m[direct+9],int(bool(m[0x827c+4]&128)),*m[0x8290:0x8294]))
    if version in (b'CMJSHOTY',b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb'):
        m.registers[6:8]=[0,2];tail+=bytes(m[0xaa47:0xaa52])
    if version in (b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb'):
        m.registers[6:8]=[28,29];tail+=bytes(m[0x9662+12*((kind-35)&255)+7] for kind in range(118,121))
    if version in (b'CMJSHOTa',b'CMJSHOTb'):
        m.registers[6:8]=[26,27];tail+=bytes((m[direct+12],m[direct+13],int(bool(m[0x827c+6]&128)),*m[0x8290:0x8294]))
    if version==b'CMJSHOTb':
        m.registers[6:8]=[28,29];profiles=bytes([0]+[m[0x9662+12*((kind-35)&255)+5] for kind in range(1,121)])
        count=max(profiles)+1;tail+=profiles+count.to_bytes(2,'little')+bytes(m[0xb12e:0xb12e+48*count])
    if old[:8]!=b'CMJSHOTT' or version not in (b'CMJSHOTV',b'CMJSHOTW',b'CMJSHOTX',b'CMJSHOTY',b'CMJSHOTZ',b'CMJSHOTa',b'CMJSHOTb') or old[8:]!=new[8:-len(tail)] or new[-len(tail):]!=tail:raise RuntimeError('Pass-defense pack does not preserve T and match original tables')
    def launch(pack):return subprocess.run([str(a.exe.resolve()),'--field',str(pack)],input='',capture_output=True,text=True).returncode
    if launch(a.old_pack.resolve()) or launch(a.new_pack.resolve()):raise RuntimeError('Valid T/V/W pack rejected')
    with tempfile.TemporaryDirectory(prefix='pass-defense-pack-',dir=a.private_temp) as temp:
        for name,data in (('truncated',new[:-1]),('trailing',new+b'\0'),('bad-row',new[:-len(tail)]+tail[:258]+bytes([255])+tail[259:])):
            path=Path(temp)/(name+'.cmjpack');path.write_bytes(data)
            if launch(path)==0:raise RuntimeError(f'Malformed {name} pack accepted')
    print('T payload preserved; pass-defense tables matched original; malformed packs rejected')


if __name__=='__main__':main()

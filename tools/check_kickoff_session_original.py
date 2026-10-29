#!/usr/bin/env python3
"""Compare saved prepared kickoff formation after earlier reset/team selection."""
import argparse,hashlib,json,random,subprocess
from pathlib import Path
from py65.devices.mpu6502 import MPU
from trace_field_script import ObservedCartridge
from check_keeper_saves_original import post_save_checkpoint
from verify_action_strength import PAYLOAD_SHA256

def snapshot(ram,phase):
 data=bytearray(post_save_checkpoint(ram,phase));data[361:365]=bytes([1,ram[0x28],ram[0x29],ram[0x629]])
 if phase==61:data[365:368]=bytes([1,ram[0x308],ram[0x38c]])
 return bytes(data)

def main():
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('rom','pack','ram','output_dir'):p.add_argument(name,type=Path)
 a=p.parse_args();rom=a.rom.read_bytes();base=a.ram.read_bytes()
 if len(rom)!=393232 or hashlib.sha256(rom[16:]).hexdigest()!=PAYLOAD_SHA256:p.error('Expected approved ROM')
 if len(base)!=2048 or base[0x441] not in range(1,11):p.error('Expected side0 field owner RAM')
 out=a.output_dir.resolve()
 if out.is_relative_to(Path(__file__).resolve().parents[1]):p.error('Private outputs must stay outside checkout')
 out.mkdir(parents=True,exist_ok=False);prg=rom[16:262160]
 rommem=ObservedCartridge(prg,bytes(2048),None,set());rommem.registers[6:8]=[26,27]
 cells=[rommem[x] for x in range(0xdc87,0xdcdf)]
 content=out/'private formation.cmjkick';content.write_text('CMJKICKFORMATION1\n'+' '.join(map(str,cells))+'\n')
 rng=random.Random(0x260);results=[];binary=str(Path('build/captain-phase-session').resolve());pack=str(a.pack.resolve())
 for side in (0,11):
  for left in range(4):
   for right in range(4):
    for repeat in range(2):
     name=f's{side}-l{left}-r{right}-n{repeat}';ram=bytearray(base)
     for slot in range(22):
      record=0x300+slot*12
      ram[record+5:record+9]=bytes(rng.randrange(256) for _ in range(4))
     for addr,value in ((0x5fb,11),(0x43b,0),(0x5ff,0),(0x62d,0),(0x600,1),(0x601,11),(0x442,11),
                        (0x389,0),(0x38b,0),(0x38e,0),(0x28,3),(0x29,2),(0x629,4),
                        (0x635,204),(0x637,124),(0x5fe,233),(0x2c,left),(0x2e,right)):
      ram[addr]=value
     ram[0x602:0x606]=bytes(4)
     source=out/(name+'-entry.cmjsession');source.write_bytes(snapshot(ram,60))
     original=bytearray(ram);original[0x5fb]=side
     m=ObservedCartridge(prg,original,None,set());m.registers[6:8]=[26,27];cpu=MPU(memory=m);cpu.pc=0xdae9;cpu.sp=0xfd;cpu.p=cpu.UNUSED
     for steps in range(20000):
      if cpu.pc==0xdaf5:break
      cpu.step()
     else:raise RuntimeError('Original kickoff placement instruction bound')
     dest=out/(name+'-native.cmjsession')
     subprocess.run([binary,pack,str(source),'--prepare-kickoff-formation',str(content),str(side),'--capture',str(dest)],capture_output=True,check=True,timeout=30)
     actual=dest.read_bytes();wanted=snapshot(bytes(m.ram[:2048]),61)
     if actual!=wanted:raise RuntimeError(f'{name} mismatch {[(i,x,y)for i,(x,y)in enumerate(zip(actual,wanted))if x!=y]}; lengths{len(actual)}/{len(wanted)}')
     if not results:
      if subprocess.run([binary,pack,str(dest),'--prepare-kickoff-formation',str(content),str(side)],capture_output=True).returncode==0:raise RuntimeError('Formation replay accepted')
      interactive=out/'interactive.cmjsession'
      run=subprocess.run([binary,pack,str(source),'--interactive'],input=f'prepare-kickoff-formation "{content}" {side}\nsave {interactive}\nquit\n',capture_output=True,text=True,check=True,timeout=30)
      if 'Error:' in run.stdout or interactive.read_bytes()!=actual:raise RuntimeError('Interactive formation mismatch')
     results.append({'side':side,'left':left,'right':right,'repeat':repeat,'instructions':steps,'match':True})
 (out/'verification.json').write_text(json.dumps({'cases':results,'scope':'Explicit prepared state after earlier reset/team selection. Original DAE9..DAF5 executes every instruction. Previous reset/menu, live frames and subsequent receiver/display are outside scope.'},indent=2)+'\n')
 print(f'{len(results)} saved kickoff formation comparisons match: {out}')
if __name__=='__main__':main()

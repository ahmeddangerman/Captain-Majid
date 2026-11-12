"""Check the private Y lineup extension and conservative format parsing."""
import subprocess
import sys
import tempfile
from pathlib import Path

exe,x_path,y_path,private=map(Path,sys.argv[1:])
if private.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
    raise ValueError('Temporary content must remain outside the checkout')
x,y=x_path.read_bytes(),y_path.read_bytes()
assert x[:8]==b'CMJSHOTX' and y[:8]==b'CMJSHOTY' and y[8:len(x)]==x[8:] and len(y)==len(x)+11

def accepts(path):
    return subprocess.run([str(exe.resolve()),'--field',str(path.resolve())],input='',capture_output=True,text=True).returncode==0

assert accepts(x_path) and accepts(y_path)
invalid=bytearray(y);invalid[-1]=0
with tempfile.TemporaryDirectory(prefix='first-match-pack-',dir=private) as tmp:
    for name,data in (('truncated',y[:-1]),('trailing',y+b'\0'),('invalid-type',invalid),('mislabelled',b'CMJSHOTX'+y[8:])):
        path=Path(tmp)/(name+'.cmjpack');path.write_bytes(data)
        assert not accepts(path),name+' accepted'
print('Y preserves X, and truncated, trailing, invalid and mislabelled variants are rejected')

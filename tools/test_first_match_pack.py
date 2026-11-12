"""Check the private Y lineup extension and conservative format parsing."""
import subprocess
import sys
import tempfile
from pathlib import Path

exe,x_path,y_path,private=map(Path,sys.argv[1:5])
z_path=Path(sys.argv[5]) if len(sys.argv)>5 else None
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
if z_path:
    z=z_path.read_bytes()
    assert z[:8]==b'CMJSHOTZ' and z[8:-3]==y[8:] and accepts(z_path)
    invalid_z=bytearray(z);invalid_z[-1]=255
    with tempfile.TemporaryDirectory(prefix='generic-pass-pack-',dir=private) as tmp:
        for name,data in (('truncated',z[:-1]),('trailing',z+b'\0'),('invalid-row',invalid_z),('mislabelled',b'CMJSHOTY'+z[8:])):
            path=Path(tmp)/(name+'.cmjpack');path.write_bytes(data)
            assert not accepts(path),name+' accepted'
    print('Z preserves Y and rejects malformed generic profiles')
    with tempfile.TemporaryDirectory(prefix='action2-pass-pack-',dir=private) as tmp:
        a=b'CMJSHOTa'+z[8:]+bytes((5,3,1,3,2,1,0))
        path=Path(tmp)/'action2.cmjpack';path.write_bytes(a)
        assert accepts(path),'action2 pack rejected'
        for name,data in (('truncated',a[:-1]),('trailing',a+b'\0'),('zero-range',a[:-7]+bytes(7)),('mislabelled',b'CMJSHOTZ'+a[8:])):
            path=Path(tmp)/(name+'.cmjpack');path.write_bytes(data)
            assert not accepts(path),name+' accepted'
    print('action2 pack preserves Z and rejects malformed comparison data')
    with tempfile.TemporaryDirectory(prefix='goal-area-pack-',dir=private) as tmp:
        b=b'CMJSHOTb'+a[8:]+bytes(121)+(1).to_bytes(2,'little')+bytes(48)
        path=Path(tmp)/'goal-area.cmjpack';path.write_bytes(b)
        assert accepts(path),'goal-area pack rejected'
        bad=bytearray(b);bad[-171]=255
        for name,data in (('truncated',b[:-1]),('trailing',b+b'\0'),('bad-profile',bad),('mislabelled',b'CMJSHOTa'+b[8:])):
            path=Path(tmp)/(name+'.cmjpack');path.write_bytes(data)
            assert not accepts(path),name+' accepted'
    print('goal-area pack preserves action2 and rejects malformed profiles')


print('Y preserves X, and truncated, trailing, invalid and mislabelled variants are rejected')

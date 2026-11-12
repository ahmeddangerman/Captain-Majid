"""Check private U/V/W packs and optional X; pass executable, U, V, temp, W, [X]."""
import subprocess
import sys
import tempfile
from pathlib import Path

exe, old, extended, private_temp, direct = map(Path, sys.argv[1:6])
block = Path(sys.argv[6]) if len(sys.argv)>6 else None
exe = exe.resolve()
if private_temp.resolve().is_relative_to(Path(__file__).resolve().parents[1]):
    raise ValueError('Private test files must stay outside the checkout')

def run(path, mode='--field', data=''):
    return subprocess.run([str(exe), mode, str(path.resolve())], input=data,
                          text=True, capture_output=True)

u, v = old.read_bytes(), extended.read_bytes()
w = direct.read_bytes()
assert u[:8] == b'CMJSHOTU' and v[:8] == b'CMJSHOTV'
assert v[8:len(u)] == u[8:], 'V must preserve U payload'
assert w[:8] == b'CMJSHOTW' and w[8:len(v)] == v[8:] and len(w)==len(v)+7
assert run(old).returncode == 0 and run(extended).returncode == 0 and run(direct).returncode == 0
if block:
    x=block.read_bytes()
    assert x[:8]==b'CMJSHOTX' and x[8:len(w)]==w[8:] and len(x)>len(w)
    assert run(block).returncode==0
# Find choices genuinely supported by V with variant zero; old U has no rule data.
samples = [f'23 0 35 {x} {y} {r} 0 ' + ' '.join(['0'] * 10)
           for x in (80, 108, 159, 200) for y in (84, 108, 124, 172) for r in range(8)]
inputs = '\n'.join(samples) + '\n'
a, b = run(old, '--pass-defender-final', inputs), run(extended, '--pass-defender-final', inputs)
assert a.returncode == b.returncode == 0
old_rows, new_rows = a.stdout.splitlines(), b.stdout.splitlines()
assert len(old_rows) == len(new_rows) == len(samples)
assert any(row == '1' for row in new_rows), 'No supported final choice tested'
assert all(row == 'unsupported' for row in old_rows), 'U must not assume absent special options'
with tempfile.TemporaryDirectory(prefix='pass-pack-versions-', dir=private_temp) as tmp:
    bad_range=bytearray(w);bad_range[-7]=0
    bad_eligibility=bytearray(w);bad_eligibility[-5]=2
    bad_thresholds=bytearray(w);bad_thresholds[-4:]=bytes((0,1,0,0))
    malformed=[('u-truncated', u[:-1]), ('u-trailing', u + b'\0'),
                       ('v-truncated', v[:-1]), ('v-trailing', v + b'\0'),
                       ('w-truncated',w[:-1]),('w-trailing',w+b'\0'),
                       ('w-invalid-range',bad_range),('w-invalid-eligibility',bad_eligibility),
                       ('w-invalid-thresholds',bad_thresholds),
                       ('extended-mislabelled-u', b'CMJSHOTU' + v[8:])]
    if block:
        bad_flag=bytearray(x);bad_flag[len(w)+2]=2
        malformed += [('x-truncated',x[:-1]),('x-trailing',x+b'\0'),('x-invalid-flag',bad_flag),('x-mislabelled-w',b'CMJSHOTW'+x[8:])]
    for name, data in malformed:
        path = Path(tmp) / (name + '.cmjpack')
        path.write_bytes(data)
        assert run(path).returncode != 0, name + ' accepted'
print('U/V/W' + ('/X' if block else '') + ' payloads, missing rules, malformed packs and mislabelled formats checked')

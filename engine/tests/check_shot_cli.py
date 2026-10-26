#!/usr/bin/env python3
"""Exercise skill, action-power and goalkeeper research protocols."""
import argparse
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
binary = str(parser.parse_args().binary.resolve())
fixtures = [
    ("skill-index", "skill", "10 5 3", "23"),
    ("action-power", "action", "256 100 100 0 0 0", "256 229 229 0"),
    ("keeper", "keeper", "100 256 100 100 200 50 0 240 4 0 3 10 3 4 200 180 100 0",
     "256 150 128 11 139 2 3 0"),
]
for mode, command, fields, expected in fixtures:
    result = subprocess.run([binary, f"--{mode}-batch"], input=fields + "\n",
                            capture_output=True, text=True, timeout=5, check=True)
    assert result.stdout.strip() == expected, result.stdout
    for bad in (fields + " extra", " ".join(fields.split()[:-1]), "-1 " + fields.split(" ", 1)[1]):
        result = subprocess.run([binary, f"--{mode}-batch"], input=bad + "\n",
                                capture_output=True, text=True, timeout=5)
        assert result.returncode == 2 and not result.stdout, (mode, bad, result)
    result = subprocess.run([binary, "--replica"],
        input=f"state\n{command} {fields}\n{command} {fields} extra\nstate\nquit\n",
        capture_output=True, text=True, timeout=5, check=True)
    replies = result.stdout.splitlines()
    assert len(replies) == 4 and replies[0] == replies[3]
    assert replies[1:3] == [expected, "error malformed"], replies

for mode, fields in [
    ("skill-index", "256 0 0"),
    ("action-power", "65536 0 0 0 0 0"),
    ("action-power", "256 100 100 2 0 0"),
    ("action-power", "256 100 100 0 0 2"),
    ("keeper", "100 256 100 100 0 50 0 240 4 0 3 10 3 4 200 180 100 0"),
    ("keeper", "100 256 100 100 200 50 2 240 4 0 3 10 3 4 200 180 100 0"),
    ("keeper", "100 256 100 100 200 50 0 240 4 0 3 10 3 4 200 180 100 1"),
]:
    result = subprocess.run([binary, f"--{mode}-batch"], input=fields + "\n",
                            capture_output=True, text=True, timeout=5)
    assert result.returncode == 2 and not result.stdout, (mode, fields)
print("Shot research CLI checks passed: strict inputs and fixture state preserved")

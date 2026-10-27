#!/usr/bin/env python3
"""Exercise energy, keeper state/story, and reference RNG research protocols."""
import argparse
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
binary = str(parser.parse_args().binary.resolve())
fixtures = [
    ("energy", "energy", "100 30 1 0 0 0 0 0", "70 30 0 1"),
    ("energy", "energy", "20 30 1 0 0 0 0 0", "0 30 1 0"),
    ("keeper-story", "story", "0 0 5 1 1 128", "3 1 0"),
    ("keeper-story", "story", "0 0 5 4 131 128", "0 0 0"),
    ("keeper-state", "keeperstate", "250 70 9 2 0 1 1", "5 127 4"),
    ("random-step", "random", "0 0 73 0 0 0 0", "1 255 255 0"),
    ("outcome", "outcome", "0 0 6 0 0 0 122 47 222 7 7 102 206 119 227 248 203 74 132 208 235 2 4 5 7 8 9 10 11",
     "0 0 11 248 120 227 0 0 102 0 203 74 132 208 4"),
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
    ("energy", "65536 30 1 0 0 0 0 0"),
    ("energy", "100 30 1 0 0 0 2 0"),
    ("energy", "100 30 1 0 0 0 0 2"),
    ("keeper-story", "256 0 5 1 1 128"),
    ("keeper-state", "250 70 9 2 0 2 1"),
    ("keeper-state", "250 70 9 2 0 1 2"),
    ("random-step", "0 0 73 0 0 0 2"),
]:
    result = subprocess.run([binary, f"--{mode}-batch"], input=fields + "\n",
                            capture_output=True, text=True, timeout=5)
    assert result.returncode == 2 and not result.stdout, (mode, fields)
print("State research CLI checks passed: exact inputs and ordered fixture state preserved")

#!/usr/bin/env python3
"""Exercise the actual terminal protocol and strict input validation."""
import argparse
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
args = parser.parse_args()
binary = str(args.binary.resolve())
commands = [
    "state", "contact 7 0 extra", "contact -1 0", "contact 7 4294967296",
    "contact 7", "contact 7 +0", "contact 7 0x0", "contact 7 0.0",
    "state", "contact 42 0", "contact 999 0", "contact 7 1", "contact 7 0",
    "contact 7 0", "state", "quit",
]
result = subprocess.run(
    [binary, "--replica"], input="\n".join(commands) + "\n",
    capture_output=True, text=True, timeout=10, check=True,
)
lines = result.stdout.splitlines()
if len(lines) != len(commands) - 1:
    raise AssertionError(f"Unexpected reply count: {result.stdout!r}")
initial = "CM1|rules=contact-loss-v1|power=300|loss=37|next=0|owner=0|participants=7:0,42:1"
assert lines[0] == initial
assert all(line == "error malformed" for line in lines[1:8]), lines
assert lines[8] == initial
assert lines[9:12] == ["rejected unauthorized", "rejected unknown_participant", "rejected out_of_order"]
assert lines[12] == "accepted"
assert lines[13] == "rejected out_of_order"
assert lines[14] == initial.replace("power=300", "power=263").replace("next=0", "next=1")
for malformed in ("-1 0", "1 256", "65536 1", "1 1 extra", "1 +1", "1.0 1"):
    bad = subprocess.run([binary, "--strength-batch"], input=malformed + "\n",
                         capture_output=True, text=True, timeout=5)
    assert bad.returncode == 2 and not bad.stdout, (malformed, bad)
print("CLI checks passed: malformed input and rejected commands preserve state")

fixture = "300 256 100 100 200 50 0 0 0 1 3 160 100 50 0"
expected = "256 150 37 128 1 263 3 0"
result = subprocess.run([binary, "--defender-batch"], input=fixture + "\n",
                        capture_output=True, text=True, timeout=5, check=True)
assert result.stdout.strip() == expected, result.stdout
for malformed in (fixture + " extra", fixture.replace("300", "65536", 1),
                  "300 256 100 100 0 50 0 0 0 1 3 160 100 50 0",
                  "300 256 100 100 200 50 2 0 0 1 3 160 100 50 0",
                  "300 256 100 100 200 50 0 0 0 1 3 100 160 50 0"):
    bad = subprocess.run([binary, "--defender-batch"], input=malformed + "\n",
                         capture_output=True, text=True, timeout=5)
    assert bad.returncode == 2 and not bad.stdout, (malformed, bad)
result = subprocess.run([binary, "--replica"],
                        input=f"state\ndefender {fixture}\ndefender {fixture} extra\nstate\nquit\n",
                        capture_output=True, text=True, timeout=5, check=True)
assert result.stdout.splitlines() == [initial, expected, "error malformed", initial]
print("Defender CLI checks passed: strict parameters and stateless replica calculation")

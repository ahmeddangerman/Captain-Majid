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

#!/usr/bin/env python3
"""Development proof: two native replicas driven through loopback TCP adapters.

This harness supplies transport, not production networking or authentication.
"""
from __future__ import annotations

import argparse
import queue
import socket
import subprocess
import threading
import time
from pathlib import Path


def receive_line(connection: socket.socket, timeout: float = 5) -> bytes:
    deadline = time.monotonic() + timeout
    original_timeout = connection.gettimeout()
    data = bytearray()
    try:
        while len(data) < 4096:
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError("Development protocol line deadline exceeded")
            connection.settimeout(remaining)
            value = connection.recv(1)
            if not value:
                if data:
                    raise ValueError("Connection closed during a command/reply")
                return b""
            data.extend(value)
            if value == b"\n":
                return bytes(data)
        raise ValueError("Oversized development protocol line")
    finally:
        connection.settimeout(original_timeout)


class Adapter:
    def __init__(self, binary: Path, delay: float):
        self.delay = delay
        self.errors: queue.Queue[BaseException] = queue.Queue()
        self.lines: queue.Queue[str] = queue.Queue()
        self.listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            self.listener.bind(("127.0.0.1", 0))
            self.listener.listen(1)
            self.listener.settimeout(5)
            self.address = self.listener.getsockname()
            self.child = subprocess.Popen(
                [str(binary.resolve()), "--replica"], stdin=subprocess.PIPE,
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, bufsize=1,
            )
        except BaseException:
            self.listener.close()
            raise
        self.reader = threading.Thread(target=self._read_output, daemon=True)
        self.worker = threading.Thread(target=self._serve, daemon=True)
        self.reader.start()
        self.worker.start()

    def _read_output(self):
        assert self.child.stdout is not None
        for line in self.child.stdout:
            self.lines.put(line)
        self.lines.put("")

    def _serve(self):
        try:
            connection, _ = self.listener.accept()
            with connection:
                connection.settimeout(5)
                while True:
                    line = receive_line(connection)
                    if not line:
                        return
                    time.sleep(self.delay)
                    assert self.child.stdin is not None
                    self.child.stdin.write(line.decode("ascii"))
                    self.child.stdin.flush()
                    response = self.lines.get(timeout=5)
                    if not response:
                        raise RuntimeError("Native replica disconnected before replying")
                    connection.sendall(response.encode("ascii"))
        except BaseException as error:
            self.errors.put(error)
        finally:
            self.listener.close()

    def close(self):
        # Caller closes its connection first, allowing the adapter to observe EOF.
        self.worker.join(timeout=6)
        stuck = self.worker.is_alive()
        if stuck:
            self.listener.close()
        self.child.terminate()
        try:
            self.child.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.child.kill()
            self.child.wait(timeout=5)
        self.reader.join(timeout=2)
        for pipe in (self.child.stdin, self.child.stdout, self.child.stderr):
            if pipe is not None:
                pipe.close()
        if stuck:
            raise RuntimeError("Adapter did not stop after disconnect")
        if not self.errors.empty():
            raise RuntimeError("TCP adapter failed") from self.errors.get()


def exchange(connection: socket.socket, command: str) -> str:
    connection.sendall((command + "\n").encode("ascii"))
    response = receive_line(connection)
    if not response:
        raise RuntimeError("Missing replica reply")
    return response.decode("ascii").rstrip("\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=Path("build/captain-prototype"))
    args = parser.parse_args()
    adapters = []
    connections = []
    try:
        for delay in (0, 0.02):
            adapter = Adapter(args.binary, delay)
            adapters.append(adapter)
            connection = socket.create_connection(adapter.address, timeout=5)
            connection.settimeout(5)
            connections.append(connection)

        def both(command):
            answers = [exchange(connection, command) for connection in connections]
            if answers[0] != answers[1]:
                raise AssertionError(f"Replicas disagree after {command}: {answers}")
            return answers[0]

        initial = both("state")
        if initial != "CM1|rules=contact-loss-v1|power=300|loss=37|next=0|owner=0|participants=7:0,42:1":
            raise AssertionError(f"Unexpected initial fixture: {initial}")

        defender_cases = [
            ("300 256 100 100 200 50 0 0 0 1 3 160 100 50 0", "256 150 37 128 1 263 3 0"),
            ("300 256 100 100 200 50 1 1 4 0 3 160 100 50 0", "16 9 2 255 0 300 3 0"),
            ("300 256 100 100 200 50 0 1 4 0 3 160 100 50 0", "256 255 63 75 2 300 131 1"),
            ("300 256 100 100 0 50 0 0 0 1 3 160 100 50 0", "error malformed"),
        ]
        for fixture, expected in defender_cases:
            if both("defender " + fixture) != expected:
                raise AssertionError(f"Unexpected defender result for {fixture}")
            if both("state") != initial:
                raise AssertionError("Research calculation mutated ordered fixture state")

        shot_cases = [
            ("skill 10 5 3", "23"),
            ("action 128 100 100 0 0 0", "128 229 114 0"),
            # Feed that action's power (114) into the next independent research calculation.
            ("keeper 114 256 100 100 200 50 0 240 4 0 3 10 3 4 200 180 100 0",
             "256 150 145 11 156 2 3 0"),
            ("action 256 100 100 2 0 0", "error malformed"),
            ("keeper 114 256 100 100 0 50 0 240 4 0 3 10 3 4 200 180 100 0", "error malformed"),
        ]
        for command, expected in shot_cases:
            if both(command) != expected:
                raise AssertionError(f"Unexpected shot research reply for {command}")
            if both("state") != initial:
                raise AssertionError("Shot research query mutated ordered fixture state")

        cases = [
            ("contact 42 0", "rejected unauthorized"),
            ("contact 999 0", "rejected unknown_participant"),
            ("contact 7 1", "rejected out_of_order"),
            ("contact 7 0 extra", "error malformed"),
            ("contact 7 0", "accepted"),
            ("contact 7 0", "rejected out_of_order"),
        ] + [(f"contact 7 {i}", "accepted") for i in range(1, 10)]
        next_sequence = 0
        for command, expected_reply in cases:
            actual_reply = both(command)
            if actual_reply != expected_reply:
                raise AssertionError(f"{command}: {actual_reply} != {expected_reply}")
            if expected_reply == "accepted":
                next_sequence += 1
            expected_state = initial.replace("power=300", f"power={max(0, 300 - next_sequence * 37)}")
            expected_state = expected_state.replace("next=0", f"next={next_sequence}")
            if both("state") != expected_state:
                raise AssertionError(f"Unexpected state after {command}")
        print(f"TCP replay passed: 2 native processes, {len(cases)} commands, identical full snapshots")
        print("Included delayed delivery, duplicate/future commands, invalid input, and participant checks")
        print("Defender TCP proof passed: ordinary, weakened, special, and rejected inputs agree")
        print("Shot TCP proof passed: skill adjustment, action power, keeper comparison, and invalid input agree")
    finally:
        for connection in connections:
            connection.close()
        errors = []
        for adapter in adapters:
            try:
                adapter.close()
            except Exception as error:
                errors.append(error)
        if errors:
            raise RuntimeError("Replica cleanup failed") from errors[0]
    print("Disconnect cleanup passed: adapter threads and native processes stopped")


if __name__ == "__main__":
    main()

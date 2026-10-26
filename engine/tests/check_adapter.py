#!/usr/bin/env python3
"""Exercise failure paths in the loopback development adapter."""
import argparse
import socket
import subprocess
import sys
import threading
import time
import unittest
from pathlib import Path
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "tools"))
import check_replica_sync as sync

binary = Path("build/captain-prototype")

class AdapterFailures(unittest.TestCase):
    def test_socket_setup_failure_leaves_no_child_running(self):
        children = []
        real_popen = subprocess.Popen

        def spawn(*args, **kwargs):
            child = real_popen(*args, **kwargs)
            children.append(child)
            return child

        try:
            with patch.object(sync.subprocess, "Popen", side_effect=spawn):
                with patch.object(sync.socket, "socket", side_effect=OSError("allocation failed")):
                    with self.assertRaises(OSError):
                        sync.Adapter(binary, 0)
            self.assertTrue(all(child.poll() is not None for child in children),
                            "Failed adapter construction left a child running")
        finally:
            for child in children:
                if child.poll() is None:
                    child.terminate()
                child.communicate(timeout=5)

    def test_continuous_trickle_obeys_line_deadline(self):
        receiver, sender = socket.socketpair()

        def trickle():
            try:
                for _ in range(12):
                    sender.sendall(b"x")
                    time.sleep(0.04)
                sender.sendall(b"\n")
            except OSError:
                pass
            finally:
                sender.close()

        worker = threading.Thread(target=trickle)
        worker.start()
        try:
            with self.assertRaises(TimeoutError):
                sync.receive_line(receiver, timeout=0.12)
        finally:
            receiver.close()
            worker.join(timeout=2)
        self.assertFalse(worker.is_alive())


if __name__ == "__main__":
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--binary", type=Path, default=binary)
    args, remaining = parser.parse_known_args()
    binary = args.binary
    unittest.main(argv=[sys.argv[0], *remaining])

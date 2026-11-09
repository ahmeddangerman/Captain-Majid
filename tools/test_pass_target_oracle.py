import subprocess
import sys
import unittest
from pathlib import Path
from verify_pass_target_original import iter_cases


class PassTargetOracleTests(unittest.TestCase):
    def test_domain_and_sentinel_coverage(self):
        cases = list(iter_cases())
        self.assertEqual(len(cases), 65536 + 2000 + 256)
        self.assertEqual(cases[0], (7, 42, 0, 0))
        self.assertEqual(cases[65535], (7, 42, 255, 255))
        self.assertEqual(cases[-1], (255, 255, 0, 0))

    def test_discovery_needs_no_optional_emulator(self):
        result = subprocess.run([sys.executable, '-S', '-c', 'import verify_pass_target_original'],
                                cwd=Path(__file__).resolve().parent, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()

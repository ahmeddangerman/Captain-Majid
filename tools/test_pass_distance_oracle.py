import subprocess
import sys
import unittest
from pathlib import Path

from verify_pass_distance_original import iter_cases


class PassDistanceOracleTests(unittest.TestCase):
    def test_exhaustive_stream_covers_valid_domain(self):
        cases = iter_cases(True)
        self.assertEqual(next(cases), (48, 80, 0))
        count = 1
        last = None
        for last in cases:
            count += 1
        self.assertEqual(count, 160 * 96 * 240)
        self.assertEqual(last, (207, 175, 239))

    def test_sample_stream_is_deterministic(self):
        self.assertEqual(list(iter_cases(False)), list(iter_cases(False)))
        self.assertEqual(len(list(iter_cases(False))), 22560)

    def test_discovery_needs_no_optional_emulator(self):
        result = subprocess.run([sys.executable, '-S', '-c', 'import verify_pass_distance_original'],
                                cwd=Path(__file__).resolve().parent, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)


if __name__ == '__main__':
    unittest.main()

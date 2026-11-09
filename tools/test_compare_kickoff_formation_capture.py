import os
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'tools' / 'compare_kickoff_formation_capture.py'
BINARY = Path(sys.argv.pop(1)) if len(sys.argv) > 1 else ROOT / 'build' / ('kickoff-formation-tests.exe' if os.name == 'nt' else 'kickoff-formation-tests')


class CaptureComparisonTest(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        before = bytearray(2048)
        after = bytearray(2048)
        for slot in range(22):
            base = 0x300 + 12 * slot
            before[base + 5:base + 9] = bytes((17, 10, 19, 20))
            after[base + 5:base + 9] = bytes((17, 52 if slot < 11 else 60,
                                               0 if slot in (0, 11) else 19,
                                               84 if slot < 11 else 92))
        self.before, self.after, self.table = (self.root / name for name in ('before.ram', 'after.ram', 'formation.cmjkick'))
        self.before.write_bytes(before)
        self.after.write_bytes(after)
        self.table.write_text('CMJKICKFORMATION1\n' + ' '.join(map(str, [0] * 44 + [13] * 44)))

    def run_comparison(self):
        return subprocess.run([sys.executable, str(SCRIPT), str(self.before), str(self.after), str(self.table),
                               '--side', '0', '--binary', str(BINARY)], capture_output=True, text=True)

    def test_matching_capture_and_named_first_difference(self):
        result = self.run_comparison()
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('44 coordinate words', result.stdout)
        data = bytearray(self.after.read_bytes())
        data[0x300 + 12 * 3 + 6] += 1
        self.after.write_bytes(data)
        result = self.run_comparison()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('player[3].x', result.stderr)

    def test_rejects_truncated_evidence(self):
        self.before.write_bytes(bytes(2047))
        result = self.run_comparison()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('2048', result.stderr)

    def test_rejects_invalid_table_and_changed_formations(self):
        self.table.write_text('CMJKICKFORMATION1\n' + ' '.join(map(str, [240] + [0] * 87)))
        result = self.run_comparison()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('0..239', result.stderr)
        self.table.write_text('CMJKICKFORMATION1\n' + ' '.join(map(str, [0] * 88)))
        data = bytearray(self.after.read_bytes())
        data[0x2c] = 1
        self.after.write_bytes(data)
        result = self.run_comparison()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('Formations changed', result.stderr)


if __name__ == '__main__':
    unittest.main()

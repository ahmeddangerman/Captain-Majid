import hashlib
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from export_pass_distance import export_data, write_private


class ExportPassDistanceTests(unittest.TestCase):
    def test_import_needs_no_optional_emulator(self):
        result = subprocess.run([sys.executable, '-S', '-c', 'import export_pass_distance'],
                                cwd=Path(__file__).resolve().parent, capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    def test_extracts_only_the_table_from_verified_payload(self):
        rom = bytearray(393232)
        rom[:4] = b"NES\x1a"
        start = 16 + 30 * 8192 + (0xdfbd - 0xc000)
        rom[start:start + 20] = bytes(range(20))
        digest = hashlib.sha256(rom[16:]).hexdigest()
        with patch('export_pass_distance.PAYLOAD_SHA256', digest):
            self.assertEqual(export_data(rom), b'CMJPDST1' + bytes(range(20)))
        with self.assertRaisesRegex(ValueError, 'verified Japanese reference'):
            export_data(rom)

    def test_refuses_checkout_and_existing_output(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp) / 'checkout'
            root.mkdir()
            with self.assertRaisesRegex(ValueError, 'outside'):
                write_private(root / 'table.cmjpdist', b'data', root)
            existing = Path(temp) / 'existing.cmjpdist'
            existing.write_bytes(b'original')
            with self.assertRaises(FileExistsError):
                write_private(existing, b'new', root)
            self.assertEqual(existing.read_bytes(), b'original')
            fresh = Path(temp) / 'fresh.cmjpdist'
            write_private(fresh, b'new', root)
            self.assertEqual(fresh.read_bytes(), b'new')


if __name__ == '__main__':
    unittest.main()

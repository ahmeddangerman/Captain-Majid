import unittest
from check_ai_cell_fallback_original import has_shot_gates

class PackVersionTests(unittest.TestCase):
    def test_versions_with_shot_gates(self):
        for version in b'STUVWXY':
            self.assertTrue(has_shot_gates(b'CMJSHOT' + bytes((version,))))
    def test_older_or_invalid_versions_do_not_claim_gates(self):
        for magic in (b'CMJSHOTR', b'CMJSHOTQ', b'CMJSHOTZ', b'garbage'):
            self.assertFalse(has_shot_gates(magic))

if __name__ == '__main__': unittest.main()

import tempfile
import unittest
from pathlib import Path

from verify_player7_pass_end_to_end import EvidenceMismatch, case_ids, compare_checkpoint, compare_represented


class ReplayChecks(unittest.TestCase):
    def test_first_mismatch_names_boundary_and_field(self):
        expected = bytes(324)
        actual = bytearray(325)
        actual[23] = 7
        actual[24] = 8
        with self.assertRaisesRegex(EvidenceMismatch, r"case alpha.*second defender.*field\.ball_y.*native 7.*original 0"):
            compare_represented(actual, expected, True, "alpha", "second defender")

    def test_extended_phase_byte_does_not_shift_earlier_fields(self):
        expected = bytes(324)
        actual = bytearray(325)
        actual[250] = 1
        with self.assertRaisesRegex(EvidenceMismatch, r"clock\.remaining\.low"):
            compare_represented(actual, expected, True, "0", "dispatch")

    def test_extended_phase_byte_shifts_subsequent_fields(self):
        expected = bytes(324)
        actual = bytearray(325)
        actual[291] = 9
        with self.assertRaisesRegex(EvidenceMismatch, r"recovery\.opposing_player"):
            compare_represented(actual, expected, True, "0", "defender")

    def test_truncated_checkpoint_identifies_boundary(self):
        with self.assertRaisesRegex(EvidenceMismatch, r"case 7, boundary arrival: truncated"):
            compare_checkpoint(b"short", bytes(2048), 99, 19, "7", "arrival")

    def test_case_discovery_requires_unambiguous_nonempty_names(self):
        with tempfile.TemporaryDirectory() as temp:
            directory = Path(temp)
            (directory / "0-clock.cmjsession").touch()
            (directory / "12-choice1-clock.cmjsession").touch()
            self.assertEqual(case_ids(directory, "-clock.cmjsession"), ["0", "12-choice1"])
            with self.assertRaisesRegex(ValueError, "No starting checkpoints"):
                case_ids(directory, "-absent.cmjsession")


if __name__ == "__main__":
    unittest.main()

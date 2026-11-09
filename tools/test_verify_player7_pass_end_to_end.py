import tempfile
import unittest
from pathlib import Path

from verify_player7_pass_end_to_end import EvidenceMismatch, case_ids, compare_checkpoint, compare_contact_context, compare_flight_context, compare_represented


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

    def test_retained_contact_uses_original_defender_evidence(self):
        source = bytearray(2048)
        source[0x43d], source[0x32], source[0x43f] = 1, 18, 60
        actual = bytearray(421)
        actual[399:413] = bytes((1, 2, 1, 0, 0, 18, 0, 59, 0, 0, 0, 255, 0, 0))
        with self.assertRaisesRegex(EvidenceMismatch, r"boundary field.handoff, contact.cost.low: native 59, original 60"):
            compare_contact_context(actual, source, 2, 255, "0", "field.handoff")

    def test_flight_metadata_reports_first_changed_field(self):
        raw = bytearray(2048)
        raw[0x62b], raw[0x638], raw[0x61a], raw[0x5fc] = 3, 103, 255, 7
        actual = bytearray(418)
        actual[339:345] = bytes((1, 3, 103, 254, 0, 1))
        actual[393:398] = bytes((1, 7, 103, 0, 0))
        with self.assertRaisesRegex(EvidenceMismatch, r"boundary flight.setup, flight.counter: native 254, original 255"):
            compare_flight_context(actual, raw, "0", "flight.setup")

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

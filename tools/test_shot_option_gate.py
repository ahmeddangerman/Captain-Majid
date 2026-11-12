import unittest

from export_shot_content import no_variant_zero_shot_option


class ShotOptionGateTests(unittest.TestCase):
    def test_empty_and_other_variants(self):
        memory = bytearray(65536)
        self.assertTrue(no_variant_zero_shot_option(memory, 0))
        self.assertFalse(no_variant_zero_shot_option(memory, 1))
        memory[0x9000:0x9005] = bytes((0x22, 0x18, 0x25, 0x18, 3))
        self.assertTrue(no_variant_zero_shot_option(memory, 0x9000))

    def test_variant_zero_is_not_safe(self):
        memory = bytearray(65536); memory[0x9000:0x9002] = bytes((0x30, 3))
        self.assertFalse(no_variant_zero_shot_option(memory, 0x9000))

    def test_missing_terminator_rejected(self):
        memory = bytearray(65536); memory[0xbfff] = 0x22
        with self.assertRaises(ValueError): no_variant_zero_shot_option(memory, 0xbfff)


if __name__ == '__main__': unittest.main()

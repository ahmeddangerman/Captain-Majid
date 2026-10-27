#!/usr/bin/env python3
"""Checks for bounded cartridge tracing; all PRG bytes are synthetic."""
import unittest
from reference_cartridge import ReferenceCartridge

class CartridgeChecks(unittest.TestCase):
    def setUp(self):
        self.memory = ReferenceCartridge(b''.join(bytes([i])*8192 for i in range(32)))

    def test_unsupported_expansion_and_hardware_reject(self):
        for address in (0x2000,0x401F,0x4020,0x5000,0x5FFF):
            with self.subTest(address=address):
                with self.assertRaises(RuntimeError):
                    _ = self.memory[address]
                with self.assertRaises(RuntimeError):
                    self.memory[address] = 1
        self.memory[0x6000] = 55
        self.assertEqual(self.memory[0x6000],55)

    def test_internal_ram_mirrors_and_slice_bounds(self):
        self.memory[0x1800] = 37
        self.assertEqual([self.memory[a] for a in (0,0x800,0x1000,0x1800)],[37]*4)
        with self.assertRaises(ValueError):
            self.memory[0:2] = [1]

    def test_prg_mapping_and_mode(self):
        windows = (0x8000,0xA000,0xC000,0xE000)
        self.assertEqual([self.memory[a] for a in windows],[26,27,30,31])
        self.memory[0x8000] = 6
        self.memory[0x8001] = 28
        self.assertEqual(self.memory[0x8000],28)
        self.memory[0x8000] = 0x46
        self.assertEqual([self.memory[a] for a in windows],[30,27,28,31])
        with self.assertRaises(RuntimeError):
            self.memory[0xC000] = 1
        self.memory[0x8000] = 0
        with self.assertRaises(RuntimeError):
            self.memory[0x8001] = 1

if __name__=='__main__':unittest.main()

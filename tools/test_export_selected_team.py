import unittest
import export_shot_content as content

class SelectedTeamExportTests(unittest.TestCase):
    def test_selected_team_record_and_bounds(self):
        self.assertTrue(hasattr(content, "selected_team_bytes"), "selected-team exporter is missing")
        memory = [0] * 65536
        memory[0x818e:0x8199] = [0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4]
        memory[0xbab2:0xbab4] = [0x00, 0xb0]
        memory[0xb000:0xb006] = [9, 8, 7, 6, 5, 4]
        self.assertEqual(content.selected_team_bytes(memory, 3), bytes([9, 8, 7, 6, 5, 4]))
        for team in (2, 7, 255):
            with self.assertRaises(ValueError): content.selected_team_bytes(memory, team)
        memory[0xbab2:0xbab4] = [0xff, 0xbf]
        with self.assertRaises(ValueError): content.selected_team_bytes(memory, 3)

if __name__ == "__main__": unittest.main()

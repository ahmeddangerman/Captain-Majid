import unittest

import verify_observed_world_round as observed_world
from verify_observed_world_round import collect_samples, compare_checkpoints
from verify_observed_nmi_world_round import collect_nmi_samples


def trace_events():
    events = []
    for frame in range(22):
        base = bytearray(2048)
        base[0x1c], base[0xe2], base[0xe3], base[0x517] = 1, frame, 55, 64
        loaded = bytearray(base)
        loaded[0x32], loaded[0x33] = 160, 3
        for offset, (pc, ram) in enumerate((('E3CA', base), ('E3DE', loaded),
                                             ('E406', base), ('E412', base))):
            events.append((frame * 4 + offset, pc, frame, bytes(ram)))
    return events


class ObservedWorldVerificationTest(unittest.TestCase):
    def test_selects_round_and_contact_boundary(self):
        selector = getattr(observed_world, 'trace_bounds', None)
        self.assertTrue(callable(selector), 'round boundary selection is unavailable')
        events = [(0, 'E407', 1), (1, 'E145', 2), (2, 'E407', 3), (3, 'E4BE', 4)]
        self.assertEqual(selector(events, 1), (0, 1, 'E145'))
        self.assertEqual(selector(events, 2), (2, 3, 'E4BE'))
        with self.assertRaisesRegex(ValueError, 'round'):
            selector(events, 3)

    def test_extracts_explicit_events(self):
        rows = collect_samples(trace_events())
        self.assertEqual(len(rows), 22)
        self.assertEqual(rows[7], (1, 0, 7, 55, 1, 1, 160, 3))

    def test_rejects_missing_or_ambiguous_event(self):
        events = trace_events()
        with self.assertRaisesRegex(ValueError, 'speed-loader'):
            collect_samples([event for event in events if not (event[1] == 'E3DE' and event[2] == 3)])
        with self.assertRaisesRegex(ValueError, 'duplicate'):
            collect_samples(events + [events[0]])
        with self.assertRaisesRegex(ValueError, 'outside'):
            collect_samples(events + [(1000, 'E3CA', 50, events[0][3])])
        index, pc, frame, raw = events[3]
        changed = bytearray(raw)
        changed[0xe2] ^= 1
        events[3] = (index, pc, frame, bytes(changed))
        with self.assertRaisesRegex(ValueError, 'random'):
            collect_samples(events)

    def test_reports_first_named_boundary_difference(self):
        original = bytearray(413)
        original[:10], original[10] = b'CMJSESSION', 129
        native = bytearray(original) + bytes((255,))
        native[10] = 130
        native[22], native[23] = 5, 6
        with self.assertRaisesRegex(ValueError, r'field_round, field.ball_x: native 5, original 0'):
            compare_checkpoints(native, original)
        with self.assertRaisesRegex(ValueError, 'length'):
            compare_checkpoints(native[:-1], original)
        native[0] = 0
        with self.assertRaisesRegex(ValueError, 'magic'):
            compare_checkpoints(native, original)

    def test_nmi_inputs_follow_recorded_random_state(self):
        events = []
        entry = bytearray(2048)
        entry[0xe1:0xe4] = bytes((250, 3, 4))
        previous = bytes(entry)
        rows = []
        for frame in range(22):
            before = bytearray(previous)
            index = before[0xe1]
            before[0x300 + index], before[0x700 + index] = 5, 6
            before[(0x780 + index) & 2047] = 7
            after = bytearray(before)
            after[0xe1:0xe4] = bytes(((index + 1) & 255, frame, frame + 1))
            tail = bytearray(after)
            tail[0xe2] >>= 1
            for offset, (pc, ram) in enumerate((('C9C5', before), ('C9E8', after), ('E3CA', after),
                                                 ('E412', after), ('E48F', tail))):
                events.append((frame * 5 + offset, pc, frame, bytes(ram), 1))
            rows.append((1, 0, 0, 0, 1, 1, 160, 3))
            previous = bytes(tail)
        result = collect_nmi_samples(events, entry, previous, rows)
        self.assertEqual(len(result), 22)
        self.assertEqual(result[0], (1, 0, 5, 6, 7, 1, 1, 1, 160, 3))
        with self.assertRaisesRegex(ValueError, 'missing NMI'):
            collect_nmi_samples([event for event in events if not (event[1] == 'C9E8' and event[2] == 3)], entry, previous, rows)
        broken = list(events)
        i, pc, frame, ram, carry = broken[5]
        modified = bytearray(ram)
        modified[0xe1] ^= 1
        broken[5] = (i, pc, frame, bytes(modified), carry)
        with self.assertRaisesRegex(ValueError, 'random state'):
            collect_nmi_samples(broken, entry, previous, rows)


if __name__ == '__main__':
    unittest.main()

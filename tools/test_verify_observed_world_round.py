import unittest

from verify_observed_world_round import collect_samples, compare_checkpoints


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


if __name__ == '__main__':
    unittest.main()

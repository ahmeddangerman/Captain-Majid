# Defender selection design

## Goal

Reproduce the Japanese ROM routine at `$E4D7` that selects nearby opposing field-player IDs into the ordered defender list. This supplies a previously caller-chosen input to the field-contact calculation without inventing player stats or contact-table values.

## Verified behavior

- `$05FB == 0` scans player IDs 12 through 21 in ascending order and returns at most five IDs.
- `$05FB == 11` scans IDs 1 through 10 in ascending order and returns at most four IDs.
- A nonzero player-record byte at offset `+10` skips that player.
- Player coordinates come from offsets `+6` (X) and `+8` (Y); the ball position is `$0635/$0637`.
- Both absolute coordinate differences must be strictly less than the radius passed in A. The check is a square around the ball, not a circular distance.
- The routine clears `$0600` before scanning and writes the selected IDs in scan order beginning at `$0601`.
- Verified callers use radii 7 and 10. The native function accepts any byte-sized radius to preserve the routine's interface.

## Native interface

Add an explicit input containing the possession side, ball coordinates, radius, and 20 field-player records ordered as IDs 1–10 followed by 12–21. Each record contains X, Y, and the raw unavailable/status condition as a boolean. Return an ordered vector of player IDs. Reject possession sides other than 0 and 11.

The selector does not load player attributes, calculate contact thresholds, update match state, or choose a caller-specific radius. Those remain separate tasks.

## Verification

Unit tests cover both team directions, ordering, strict boundaries, unavailable players, each side's capacity, zero radius, and invalid-side rejection. A seeded differential tool runs `$E4D7` in the verified Japanese ROM and compares the exact selected ID list against the native batch interface.

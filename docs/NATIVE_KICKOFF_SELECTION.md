# Kickoff team and receiver selection

The original post-goal setup is more than a possession flip. Its DB4B..DB61
section can select either team based on the opponent-team byte and the high bit
of the captured random byte; opponent team3 selects side0. Both original side
fields receive the same selected side. The native pure function
`select_kickoff_side(opponent_team, random)` computes that value without advancing
randomness or writing session state.

Later, DAF5..DB0B selects a receiver from five version-specific field-player
slots, using random&7 and subtracting5 once for values5..7. The native function
`select_kickoff_receiver(side, random, candidates)` takes the table explicitly,
validates sides0/11 and candidate slots1..10, and returns the selected player ID.
The approved Japanese ROM table is observed privately; the native library does not
embed the ROM data. These functions do not yet initialize a kickoff session.

The native tests each failed with an unimplemented stub before passing. The bounded
original CPU observer executes every instruction in the two sections and matches
65,536 side-selection inputs (all team/random bytes) and512 receiver inputs (both
sides/all random bytes) using the approved Japanese table. Private evidence:
`../Captain-Majid-analysis/kickoff-selections-final/verification.json`.

```
PYTHONPATH=tools python3 tools/check_kickoff_selection_original.py \
 /private/Japanese-base.nes /private/new-output-directory
```

The original earlier reset and formation setup, displayed goal/menu, player
positions, and full kickoff continuation have not been connected or verified by
these calculations. The game is not yet a complete playable match. CMake definitions
are updated but this executor lacks CMake.

## Saved receiver handoff (2026-10-29)

After the prepared formation is placed, `prepare_session_kickoff_receiver`
selects the original receiver using an explicit random byte and five supplied
candidate slots. Phase62 saves the receiver separately; the current carrier and
ball position remain unchanged. This stops before original DB0B ball positioning
and DB0E presentation, so neither is assumed to have happened. The earlier reset
and side selection remain outside this prepared boundary.

Snapshot28 (`S`, 370 bytes) appends a presence byte and receiver byte. Readers
accept older snapshots including version27; version27 cannot encode phase62.
The development CLI accepts `--prepare-kickoff-receiver RANDOM C1 C2 C3 C4 C5`.
The caller must obtain content from the approved private ROM or other authorized
source; neither the ROM nor derived content is distributed here.

`tools/check_kickoff_receiver_session_original.py` runs the original
DAF5..DB0B instructions for both sides and every random byte, then compares
all represented serialized session bytes with the native transition. The 512
cases use previously prepared formations; this does not verify a complete
kickoff or a playable match. Private evidence: `kickoff-receiver-session-first`.

## Saved initial ball placement (2026-10-29)

After receiver selection, `prepare_session_kickoff_ball` executes the represented
DB0B/E6EC behavior: copy the current carrier's integer coordinates to the ball
and calculate its field cell. The current carrier remains side+8; the separately
selected receiver becomes owner only after the still-unimplemented presentation.
Session phase63 saves the result before presentation36. Snapshot29 (`T`) retains
the 370-byte layout, adds phase63, and reads older versions including `S`.
The CLI operation is `--prepare-kickoff-ball`.

Original ROM comparison covers 256 saved transitions: both sides and 128 interior
positions each, including edges. Every instruction through DB0E and the called
routine executes. The original retains fractional ball bytes; these are not yet
represented in the session and are not claimed as native parity. State validation
currently restricts this kickoff handoff to positions inside the field. Earlier
reset, presentation, ownership transfer and live gameplay remain unimplemented.
Private evidence: `kickoff-ball-session-interior`.

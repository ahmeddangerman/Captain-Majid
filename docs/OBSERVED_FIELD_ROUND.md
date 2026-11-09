# Evidence-fed field-round comparison

`--advance-observed-world-file PRIVATE_EVENTS` is a separate development operation for a saved field-round entry. It consumes 22 caller-supplied pre-poll observations. For each record it sets the represented random bytes to the supplied values, optionally applies one explicitly observed control-travel step, then runs the existing input and player-processing step. The existing `--advance-world` and `--advance-world-file` operations remain unchanged.

The private text file begins with `CMJWORLDOBS1`, followed by exactly 22 records of eight decimal bytes:

```text
HELD PRESSED RANDOM_FIRST RANDOM_SECOND TRAVEL_PRESENT TRAVEL_HELD SPEED_LOW SPEED_HIGH
```

`TRAVEL_PRESENT` must be 0 or 1; when it is 0 the remaining travel fields must be zero. The speed is the observed loaded word, low byte first. The operation validates the complete file before changing the session, preserves the existing early menu boundary, and rejects trailing data. It does not generate random values, frame timing, sound, presentation, or additional unrecorded travel events. It supports at most one supplied travel invocation before each poll; other schedules require a different representation.

`tools/verify_observed_world_round.py` accepts a private trace directory, pack, executable and empty output directory. The trace must contain the instrumented original's `instructions.txt` and indexed 2,048-byte RAM captures at E407, E3CA/E3DE/E406, E412 and E145 with frame IDs. It checks event ordering, constructs the private evidence file, runs the native operation and reports the first differing represented field at the field-round boundary. Output must remain outside the Git checkout.

Four directional original-emulator traces from one startup each matched 322 original-derived bytes and canonical session context after 22 polls with these explicitly supplied events. Those comparisons do not establish a general live scheduler, audio behavior, other match contexts or a complete playable match.

## Calculating random from captured NMI inputs

`--advance-observed-nmi-world-file PRIVATE_EVENTS INITIAL_INDEX` provides a second comparison mode. Its file begins with `CMJWORLDNMI1` and contains exactly 22 records:

```text
HELD PRESSED PLAYER_BYTE SOUND_BYTE TAIL_BYTE CARRY TRAVEL_PRESENT TRAVEL_HELD SPEED_LOW SPEED_HIGH
```

The caller supplies the three bytes sampled by the original random routine, its incoming carry, and any observed travel invocation. `CARRY` and `TRAVEL_PRESENT` must be 0 or 1; unused travel fields must be zero. The native random kernel calculates the next random bytes and index before each poll; any target-selection change to the first random byte feeds into the next step. The initial index is an explicit byte argument. The final index is printed separately and is **not** stored in the existing session snapshot, so a caller must retain it to continue a comparison. This mode does not derive those memory samples, carry, or event timing, and it does not execute NMI, audio or presentation.

`tools/verify_observed_nmi_world_round.py` accepts the same private paths as the observed-world verifier. It additionally requires captured C9C5/C9E8 random boundaries and E48F player-step boundaries, checks random-state continuity, and compares the final original-derived session fields, canonical context and random index. In four direction traces from the same startup, the calculated sequence matched the original after 22 polls; these are bounded emulator observations rather than a general scheduler.

## Observed NMI/control poll and later round selection

`--advance-observed-nmi-poll PRIVATE_EVENTS INITIAL_INDEX` composes one recorded random invocation, optional travel invocation and control input poll in that order. The file uses header `CMJPOLLNMI1` and exactly one record with the same ten fields as `CMJWORLDNMI1`. Invalid flags, unused travel inputs or trailing data are rejected before a new session is captured. The final random index is printed separately and is not included in session snapshots. This is a bounded evidence-fed transition, not an NMI/frame scheduler.

`tools/verify_observed_nmi_poll.py` accepts a private trace directory, private pack, executable, private output directory and expected poll count. It checks event order, reports the first differing poll and represented field, and checks the final random index. Fifty-five control polls in one original-emulator run matched 322 original-derived bytes and canonical context, but each comparison was independently seeded from its original entry boundary.

Both field-round comparators accept `--round N` to select a one-based round in a longer trace. Five independently seeded rounds in the same startup trace matched the represented state and random index; the first four reached control and the fifth reached a two-defender encounter. Clock intervals between those rounds have not been continuously replayed. Original instruction traces show NMI/random and travel events interleaved with both clock and stamina work; their schedule remains explicit and unsupported by the existing monolithic clock transition.

# Evidence-fed field-round comparison

`--advance-observed-world-file PRIVATE_EVENTS` is a separate development operation for a saved field-round entry. It consumes 22 caller-supplied pre-poll observations. For each record it sets the represented random bytes to the supplied values, optionally applies one explicitly observed control-travel step, then runs the existing input and player-processing step. The existing `--advance-world` and `--advance-world-file` operations remain unchanged.

The private text file begins with `CMJWORLDOBS1`, followed by exactly 22 records of eight decimal bytes:

```text
HELD PRESSED RANDOM_FIRST RANDOM_SECOND TRAVEL_PRESENT TRAVEL_HELD SPEED_LOW SPEED_HIGH
```

`TRAVEL_PRESENT` must be 0 or 1; when it is 0 the remaining travel fields must be zero. The speed is the observed loaded word, low byte first. The operation validates the complete file before changing the session, preserves the existing early menu boundary, and rejects trailing data. It does not generate random values, frame timing, sound, presentation, or additional unrecorded travel events. It supports at most one supplied travel invocation before each poll; other schedules require a different representation.

`tools/verify_observed_world_round.py` accepts a private trace directory, pack, executable and empty output directory. The trace must contain the instrumented original's `instructions.txt` and indexed 2,048-byte RAM captures at E407, E3CA/E3DE/E406, E412 and E145 with frame IDs. It checks event ordering, constructs the private evidence file, runs the native operation and reports the first differing represented field at the field-round boundary. Output must remain outside the Git checkout.

Four directional original-emulator traces from one startup each matched 322 original-derived bytes and canonical session context after 22 polls with these explicitly supplied events. Those comparisons do not establish a general live scheduler, audio behavior, other match contexts or a complete playable match.

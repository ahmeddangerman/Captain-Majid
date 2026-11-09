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

## Ordered clock interval and continuous captured control chain

`--advance-observed-nmi-clock PRIVATE_EVENTS INITIAL_INDEX` is a bounded comparison operation for `control_clock_pending`. It performs the logical normal clock update first, then consumes explicit ordered records in `CMJCLOCKNMI1`:

```text
R PLAYER_BYTE SOUND_BYTE TAIL_BYTE CARRY
T HELD SPEED_LOW SPEED_HIGH
S
E
```

`R` calculates one random step using captured memory bytes and carry; `T` applies one captured travel invocation; `S` marks the observed goal-area check and field-cycle update; `E` marks stamina processing. Exactly one `S` followed by one `E` is required. The original clock's normal update and each marked logical kernel are atomic in this comparison; arbitrary interruption inside those kernels is not modelled. Clock warning, added-time, period-end, and goal-area branches are rejected. The random index is explicitly supplied and returned separately, and no session snapshot format changed. Caller-supplied event order and speed are evidence, not a generated scheduler.

`tools/verify_captured_control_chain.py` takes private `--trace-directory`, `--pack`, `--exe` and `--output` paths plus `--expected-polls` and `--expected-rounds`. It verifies individual input polls and then feeds each native result into the next poll, clock or field round. It compares all represented fields and random index at each boundary and names the first mismatch. It requires unambiguous observed schedule and stamina boundaries and currently stops at the supported E4BE encounter; it fails closed for unrecognized trace shapes.

One original-emulator startup trace continuously matched 55 control polls, five normal clock intervals and five 22-player field rounds, reaching a two-defender encounter: 65 checked native boundaries, each against 322 original-derived bytes, canonical context and random index. The first interval has a random call between schedule and the stamina write; the remaining four have the stamina write before that call. Original-derived events remain supplied, and this single startup does not establish general AI, a frame scheduler, unobserved clock paths or a playable match. The existing separate phase317-to-pass-to-resumed-control replay remains a different sequence; the two have not been connected.

## Captured encounter-to-menu wait

`--advance-observed-nmi-sequence PRIVATE_EVENTS INITIAL_INDEX` accepts `CMJNMISEQ1` records using the same `R` and `T` syntax as the clock format, without `S`/`E`. It advances only captured random and travel invocations at a field-contact or human action-menu boundary, returns the index separately, and performs no presentation, input polling or frame scheduling. Unknown session phases, milestone records and malformed values reject.

The continuous control-chain verifier optionally accepts `--menu-boundary-frame FRAME` and `--menu-end-frame FRAME`. It checks the captured random/travel order around the acknowledged action-menu entry and through the neutral menu wait. One original startup matched 68 chained native boundaries: the previously described control-to-encounter sequence, 13 random/travel pairs before the menu, the existing menu entry, and 184 random/travel pairs during neutral menu wait. The supplied travel calls were neutral in this capture. A separate original control probe from the same saved state showed that down followed by confirm opens the pass target screen; this choice and later defender handling have not yet been integrated into the continuous native replay.

## Captured human-menu polls

`tools/verify_captured_menu_polls.py` accepts private trace, source session, pack and output paths, the separately retained initial random index, expected poll count and expected action. It chains every native result forward, supplies recorded button edges and NMI/travel events in observed order, and compares both input and result boundaries, including random index. It reports the first differing poll and field. The startup control-chain verifier prints the final random index for this continuation.

From the 68-boundary startup/menu trace, a second original trace restored the same state and selected ordinary pass with left and confirm. Ninety-five consecutive native menu polls matched both original input and output boundaries, including nonzero travel while left was held; the confirmed action was 1. Random, input and travel events remain recorded inputs, and presentation is acknowledged. Confirming the action opens a separate pass-type submenu; its choice and the subsequent target screen are not yet continuously replayed.

## Ordinary pass target after acknowledged submenu

The saved session operation `--begin-ordinary-human-pass-target` initializes phase331 from a confirmed ordinary side-zero pass. It explicitly acknowledges the ordinary pass-type submenu; it does not simulate that submenu or presentation. The original-backed initializer refreshes ball position from the carrier, computes the cell and cursor, and sets mode/subtype. Phase331 uses the existing extended session format and presently supports initialization only: target movement, recipient selection and defender continuation are not yet generalized.

`tools/verify_captured_pass_target.py` accepts a private original trace, prior native menu checkpoint, random index, pack, executable and output directory. It validates the confirmed action and the observed submenu confirmation edge, follows captured random/travel events to the target boundary, then compares the native target checkpoint and index. In the captured startup it matched 571 supplied NMI/travel pairs and the original target state at cell114. It acknowledges the original submenu rather than running its menu logic. `tools/verify_ordinary_pass_sessions.py` independently compared 16 complete saved target checkpoints over eight carrier positions in the startup and an earlier original encounter. This does not connect the startup encounter to the separately verified phase317 pass sequence or prove a playable match.

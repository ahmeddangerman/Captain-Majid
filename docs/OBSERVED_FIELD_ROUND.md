# Evidence-fed field-round comparison

The same 22-slot, event-fed field-player kernel can now follow a supported returned-shot dispatch into phase363. The original live startup matches at $C618; captured event order is supplied rather than generated. `--advance-observed-field-movement` and the older pass-named command refer to the same operation. The following shot-flight interruption is not yet modeled by this phase.


At the guarded returned-shot preparation boundary, `M`, `L` and `C` samples respectively supply observed motion, last-carrier-cell and defender-cursor reset bytes. `C` currently accepts only the observed reset to zero. These preserve captured order and do not emulate presentation. See CLI_HANDOFF for the evidence and bounds.

At a guarded returned-encounter boundary, explicit `M`, `F` and `D` records can supply observed motion, facing and display-parameter bytes before opening the shared action menu. `D` is an observed presentation sample, not a renderer. The original cursor residue remains preserved. See CLI_HANDOFF for coverage.

For returned possession with a retained pass-defender cursor, `tools/verify_returned_control_chain.py` accepts private source, trace, pack and executable paths and compares captured poll, clock and world boundaries. The `--advance-observed-ordinary-return KIND FILE INDEX` development operation uses `poll`, `clock` or `world`; it preserves the cursor in separate phases rather than broadening older guarded states. It takes captured NMI/travel ordering and does not generate a scheduler. See CLI_HANDOFF for coverage and limits.

For an ordinary-pass dispatch, `--advance-observed-pass-movement FILE INITIAL_INDEX` accepts the same `CMJNMISEQ1` random/travel records interleaved with exactly 22 ordered `P ID` records, ID 0 through 21. This records the player order; IDs 0, 11 and the carrier are skipped by the field-player kernel. It finishes at the saved pass field-movement boundary and reports the final random index. It does not generate event timing. See CLI_HANDOFF for comparisons and bounds.

The ordinary-pass defender trace can additionally feed explicit presentation samples `M VALUE` (motion request) and `F VALUE` (facing) at the guarded prepared-pass boundary. These copy observed bytes; they do not run presentation. First-defender preparation also requires an explicitly observed ball variant, and comparison requires the direct caller's supplied table row. See CLI_HANDOFF for coverage and limits.

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

## Ordinary target cursor movement

`--poll-ordinary-human-pass-cursor HELD PRESSED` supports movement in phase331. The lowest held direction bit selects delta +12, -12, +1 or -1; movement outside the linear range 0–239 is rejected by retaining the previous cursor. A ±1 step may cross a 12-cell row boundary. Pressed bits below 64 do not change this movement; A/B confirmation and cancellation currently reject atomically. When movement occurs, the operation reports a cursor visual update; the original UI/frame wait is acknowledged rather than simulated.

`tools/verify_ordinary_pass_cursor_original.py` compared this arithmetic with original D80F..D832 in 122,880 controlled cases: all 240 cells and 256 held bytes across two independent target RAM seeds. `tools/verify_ordinary_pass_cursor_sessions.py` compared 160 complete original/native cursor-poll checkpoints across two encounter contexts and representative edges/direction combinations, with UI waits acknowledged.

`CMJNMISEQ1` also supports `M VALUE` only at phase331: it sets an explicitly captured control motion-request byte. It is an observed presentation-state sample, not a simulated animation or scheduler. `tools/verify_captured_target_polls.py` consumes a private original trace and a previous target checkpoint, preserving captured random/travel event order and inserting recorded motion samples at observed boundaries. One original-emulator trace matched 55 consecutive target polls, including the cursor moving 114 to 126 and one supplied motion sample. This is still a captured-event replay, and confirmation, recipient selection and defender continuation are separate unsupported steps.

## Preserving pass-target candidates

After a cursor actually changes, the original scans field players in ID order, skipping the carrier and both goalkeepers, and retains at most four players in the target cell. For a side-zero pass, eligible teammates take priority; opponents are included only when no teammate has been found. The scan records the count and teammate count separately. `select_ordinary_pass_candidates` matched 2,048 constructed original CPU cases across two target seeds, and the resulting prefix also matched 106 moved polls within 160 full original/native cursor checkpoints.

Phase331 snapshots now use version148 when candidate context is available. A new target is marked unscanned; a cursor move stores the scanned prefix and its teammate count, and later neutral input preserves that snapshot even if player positions change. Version146 target snapshots still load with candidate context explicitly unknown; a later cursor move produces a new scan. The initial original scratch candidate list can be stale, so an unscanned context does not claim that list is empty. Candidate confirmation, recipient selection, multi-candidate chooser and the presentation engine are not yet implemented. The original scan verifier accepts private ROM and target RAM paths: `tools/verify_ordinary_pass_candidates_original.py`.

## Ordinary target confirmation

`--confirm-ordinary-human-pass-target` acknowledges a captured A-only confirmation with no simultaneous cursor movement. A known unscanned initial target, or a scanned target containing no teammates, confirms a free pass (recipient255). A scanned target containing exactly one teammate confirms that player. Multiple teammates reject because the original enters a separate recipient chooser; an older version146 target with unknown candidate state also rejects until a move performs a known scan. B/cancel and combined B/A remain unsupported; original controlled observations show B takes precedence. The operation saves phase332 and an explicit recipient in version149, with the cursor as the confirmed destination. It does not execute the chooser, defenders, passing animation or flight.

`tools/verify_ordinary_pass_confirmation_sessions.py` compared six complete original/native confirmation checkpoints for empty, opponent-only and single-teammate targets in two encounter contexts, and checked that two multi-teammate chooser boundaries reject atomically. The captured startup replay also matched 25 further target polls and an A confirmation of the opponent-only cell126 as free recipient255 using captured event inputs: `tools/verify_captured_target_polls.py --confirm`. No defender decision or passing continuation is implied.

## Explicit defender choices and ordinary pass preparation

`--begin-ordinary-pass-defenses` acknowledges entry to the defender-choice period after a confirmed target; `--record-ordinary-pass-defense DEFENDER ACTION SUBTYPE` records one externally supplied choice in selected-defender order. The current general path accepts action1/subtype0 only, with a nonempty opposing prefix. The engine does not choose or infer defender AI actions. `--prepare-ordinary-pass` then uses the existing profile, stamina and power kernels for an ordinary side-zero pass in field situation0. Phases333–335 and version150 retain the recipient, candidate snapshot and recorded prefix. The path stops before defender processing.

`tools/verify_captured_pass_defenses.py` accepts private trace paths and caller-supplied instruction-boundary indices. It follows the original order of the pass-mode change, captured random/travel events and defender choice writes, then optionally compares the original attacker-prepared boundary. The startup trace matched two supplied choices and attacker preparation: owner9 stamina733?713, power12. Candidate scratch is compared against its earlier target boundary because the original reuses those bytes during defender selection. `tools/verify_ordinary_pass_defenses_sessions.py` also matched 24 earlier original three-defender cases at both preparation entry and prepared state. Those comparisons establish the bounded composition with supplied choices, not a general defender AI or a playable match.

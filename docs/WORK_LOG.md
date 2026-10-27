# Development log

This is the running reference for work between chat updates. Entries distinguish implemented code, observed original behavior, and remaining gaps. Source commits stay local unless publication is requested; ROMs and derived content remain private.

## Current position

- **Full port: roughly 7%.** This is a subjective planning estimate, not a measured percentage of recovered code. There is no playable match yet.
- **Available:** native shot calculations, private content loading, a resettable shot laboratory, and an optional SDL2 scene with placeholder graphics.
- **Current milestone:** supported goal/save scenes in both directions, with derived routes and explicit keeper recovery.
- **Next engine work:** live ball-flight contacts, player movement and the phase after each shot.
- **Next major milestone:** coherent persistent match state and supported phase transitions, with explicit simulation randomness for future online play.
- **Suggested model:** Sol 6.1 High for original-code tracing and C++ integration. This is a judgment about the task, not a benchmark result.

## 2026-10-27 — completed local checkpoints

| Commit | Change | Evidence and limits |
| --- | --- | --- |
| `e6a6f6d` | Corrected prepared defender contacts to preserve power in the selected-defender caller. | Seven original loop observations; production compilation. Automated native regressions were not run for this increment. |
| `a1cbbf0` | Added continuous original attacker-to-keeper tracing. | Four side0 observations reach the keeper comparison boundary. Host frame/display acknowledgements are explicit; full outcome scripts and side11 remain unresolved. |
| `9a7c423` | Added private data pack loading and native shot laboratory. | Linux production builds and normal CLI usage. Data stays outside Git; scene resets between shots. |
| `004434a` | Added optional SDL2 shot scene using the same calculation. | Linux production compilation and software image capture. Real window/input, Windows, artwork, audio and a full match remain unverified or unfinished. |
| `b8d19e0` | Derived shot distance, target and loose-ball route from private replaceable content. | Original observations, production build, normal lab usage and later fresh static review. |
| `b7ee43d` | Added ordered elapsed keeper recovery. | Original caller observations, native build/usage and fresh static review; full match timing remains pending. |
| `a7bd1f6` | Extended original tracing through goal/punch/rebound handoffs and corrected the scene's keeper-stage count. | Four continuous original observations, native build/usage and fresh static review; explicit presentation acknowledgements remain necessary. |
| `2d2ad7c` | Added both native shot directions, controlled original catch observation and separate flight-contact records. | Mirrored slots/energy and explicit input masks reviewed statically; movement selection remained separate. |
| `61db9dd` | Connected one later flight-contact burst and private pack version3. | Crowded power33→21/ratio22 inspected; terminal handlers remain pending. Make/CMake builds and static review recorded. |
| `7460549` | Selected candidates from one position/status snapshot. | Cell/cap/borrow rules traced in the original, native builds/usage and fresh static review recorded. Continuous movement remains separate. |

## 2026-10-27 20:25 UTC — resumed work

- Resumed the uncommitted shot approach increment: original distance adjustment, destination cell, final target position, and loose-ball routing.
- Eight isolated original observations and four continuous shot observations are recorded privately. These are research observations, not a native differential suite.
- Version2 private packs add geometry tables; the reader retains version1 support.
- The independent reviewer stopped at a model usage limit. Its partial review reported no finding, but **the review did not finish**. Author review and production builds are being recorded separately.
- The temporary build tools/SDK from the prior runtime are absent after resumption. The repository and private analysis files remain available; rebuilding the default native targets needs only the installed C++ compiler and Make.
- Next: finish the approach checkpoint, record its commit, and inspect the original goalkeeper recovery caller before connecting elapsed time.

### Shot approach checkpoint

Local source commit: `b8d19e0`.

- `make -j2 all` completed successfully after resumption, with no compiler warnings in the captured output.
- Normal CLI usage loaded both pack versions. Version2 produced a distance45 goal aimed at(204,124), and a distance90 loose-ball diversion aimed at(196,124). Version1 retained its explicit scene behavior.
- Author review covered route gates, subtype handling, separate random stages and pack layout. Independent review remains incomplete due to its usage limit. No automated tests were added or run.
- Keeper recovery research is next; this does not change the rough full-port estimate yet.

### Keeper recovery — in progress

- Original inspection identified wrapped byte subtraction, stun precedence, and a single penalty reduction per update.
- Implementation follows the [recovery plan](superpowers/plans/2026-10-27-keeper-recovery.md). The scene will accept an explicit pre-shot elapsed step; a complete match clock is still pending.

### 2026-10-27 20:33 UTC — recovery implemented

Local source commit: `b7ee43d`.

- Added a pure keeper/shared-state recovery API and explicit ordered recovery steps in the resettable native scene. Timer, penalty, duration and stun now reach the corresponding comparison/outcome/status inputs together.
- Seven isolated original observations and two continuous original shot observations are recorded privately. The nonzero-state shot preserves the real1,1,2,1sequence and reaches keeper comparison with penalty29.
- `make -j2 all` completed successfully. Normal CLI usage produced penalty29/duration1 for that explicit sequence; a still-stunned keeper diverted before comparison; a later recovered keeper reached comparison. No automated tests were added or run.
- A fresh independent read-only review of both approach and recovery reported no actionable correctness findings, including static inspection of the original routines. It did not run tests or establish full-match equivalence.
- Full-port estimate remains roughly7%: these are stronger engine components, not a playable match. Next work is the original outcome and next-phase boundary.

### 2026-10-27 20:43 UTC — original goal/save handoffs reached

- Diagnosed the stalled original goal trace: omitted graphics-transfer consumption allowed its queue to overwrite script state. Added an opt-in, separately logged one-descriptor host acknowledgement; no original subroutine calls are skipped and full NMI/PPU/audio remain unmodeled.
- Four original observations now reach goal, punch or rebound handoffs before the next phase. Details and counts are in [continuous shot outcomes](CONTINUOUS_SHOT_OUTCOMES.md).
- The goal observation exposed an earlier scene input error: keeper selection replaces the field count with1 before comparison. Corrected that development scene input; general prepared API outcome inputs remain explicit.
- Production build, normal laboratory inspection and review are being recorded before the local checkpoint. This is not a playable match or a native full-state equivalence claim.

Checkpoint evidence: `make -j2 all` completed successfully. Normal interactive usage inspected all four native scenes, including goal count1/active0/kickoff4. Fresh independent static review found no actionable issues and checked the four original reports; no automated tests were added or run.

### Mirrored catch — current work

- The opposite-side original keeper choice now reaches a continuous catch handoff using a disclosed A+Right input mask. A controlled observation retains power33, ratio35 and keeper stamina980; the native scene now supports this mirrored calculation too.
- A crowded observation identifies four later flight contacts, reducing power33→21. These belong to a different caller from the initial selected-defender loop. The native scene does not yet reconstruct their movement/selection/scheduling.
- Added separate flight-contact records and an explicit button-mask option to make the original observations reproducible. Details are in [both directions and catch research](MIRRORED_SHOT_SCENE.md).
- The full-port estimate remains roughly7%. Catch/shot components and their evidence have improved; there is still no playable match or online service.

Checkpoint evidence: Production `make -j2 all` completed successfully. Normal usage inspected mirrored catch, side0 recovery and version1 defaults. Fresh independent static review found no actionable issues and checked both original reports. No automated tests were added or run.

### Later flight-contact calculation — implemented

Local source commit: `61db9dd`.

- Added one caller-supplied burst after approach and before loose-ball/keeper-status routing. It loads player/move data, applies later result1 loss, and leaves defender stamina unchanged.
- The crowded native catch now gives four contacts, power33→21 and keeper ratio22, with inspected catch/stamina fields agreeing with the original crowded observation. Live position-based selection and repeated flight-frame scheduling are still missing.
- Terminal later-flight results stop and report a pending handoff with no keeper calculation; their handler state is not presented as implemented.
- Private content version3 adds16bytes; versions1/2 remain readable. Original data/exports stay outside Git.
- Make and CMake production builds completed with no warnings/errors in their captured output; CMake tests were disabled. Normal usage covered crowded catch, controlled catch, older default packs, goal and a pending terminal boundary. No automated tests were added or run.
- Fresh independent static review found no runtime blocker and corrected one eligibility-table locator. Its two reference values were unchanged, so existing numerical observations remain the same. The reviewed exporter produced a new private11,833-byte pack.
- Next: select contacts from live positions and implement terminal/restart/recovery phase transitions in persistent state. Full-port estimate remains roughly7%, with no playable match yet.

### Position-snapshot flight selection — implemented

Local source commit: `7460549`.

- Added original field-cell encoding and selection from opposing position/status records, including scan order,5/4limits and the original subtraction borrow.
- Six isolated original observations are recorded privately, covering caps, status/cell filtering, threshold boundaries and the invalid-cell sentinel.
- The native scene now constructs its crowded layout and asks the selector for candidates before calculating the burst. Continuous movement/ball-step scheduling and terminal handler state remain pending.
- Details and evidence limits are in [flight selection](FLIGHT_SELECTION.md). No automated tests were added or run.

Checkpoint evidence: Make/CMake production builds completed with no warnings/errors. Normal usage retained crowded catch power21/ratio22 and controlled goal power224/kickoff4. Fresh independent static review reported no actionable defects, including cell/status filters, scan/cap/borrow rules and scene placement. Full movement and terminal phase state remain the next work.

### Terminal later-flight research — checkpoint complete

Local source commit: `824c253`.

- Two original continuous observations now stop correctly before the next phase when a contact wins the ball or knocks it loose. No keeper was reached, and the reports keep keeper outcomes absent.
- The paths differ from the initial defender handler: later code3 transfers possession for action0; code2 copies the defender's current coordinates and retains a different timing/active value.
- The native calculation still reports these as pending handoffs. The new [terminal flight observations](TERMINAL_FLIGHT_TRACES.md) give the next contract instead of guessing those writes.
- No automated tests were added or run. Full-port estimate remains roughly7%; persistent playable phases and online remain unfinished.

Checkpoint evidence: Two original outcome-mode traces and a separate keeper-mode trace reach witnessed pre-keeper boundaries without skipped calls. Native level5 usage still labels the result2 handoff pending. Fresh independent static review found no actionable issues and checked the saved reports. No native terminal handler or new test was added.

### Native position arithmetic — implemented

- Traced the next movement dependency: terminal possession changes also update nineteen field-player records, so committing only a new owner would leave inconsistent state.
- Added original raw16-bit coordinate addition/clamping, retaining fractional bytes and wrap order. Six original observations are recorded privately; the native query is stateless and does not advance the fixture.
- [Movement foundation](PLAYER_MOVEMENT_FOUNDATION.md) records the helper, exact boundaries and the still-pending speed/direction/target/status/timing integration.

Checkpoint evidence: Make/CMake production builds completed without warnings/errors. Normal batch usage produced the documented edge/wrap results; a native replica query left the fixture state and sequence unchanged. Fresh independent static review found no actionable findings. Six original observations are private; no tests were added/run. This helper does not yet advance full player movement.

## Resume here

1. Carry complete position words, targets/statuses and timing state into terminal flight resolution. Its player-control branch also invokes player movement, so owner/ball writes alone are insufficient.
2. Build on `advance_field_coordinate` to implement speed/direction, player updates and ordered flight steps, then the distinct terminal possession/recovery transitions.
3. Commit roster, ball, score and phase changes into one persistent match state before extending online replay beyond the development fixture.

Keep original data private, retain the explicit scheduling limits, and continue with Sol6.1 High for tracing/integration. Overall progress remains a rough7%, with no full match yet.

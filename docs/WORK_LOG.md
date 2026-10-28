# Development log

This is the running reference for work between chat updates. Entries distinguish implemented code, observed original behavior, and remaining gaps. Source commits stay local unless publication is requested; ROMs and derived content remain private.

## Current position

- **Full port: currently unmeasured.** The repeated7% guess has been retired following the user's correction. See [completion checkpoints](PORT_STATUS.md); historical entries below retain what was reported at the time. There is no playable match yet.
- **Available:** native shot calculations, private content loading, resettable shot lab, persistent field/clock phase-session labs, and an optional SDL2 scene with placeholder graphics.
- **Current milestone:** selected-shot/control/recovery and ordinary clearance handoffs are connected. Interrupted shots now select field or opposing goal-area restart setup. Actual restart setup, keeper/story and remaining shot/goal routes remain pending.
- **Next engine work:** connect selected keeper/story/boundary restarts and nonterminal approach/flight/keeper/goal routes; then continue goal-area/restart/keeper/story progression and live ball-flight/frame scheduling.
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
| `824c253` | Captured terminal later-flight player/recovery handoffs. | Two continuous original observations and fresh static review; native terminal handlers remain pending. |
| `28cd887` | Added raw16-bit coordinate stepping and stateless development query. | Original boundary observations, Make/CMake builds, normal queries and fresh static review. Full movement remains pending. |

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

### Resume evidence saved

- A further original run captured38movement entries with actual position words, target/status, speed, elapsed and direction. The data is private and gives concrete inputs for porting the next movement stage.
- Current source checkpoints are committed locally. ROMs, patches, extracted packs, images and original RAM/reports were not added to source control or pushed.
- The latest production build evidence is Linux Make/CMake without automated tests; native window/Windows and full match behavior remain unverified. The current result is an engine development scene with stronger shot/selection/movement components.

## Resume here

1. Connect the reward/contact-clock/status prelude to prepared terminal tails so callers do not need captured preprocessed state.
2. Implement recovery/player-control progression and live ball-flight/frame scheduling on the native movement phase.
3. Extend the persistent field state into atomic roster/ball/clock/score/phase commits before online match replay.

Keep original data private, retain explicit scheduling limits, and continue with Sol6.1 High for tracing/integration. Overall percentage is unmeasured; report achieved capabilities and remaining checkpoints.

## 2026-10-27 21:34 UTC — continuous work resumed

- User authorized successive tasks without routine approval pauses. Continue from movement into terminal transitions and persistent state while the session is available.
- Retired the unsupported overall7% figure and added observable [port completion checkpoints](PORT_STATUS.md). Historical percentages are not current measurements.
- Repository is on the existing isolated `work` branch, with prior source checkpoints committed. Private ROM/analysis and local build tools remain available. No tests will be added/run under the active developer constraint; production builds and research observations remain separately labeled.

### Player update implemented

Local source commit: `fa205cc`.

- Added full target-direction and coordinate stepping for a field player, including target/ball-cell stops, status gates, fractional positions and original byte arithmetic.
- Eight private original observations and normal native query outputs agree on the inspected position/direction/iteration fields. Production Make compilation completed without diagnostics; no automated tests were added/run.
- Working through the [native field phase plan](superpowers/plans/2026-10-27-native-field-phase.md) continuously. Next are profile speeds, positioning AI and the ordered roster update; independent whole-plan review follows integration.

### Native team phase and prepared terminal tails — checkpoint complete

- Added secondary movement profiles, formation/tactic target selection, and an ordered native field update. Private version4 content keeps original data separate and retains old pack readers.
- Added239-byte typed field snapshots and `captain-field-lab`, whose state persists between commands. Private snapshots/outputs remain outside Git and exclusive saves refuse existing files.
- Native advancement updates19players from the captured checkpoint; its represented239-byte state matches the original post-phase export. Prepared player-control and ball-recovery tails also match their respective prepared original snapshots in manual file/hash inspection.
- The recovery example initially used a checkpoint before clock/status processing. Using the actual post-prelude checkpoint agrees; no fixed elapsed/status correction was inserted into native logic.
- This is interrupt-free field-phase evidence. NMI/music/controller refresh, full reward/script timing and full-match state are not covered by that agreement. No automated tests were added/run.
- Whole-plan review and production build evidence are being completed; work continues into the next phase after that checkpoint.

Checkpoint evidence: Make/CMake production builds completed without warning/error lines. Interactive advancement retains positions across commands. One fresh independent static review found no material implementation defect; current capability summaries were brought up to date. Native/observed snapshot agreement is limited to represented state and the disclosed interrupt-free/prepared boundaries. No automated tests were added/run.

### Native contact clock and phase session — implemented

- Previous field-phase checkpoint is local commit `3e6d8e6`.
- Added contact elapsed/status rules, ordered keeper recovery and match-clock events, using private version5 content. Existing content versions remain readable.
- Added264-byte explicit phase-session snapshots and a persistent native command laboratory. Terminal contacts now run the prelude before continuing, and retain pending added-time or period-end boundaries without prematurely transferring possession.
- Nine original clock observations cover ordinary subtraction, low-byte warning behavior, zero steps, extra time and period end. Four264-byte represented session outputs match original exports in manual hash inspection: possession, loose-ball recovery, added-time pause and period-end pause.
- Production Make/CMake builds completed. Interactive usage refuses contact/movement at a paused clock boundary and resumes only through an explicit development command. No automated tests were added/run; no claim of live NMI or full-match equivalence.
- See [NATIVE_CONTACT_CLOCK.md](NATIVE_CONTACT_CLOCK.md) for exact boundaries, format, evidence and remaining gaps. Whole-plan static review follows; work continues into rewards and recovery/control progression.

Checkpoint review: One fresh independent static review found no material findings and checked original contact/clock branches, ordering, deferred continuations and bounded formats. Source is committed locally; no ROM/derived files or webpage changes are included. Work continues into rewards/recovery.

## 2026-10-28 — terminal reward integration and handoff

- Contact-clock/session source checkpoint is local commit `f611ecc`.
- Added native named-player experience accounting from private replaceable mappings/factors. Original low-byte scaling and16-bit saturation are preserved; experience does not occupy coordinate fields or silently update levels.
- Continuous original traces show one terminal reward grant for result3 and two for result2 through its shared recovery caller. The new explicit rewarded operation preserves those counts; added-time resume does not reapply them.
- Private content version6 and289-byte session version2 carry optional campaign/experience context. Earlier versions remain readable, with absent context explicit. No private ROMs, RAM, packs or snapshots are staged.
- Two represented289-byte final states from normal rewarded native commands match original exports in manual hash inspection. Nine direct original reward observations document gates/mappings/scaling/saturation. Interactive added-time resume retained its existing reward total.
- Production Make/CMake builds completed. No automated tests were added or run. Fresh whole-plan review follows; then save this checkpoint locally in response to the user's instruction to finish the task.
- Next resume point: integrate actual loose-ball recovery/player-control progression with phase-session state, then live ball-flight/frame/random scheduling. Full playable match and online match remain unfinished; overall port completion stays unmeasured.

Final checkpoint: One fresh independent static review found no material findings, including original reward byte arithmetic and mapping, saturation, grant counts, resume behavior, backward formats and atomic session updates. Make/CMake production commands returned0 without diagnostics; no automated tests were added/run. Current task is complete as a local source checkpoint. No public push, website change or private game-data upload was performed.

## 2026-10-28 — persistent recovery and prepared control tail

- Resumed from local source commit `92d87ce` under the user's request for the next step. Reused the existing isolated `work` branch and private tools/data; no public website changes.
- Corrected a research description: original9070 performs special-owner cleanup, not a random-generator call. Added a bounded original recovery observation tool with explicit host scheduler/frame/input/display acknowledgements.
- Split ball jitter from recipient selection so native field advancement occurs in the original order. Committed ownership updates preserve fractions, clock, experience and keeper role state; possession sound is an event.
- Added optional recovery context and bounded296-byte session3 snapshots, retaining versions1/2 with missing context explicit. Native `recover` and `continue-recovery` commands retain state and pause at restart, keeper-area and story gates.
- Eight original recovery observations and seven subsequent boundary observations agree with native represented saved state in manual hash inspection. Joined rewarded contact→recovery→prepared control usage retains clock452/experience2 and transfers owner14/side11 to player2/side0. Repeated recovery after control is rejected without mutation.
- Production builds and whole-plan static review are being finalized. No automated tests added/run; observations do not establish live NMI/full-match equivalence. See [NATIVE_RECOVERY_SESSION.md](NATIVE_RECOVERY_SESSION.md).

Review checkpoint: One fresh independent static review found no material implementation findings, including original byte inspection of keeper/story gates, special-owner cleanup and ball-variant initialization. Completed the planned9070 documentation correction in its remaining historical descriptions/comment and marked README's session2 reference as historical. Reviewer ran no code/builds/tests. Legacy session1/2 usage loads with recovery context absent and rejects recovery without mutating state.

Final checkpoint: Make/CMake production commands returned0 without warning/error lines after the final format guards. Fresh normal recovery/control command outputs retain the documented original snapshot hashes. Source saved locally; no private data or website changes included. No automated tests added/run. Next resume point is native player-control setup/actions, followed by explicit recovery restart/keeper/story handling and live frame/ball/random scheduling. Overall completion remains unmeasured.

## 2026-10-28 — field-carrier control setup

- Resumed from local source commit `c81afdd`. Extended original observation mode through E0DF→E145, stopping before live controller/frame logic and recording ordered presentation requests.
- Added control setup for both possession sides, ball/cell refresh from the carrier, decision/facing initialization and nearest available controlled-defender selection for opposing possession. Reused the recovery nearest-player primitive.
- Preserved original special-owner cleanup and the0469 high-byte write through the existing represented slot10. Clock values/ordinary experience totals and coordinate fractions are retained; script/ball flags are initialized as in the original setup.
- Added optional seven-byte control context and304-byte session4 snapshots, retaining readers1–3 with missing context explicit. Native `setup-control` yields phase9 control-ready, distinct from phase1 handler entry.
- Seven private original/native represented saved-state observations agree in manual hash inspection, including stale ball coordinates, count clearing, unavailable defenders, side normalization and both field edges. See [NATIVE_CONTROL_SETUP.md](NATIVE_CONTROL_SETUP.md).
- Production builds, joined-session usage and fresh whole-plan static review are being finalized. No automated tests added/run. Live inputs/actions, full frame timing, presentation and a complete match remain unfinished; overall completion stays unmeasured.

Final checkpoint: Make/CMake production builds returned0 without warning/error lines. Joined rewarded contact→recovery→prepared control→setup produces the same documented304-byte original control-ready hash. Repeated setup and legacy session3 without control context are rejected without mutating state. One fresh independent static review found no material findings and inspected original setup/selection bytes; it executed no code/builds/tests. Source saved locally, no public push or private data publication. Next: logical button/decision handling and carrier movement, with frame/clock scheduling still explicit work ahead.

## 2026-10-28 — logical input polls and controlled movement

- Resumed from local source commit `48e3873`. Added one staged post-yield input poll with explicit held/pressed masks, native per-poll flags/facing, defender cycling, decision timer and raw controlled movement.
- Preserved original priority rules: side0 heldB before movement, side11 pressedA over B, opposite axis bits choosing positive motion, full-speed diagonals, and no invented status gate for controlled movement.
- Imported private selector32 speed data in content7, distinct from team movement selectors33/34. Session5 adds the overwritten-on-poll motion byte; versions1–4 remain readable, including version4 setup checkpoints.
- Sixteen private original/native represented305-byte state observations agree in manual hash inspection. Repeated native usage from a version4 snapshot waits ten polls and steps on the eleventh, agreeing with the original timer0 result; another poll is rejected at the clock boundary without mutation.
- Added explicit clock10/menu11/AI12 pause phases. No clock/stamina/AI/menu progression or live frame/random schedule is inferred. See [NATIVE_CONTROL_INPUT.md](NATIVE_CONTROL_INPUT.md).
- Production builds and one fresh whole-plan review are being finalized. No automated tests added/run; full playable match remains unfinished and overall progress stays unmeasured.

Final checkpoint: Make/CMake production builds returned0 with no warning/error lines. One fresh independent static review found no material findings after inspecting original input/movement bytes, the bounded codecs/guards and existing observation/persistent-usage records. No repository code/builds/tests were executed by the reviewer; no automated tests were added/run for this increment. Source saved locally; no public push or private data publication. Next resume point is phase10 clock progression and phase11/12 menu/AI decisions, followed by stamina/other-player updates and live frame/random scheduling.

## 2026-10-28 — continuous work: ordinary control cycles

- User renewed all-in continuous authorization; work continues across successive engine checkpoints while the session is available. Remaining usage is not observable. Resumed from local commit `46b01b9`; no public push or website changes.
- Connected the post-input clock to keeper recovery, added-time/period boundaries, defending-keeper goal-area gate, human-side stamina regeneration/carrier drain and a22-poll ordered field round.
- Field rounds share button handling, preserve postpoll controlled-defender choice, region/RNG/target/speed/status ordering, and select nearby defenders every fifth cycle. Menu interruption retains its processed prefix; contact/special/story routes remain explicit.
- Added private content8 and314-byte session6 with optional scheduling/selected-prefix data; retained old readers and canonicalized unused selection slots. No original assets/data staged.
- Twenty-six original boundary observations agree with current native represented state in manual hash inspection. Corrected two derived assumptions using original evidence: the goal-area gate reads defending-keeper stun, and full E233 refresh clears the newly represented cosmetic motion flag.
- Normal native usage completes five walking cycles matching every original saved boundary: X91→109, clock452→447, then contact with defender16. No automated tests added/run. See [NATIVE_CONTROL_CYCLE.md](NATIVE_CONTROL_CYCLE.md).
- Final production builds and fresh whole-plan review are in progress. After this checkpoint, continue into pending encounter/action flow rather than stopping for routine approval. Overall port completion remains unmeasured; no complete playable match yet.

Checkpoint complete: Final Make/CMake production builds returned0 without warning/error lines. One fresh independent static review found no material findings and checked original order/energy/player-loop/interrupt bytes, bounded formats and atomic errors. Reviewer ran no builds/tests/simulation. No automated tests added/run. Source saved locally; continuing into encounter/action selection under the user's continuous authorization.

## Paused at user request — 2026-10-28

- Last completed local source checkpoint: `eeeef17` (ordinary control cycles). No push or private-data publication.
- The next action-menu increment is unfinished and uncommitted. Its spec/plan are `superpowers/specs/2026-10-28-native-action-menu-design.md` and `superpowers/plans/2026-10-28-native-action-menu.md`.
- Added bounded private menu observations in `tools/trace_action_menu.py`; original contact menu opens, direction+A confirms an ordinary shot, and a prepared8F72 observation returns stamina917/power45/subtype128 for the inspected input. These are research observations, not tests.
- In-progress source adds action context/phases18–20, menu opening/polling and prepared basic-shot functions in `action_menu.hpp/.cpp`. This source has not been built or reviewed. Session7/content9 codecs/export and lab integration are still missing; finish those before claiming the increment complete.
- Original presentation observations: opening resets display parameter/motion flag and0469high byte; contact entry clears facing, while free-menu entry preserves it. Cursor flash counter is preserved during opening. Special-move inventory, pass targets and opposing menus remain outside this increment.
- Resume from this marker and the existing uncommitted changes. Work stopped on the user's explicit pause; no further builds, tests, review dispatch or implementation should run until resumed.

## 2026-10-28 — native action menu and prepared ordinary shot

- Resumed from the user-requested pause at local commit `eeeef17`. Scope is the existing action-menu plan only; the user asked to stop again after this milestone. No public push or website changes.
- Added private content9 with original menu direction/icon/title/background tables, and session7 with optional nine-byte action context. Session7 is324bytes; older snapshots still load with the new context absent and reject menu entry rather than inventing action data.
- Connected ordinary side0 menu entry from a nearby-contact pause or a no-contact menu request, explicit logical direction/A polling, cursor flashing, keeper stun/penalty warning requests, and a prepared action0/subtype0 shot using the existing attacker/energy/power calculation.
- Byte-for-byte original/native represented session comparisons cover contact and free-menu entry, keeper-stun and keeper-penalty warning branches, first and repeated direction polls, same-poll direction+A confirmation, the prepared-shot boundary, and preservation of an out-of-range stale action byte until a menu selection becomes active. The observed shot result is stamina917, raw cost80, power45, subtype128. A controlled stamina60 invocation is rejected against raw cost80. These are bounded/manual CPU and CLI comparisons, not automated tests.
- Old session6 without action context loads and refuses to open the menu. Private v9 pack and all RAM/session observations remain under `/workspace/Captain-Majid-analysis`, outside Git.
- Updated the working roadmap with fresh expandable Arabic/English/Japanese translations, Arabic right-to-left text support, and quality-of-life work after the core loop is stable. The Arabic NES-style page now has five concise milestones and a dated one-paragraph progress update.
- One fresh static review found no Critical findings. It identified the stale action-byte boundary and a keeper-warning wording error; both are corrected. The stale-byte original/native 324-byte menu and selection checkpoints match.
- Final Make and CMake production builds returned0 without warning/error lines. No automated tests were added or run. The next gameplay task is defender decision/shot-flight continuation. Overall completion remains unmeasured.
- GitHub publication remains blocked: the managed network rejects GitHub API requests and `gh auth status` reports an invalid `GH_TOKEN`. I did not push code because I could not verify or set repository privacy. All project and website edits are local only.

## 2026-10-28 — reusable prepared-shot continuation

- Extracted `resolve_prepared_shot_continuation` as a public pure API for action0 shots whose attacker preparation is already committed. The caller supplies the post-cost roster and prepared power/subtype; the function does not debit the attacker again.
- `resolve_complete_prepared_shot` now performs its existing attacker preparation once, then delegates the existing field-contact, approach, flight, keeper and outcome sequence to the extracted function.
- Defender/keeper choices, random bytes, post-loop gates and optional approach/flight remain explicit. This does not add defender AI, a session command, persistent score/restart state or a full playable shot loop.
- `make -j2 all` completed with exit code0. No automated tests were added or run under the active developer instruction.
- Next: connect explicit defense decisions and downstream shot handoffs to persistent match state without inferring scheduling or charging attack stamina twice. Overall port completion remains unmeasured.

## 2026-10-28 — persistent selected shot defenses

- Resumed from `13bc9d5` under the user's continuous-work authorization.
- Added a session wrapper using saved post-cost roster, power/subtype and selected
  defenders. Choices/random samples remain explicit, comparison tables and
  interruption state derive from content/session. No attacker preparation/debit
  is repeated. Nonterminal per-contact presentation/clock scheduling is not added.
- Result2/3 joins existing rewarded terminal clock/recovery, including deferred
  added-time possession. Result0/1 pauses at phase21 before flight; result4 at
  phase22 before restart. Session8 keeps324bytes and legacy readers1–7.
- The CLI accepts strictly bounded defense-choice files, including an empty file
  for zero defenders. Invalid, reordered, duplicate and repeated operations reject.
- Focused regression first failed against an unimplemented stub. Its synthetic
  snapshot-offset and movement-content fixture errors were corrected. Initial
  selected-defense checks then passed.
- Fresh independent static review found two important issues: missing keeper-stun
  reset and stale random bytes at terminal clock entry. Both gained regressions
  observed failing before fixes. The fixes map all22 status fields and retain the
  last reached contact's random samples; empty selection retains saved samples.
- Ruling: use Make because CMake is unavailable in this executor. No CMake build
  result is claimed. Production/focused/full-suite results follow below.
- Next: saved approach/flight and keeper/outcome context, then remaining contact
  scheduling/restarts, full match loop and online integration. Overall completion
  remains unmeasured; no playable full match yet.

Research follow-on: three new original CPU observations expose failed-defender and
final-dispatch clock/status ordering; see SELECTED_CONTACT_TIMING_AUDIT.md. Native
phase21/22 are calculation checkpoints, not equivalent original-dispatch saves.
Nonterminal contacts before a later terminal result also need ordered scheduling.
This is the immediate next engine task before resuming approach/flight. Private
py65 was installed only under `/tmp/captain-research-deps` for these observations.

Final evidence: `make -j2 check` and `make -j2 all` returned0; captured build
logs contain no compiler warning/error lines. Manual CLI usage loaded the existing
v7 prepared shot and saved/reloaded phase21/version8 at power45 without a second
attacker debit; repeated and trailing-row invocations rejected. Three original CPU
observations and tool bytecode compilation completed. Reviewer findings were fixed
with failing-then-passing regressions; no other material review findings reported.
Daily website note updated separately; engine/private research remains local.

## 2026-10-28 — ordered selected-shot clock progression

- Added one-defender-at-a-time session progression with9095 status selection,
  interruption-before-status ordering, retained random samples and clock/keeper
  recovery before the next comparison. Final prefix/no-defender clock is an
  explicit elapsed step; no attacker preparation/debit repeats.
- Added phase23–27 and330-byte session9 progress. Both contact and final-clock
  added-time pauses save/resume without replay; period-end refuses continuation.
  This stops at original field dispatch, before rewards/possession/presentation.
- Added strict staged CLI operations and final RAM capture to the original tracer.
  Original pass/interruption observations match324 prior represented bytes after
  phase/version normalization. New progress metadata is not ROM-compared. Capture
  matches gameplay fields but differs in one facing byte written by the original
  script parser; no full-state capture equivalence is claimed.
- Regression failed with the unimplemented stub, then passed. Full native/CLI/
  adapter/loopback/reference suite and production build returned0. To avoid the
  Makefile repeatedly compiling all core sources, compiled those sources once
  into `/tmp/cm-staged-core.a` and supplied CORE to Make; diagnostics were clean.
- Fresh independent review found no material issues. Minor coverage gap deferred:
  direct terminal-contact added-time save/resume (the shared failed-contact path
  is covered). See NATIVE_ORDERED_SHOT_CLOCK.md for commands and evidence limits.
- Next: original selected-field dispatcher routes and presentation side effects,
  then approach/flight/keeper state. Overall port completion remains unmeasured.

## 2026-10-28 — terminal selected-result dispatch

- Continued timed phase27 through original83F5 field advance and selected-result
  handlers. Results2/3 grant exactly one selector2 reward. Action2 capture transfers
  possession and rejoins player control; action0 preserves ownership for animation.
  Result2 retains ball and selection at recovery entry; result4 stops at restart.
- Added phase28–30 and session10 marker A, retaining330bytes and readers1–9.
  CLI resolve-shot-result is atomic and rejects replay/nonterminal result0/1.
- Extended original observer with terminal dispatch-only mode. Four original/native
  dispatcher observations match324 prior represented bytes with phase/version
  normalization; six native progress bytes are excluded from the ROM comparison.
  No full-shot or script-facing equivalence is claimed.
- Regression failed against stub, then passed. Added direct terminal added-time
  save/resume coverage. Full suite and production Make build returned0 with clean
  compiler output, using an archive of the same core sources to avoid redundant
  compiles. CMake remains unavailable. Fresh review found no material issues.
- Manual capture→setup-control rejoins control-ready state with selected defender1.
  All four result handoffs reject repeated dispatch without source snapshot writes.
- See NATIVE_SELECTED_RESULT_DISPATCH.md for exact boundaries/evidence. Next:
  selected recovery with saved prefix/action context, defense animation/restart,
  then nonterminal approach/flight/keeper routes. Overall completion unmeasured;
  full playable match and online match remain unfinished.

## 2026-10-28 — selected blocked-ball recovery

- Workspace resumed after transient unavailability; source checkpoint was e10a078.
- Connected phase28 recovery with explicit full-prefix actions, including the
  original first action5 preference for an unvisited suffix at randomE2<64.
  Reused numeric zero-prefix recovery without repeating time, energy or rewards.
- Added retained-prefix recovered-owner31 and boundary32. Side11 possession may
  preserve the attacking selection; control setup clears it later. Ordinary
  prepared continuation rejoins control; keeper/story cases remain safely pending.
- Session11 marker B remains330bytes and reads versions1–10. CLI action files are
  strict; repeated recovery rejects before applying jitter again.
- Regression exposed missing new-phase validation, then passed after integration.
  Full native/CLI/adapter/loopback/reference suite and production Make build passed
  with clean compiler logs, using a same-source temporary core archive. CMake is
  unavailable. Fresh independent review reported no material findings.
- Four original/native recovery observations match324 prior represented bytes
  after phase/version normalization; new progress metadata is excluded. Both
  ordinary and suffix capture subsequently reached native control setup. See
  NATIVE_SELECTED_RECOVERY.md for limits and private evidence paths.
- Next: defense animation, selected keeper/story/boundary/restart handling, then
  nonterminal shot approach/flight/keeper integration. Overall port unmeasured;
  full playable match and online play remain unfinished.

## 2026-10-28 — capture clearance setup

- Traced E678: the action0 capture animation begins a clearance, switching active
  side and selecting a reflected target while retaining the original carrier.
  Connected through E6CC, stopping before DE96 flight scheduling. Music/task2 are
  explicit requests, with no extra clock, energy, rewards or generated randomness.
- Added private replaceable destination export/load, phase33 and one-byte saved
  distance context. Session12 marker C is332bytes and reads versions1–11.
- Regression failed against the throwing stub, then passed all32 synthetic
  sample/reflection combinations. Full native/CLI/adapter/loopback/reference suite
  and production Make build passed using a same-source core archive. Reviewer
  found no material issues; added its direct version11 compatibility check.
- Four original/native clearance setup observations match324 prior represented
  bytes with phase/version normalization, plus distance062B. Native progress
  metadata is excluded. Replay checks pass; no full-shot/renderer equivalence.
- Exporter was corrected to avoid importing py65 for a standard-library data
  export. Private content and raw observations remain outside Git. See
  NATIVE_CAPTURE_CLEARANCE.md for commands, exact boundary and evidence.
- Next: clearance DE96 flight/recovery, then restart/keeper/story and nonterminal
  shot followup. Overall port completion remains unmeasured; no full playable match.

## 2026-10-28 — clearance arrival race

- Connected DE96 bounded candidate race: nearest eligible players excluding old
  carrier/opposing player, active-first selector35 then other selector36 movement,
  first cell arrival wins. Preserves original zero-byte256-round wrap.
- Winner commits side/owner and high coordinates with retained fractions, stopping
  before DF0D presentation. Timeout stops DED4 before script52/recovery. No clock,
  stamina, experience or generated randomness is added; elapsed becomes1.
- Added private replacement speed export/load, phase34/35 and original race-byte
  context. Session13 marker D is336bytes and reads versions1–12.
- Regression failed against stub, then passed. Review found no material findings;
  added its suggested distinct nonzero selector35/36 checks. Full Make suite/build
  passed using a same-source core archive, with clean compiler logs. CMake absent.
- Four original/native race observations match324 prior represented bytes after
  phase/version normalization, plus independent distance/counter/candidate bytes.
  Progress metadata is excluded. This is not full-match/frame/renderer equivalence.
  See NATIVE_CLEARANCE_RACE.md for evidence and private paths.
- Next: winner presentation/control and timeout recovery, then remaining restart,
  keeper/story and shot followup. Overall completion remains unmeasured.

## 2026-10-28 — clearance winner and timeout handoffs

- Traced winner DF0D→E0DF and timeout DED4→8BE5. Prepared winner continuation
  now rejoins player control; timeout retains its exact later recovery entry,
  without repeating owner cleanup or jitter. Keeper-area winners remain pending.
- Shared recovery now supports movement-onward clearance entry36 and recovered
  state37 with retained selected actions/context. Ordinary prepared tails rejoin
  control, clearing transient contexts; control setup owns prefix clearing.
- Session14 marker E retains336bytes and readers1–13. Regression failed against
  stub, then passed. Full native/CLI/adapter/loopback/reference suite and production
  Make build returned0 with clean compiler logs using a same-source archive.
- Seven original/native handoff/recovery observations match324 prior represented
  bytes after phase/version normalization; retained race/distance bytes independently
  compared, native progress/absence metadata excluded. All three timeout recovery
  outputs subsequently reached control setup. No full live-match claim.
- Review found no material issues. Corrected winner observation metadata from
  SP50 to actual SP4F and regenerated reports with unchanged final RAM.
  See NATIVE_CLEARANCE_HANDOFFS.md for commands, evidence and exact limits.
- Next: restart/keeper/story and nonterminal shot followup/goal flow. Overall
  completion unmeasured; full playable match and online match remain unfinished.

## 2026-10-28 — restart route and local CLI transition

- Continued selected restart30 through original9366 situation clear and branch.
  New38/39 distinguish field versus opposing goal-area setup; later restart UI
  remains pending. No repeated reset/movement/reward/clock. Session15 marker F
  remains336bytes, preserving readers1–14.
- Focused regression failed against stub, then passed. Full suite and production
  Make build returned0 with clean diagnostics. Four original/native route states
  match324 prior represented bytes after phase/version normalization. Review found
  no material issues. See NATIVE_SELECTED_RESTART_ROUTE.md.
- Added AGENTS.md and CLI_HANDOFF.md for the user's Windows/WSL move. Transition
  package preserves the local work branch/history, source snapshot, private
  analysis, resume prompt, checksums and a fresh-destination restore script.
  Public GitHub main remains the separate promotional website; engine is local.
- Next engine work: actual field/goal-area restart setup, keeper/story and remaining
  shot/goal integration. Overall completion unmeasured; full match unfinished.


## 2026-10-28 — prepared shared restart options

- User chose to continue in the cloud. Traced shared restart options9880..98D5
  after explicitly prepared opening scripts; live scripts/frame tasks are outside
  this boundary. Added frozen held/pressed input, ordered toggles/confirmation,
  saved route/cursor and clear pending tactics/caller boundaries40..42.
- No clock, movement, stamina, reward, possession or generated random changes.
  Session16 markerG is339bytes and retains readers1..15. Strict development CLI
  supports open/poll/save/reload and rejects replay or malformed operations.
- Focused regression failed with the throwing stub, then passed. Full native/CLI/
  adapter/loopback/reference suite and production Make build passed with clean
  compiler diagnostics using the current52-source archive. CMake unavailable.
- Twelve bounded original/native cases match324prior represented bytes plus
  independently compared cursor0624. All skipped display/wait/sprite calls are
  disclosed. Native route metadata is excluded from original-memory claims.
  Evidence: ../Captain-Majid-analysis/restart-options-tedfarbr. CLI strict input,
  replay and interactive save checks pass. This is not a complete restart or match.
- Fresh independent review found no material issues. Fixed its minor misleading
  command-error text and checked the affected rebuilt CLI. Daily Arabic note adds
  only the verified saved restart options checkpoint; approved roadmap unchanged.
- Next: tactical submenu and caller player/action restart setup, keeper/story and
  nonterminal shot/goal integration. Overall port completion remains unmeasured.


## 2026-10-28 — nonterminal shots reach attacking presentation

- User requested continuous work and original comparison at every gameplay step;
  recorded that requirement in AGENTS.md. Continued phase27 result0/1 through
  original83F5 field advance to8408 before script8. Zero-defender shots supported.
- Added saved phase43 and session17H (339bytes, readers1..16). No new clock,
  reward, stamina, possession or random writes. Existing38..42 validation retained;
  phase43 requires completed nonterminal prefix and rejects replay.
- Real regression failed against terminal-only rejection, then passed. Full
  native/CLI/adapter/loopback/reference suite and production Make build passed
  with clean diagnostics using the current52-source archive. CMake unavailable.
- Four bounded original/native comparisons match324prior represented bytes;
  native metadata excluded from ROM-memory claims. Original observer skips no
  calls/frames on this boundary. Added repeatable private-input comparison command
  tools/check_nonterminal_dispatch_original.py; all four generated cases pass.
- Independent review found no material issue. Added its positiveG→H restart-save
  regression (passes) and updated the prior dispatcher documentation. Evidence and
  commands in NATIVE_NONTERMINAL_DISPATCH.md. Engine source remains private/local.
- Next original step is script8→8410 status/clock→847F keeper status→ordinary
  actionC618/DCFD. Live scripts, actual shot flight, goal/keeper integration,
  complete matches and online play remain unfinished. Overall port unmeasured.

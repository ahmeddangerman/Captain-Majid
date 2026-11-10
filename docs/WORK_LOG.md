# Development log

## 2026-11-10 - Player3 recovers blocked shot

The player7 blocked-shot replay now matches the original 22-player field movement and stationary-ball winner: player3/side0 recovers at (120,140), with his fractional coordinates retained and random index82. `tools/verify_player7_shot_terminal.py` feeds the jittered checkpoint through captured random/travel events and ordered player slots, then compares the recovered owner at original 8C31. The recovery is guarded to this observed state; invalid early recovery rejects and the saved state roundtrips. Private evidence: `../Captain-Majid-analysis/player7-loose-owner-v1-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-shot-20261110`. Windows build and all 62 registered checks passed. Events and presentation timing remain supplied, and resumed control and a complete match remain unsupported. Overall progress is unmeasured. Next compare player3 control setup and ensuing input. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 shot loose-ball jitter

The matched terminal contact now advances through three captured NMI random/travel events and the original loose-ball jitter: ball(120,124) to (120,140), selected-defender prefix cleared, random index60. `tools/verify_player7_shot_terminal.py` compares the resulting snapshot with the private original 8BE5 boundary. The guarded operation rejects other random inputs and preserves snapshot roundtrips and input immutability. Evidence: `../Captain-Majid-analysis/player7-loose-ball-v1-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-shot-20261110`. Windows build and all 62 checks passed. Frame events remain supplied; recovery and a complete match remain unsupported, and overall progress is unmeasured. Next compare the original 22-player movement and loose-ball owner. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 terminal shot contact

From the matched defender result, `tools/verify_player7_shot_terminal.py` replays the original presentation wait with captured random/travel events and three observed facing/motion samples, then matches the terminal contact boundary: elapsed1-to3, player7 status5, clock146-to144, random index57. The guarded transition reuses the contact prelude with action2 and the observed two-tick content. Tests cover sample ordering, invalid pre-presentation contact, input immutability and snapshot roundtrips. Private evidence: `../Captain-Majid-analysis/player7-shot-terminal-clock-v1-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-shot-20261110`. Windows build and all 62 registered checks passed. Frame and presentation timing are supplied, and the following loose-ball handling remains unmatched. A complete match is unsupported and overall progress unmeasured. Next compare the loose-ball jitter and recovery. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 terminal shot defense

Resumed and completed the interrupted defender work. A stage0/team3 private data pack, selected from the captured original team byte, matches defender20's action2/subtype0 preparation (strength8, cost50) and terminal comparison (strength4, loss1, score80, result2). `tools/verify_player7_shot_preparation.py` feeds the prior native shot forward through captured random/travel and presentation-motion samples, then checks the original defender preparation and result registers; random index165 matches. The new guarded prepared/compared snapshot phases roundtrip, and invalid presentation/variant inputs reject. Private evidence: `../Captain-Majid-analysis/player7-defender-result-verified-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-shot-20261110`. Windows build and all 62 registered checks passed. Original frame/presentation events and the observed team/defense choice remain supplied. The terminal contact clock, subsequent loose ball, complete match and live rendering are not yet matched; overall progress unmeasured. Next compare the original contact and loose-ball continuation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 shot defender handoff and preparation

The extended original trace reproduces all 10,849 earlier records after excluding newly instrumented instruction addresses. Following the confirmed shot, defender20's observed choice is action2/subtype0. The native replay matches the handoff and original preparation: player7 stamina385-to305, power5, random index74. `record_player7_shot_defender` validates this specific observed choice and records the opposing player; it does not persist the choice, which must be supplied again to the later defender comparison. `tools/verify_player7_shot_preparation.py` compares both original boundaries with results fed forward. Private evidence: `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-shot-20261110` and `../Captain-Majid-analysis/player7-shot-preparation-replay-v2-20261110`. Windows build and all 62 registered checks passed. The original later shows a terminal defense and loose ball; the native replay has not matched that result yet. Frames and presentation remain supplied, complete match unsupported, overall progress unmeasured. Next trace and compare the defender result. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 encounter menu and shot selection

An extended isolated original capture exactly reproduces all 8,868 prior instruction records. At the player7 encounter, confirm alone leaves the prompt waiting; right selects the shot and a later confirm advances. The native replay now matches the encounter, its captured presentation changes, menu entry and 121 original polls through shot confirmation, with final random index63. `tools/verify_player7_encounter_menu.py` feeds each result forward and compares represented session bytes; the two presentation samples are located between observed boundaries, not independently timed instructions. Tests cover the guarded samples and rejection. Private evidence: `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-player7-encounter-right-20261110` and `../Captain-Majid-analysis/player7-encounter-menu-replay-v2-20261110`. Windows build and all 62 registered checks passed. Live frame, presentation and input scheduling are still supplied; a complete match remains unsupported and overall progress unmeasured. Next compare the chosen shot's defender and outcome. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Directed player7 encounter and observed stamina ordering

An isolated original write watch identifies player7's stamina388-to385 update inside the frame at E33F/E343, after NMI random and before travel. The existing clock calculation produced the same final value earlier at the energy boundary; the guarded captured context now defers that write until an explicit `H` sample in `CMJCLOCKNMI2`, retaining `CMJCLOCKNMI1` reading. Tests cover the recorded sample, missing/wrong/out-of-order rejection and atomic input. The original directed path matches the 147-to146 clock and next 22-player round through E4BE: player7/side0, ball(120,124), defender20, random index56. Private evidence: `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-third-return-player7-stamina-watch-20261110`, `../Captain-Majid-analysis/third-return-player7-directed-third-clock-v4-20261110` and `../Captain-Majid-analysis/third-return-player7-directed-third-world-20261110`. This is one supplied original frame sample, not a generalized live stamina rule. Windows build and all 62 registered checks passed. Full match unsupported and overall progress unmeasured. Next compare the encounter/menu. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Directed player7 control after recovery

A second neutral player7 cycle matches through clock149 and the 22-player field round. A new isolated original run then holds right from frame18823; all 7,279 instruction records before the directed input match the prior run. The native path matches three groups of 11 polls, two intervening clocks and two 22-player rounds, advancing the ball from (110,124) to (120,124) through E1E7 at clock147, random index27. Captured NMI/travel and timing remain supplied. The following original clock contains an additional player7 stamina change during a frame; it is not matched and the replay stops before it. Private evidence: `../Captain-Majid-analysis/third-return-player7-second-world-20261110`, `../Captain-Majid-analysis/third-return-player7-directed-third-polls-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-third-return-player7-directed-20261110`. No engine code changed and no new full suite run was needed for this trace-only increment. A complete match remains unsupported, overall progress unmeasured. Next trace the stamina write before continuing. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Player7 recovers and resumes control

Shared stationary-ball recovery matches original player7 possession at 8107, preserving fractional position bytes and recording the observed ball variant2. After 51 captured wait pairs to E0DF and 60 further pairs with sampled motion0-to1-to0, shared control setup matches the pre-NMI player7 boundary. Two more pairs, 11 neutral polls, the 151-to150 clock tick and a 22-player field round match original E145, player7/side0, random index157. Represented fields and score match at each checked boundary. Evidence: `../Captain-Majid-analysis/third-return-recovered-owner-20261110.cmjsession`, `../Captain-Majid-analysis/third-return-player7-presentation-wait-20261110`, `../Captain-Majid-analysis/third-return-player7-polls-v3-20261110`, `../Captain-Majid-analysis/third-return-player7-clock-20261110` and `../Captain-Majid-analysis/third-return-player7-world-20261110`. Tests cover guards, coordinate fractions, atomic errors and snapshot roundtrips; Windows build and all 62 registered checks passed. Timing/presentation remain supplied; a complete playable match remains unsupported and overall progress unmeasured. Next compare the following player7 input and clock cycle. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third-return loose-ball jitter and movement

Shared jitter calculation matches original C606: random118/100 moves the loose ball from (110,108) to (110,124), resets jitter control and clears the selected defenders. The following 22-slot field movement, with recorded random/travel events, matches original E0DE at random index6 and elapsed0. Represented fields and score match both boundaries. Private evidence: `../Captain-Majid-analysis/third-return-jitter-20261110.cmjsession` and `../Captain-Majid-analysis/third-return-recovery-movement-v2-20261110`. Unit tests cover guards, atomic errors and extended snapshot roundtrips. Windows build and 62 registered checks passed. The next original boundary selects player7, not yet connected; timing/presentation remain supplied, full match unsupported and overall progress unmeasured. Next compare loose-ball owner selection. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third-return result2 contact clock

From the classified defender result, 117 captured random/travel pairs and sampled motion0-to2-to16-to0 and facing0-to64 match the original pre-contact boundary. Shared contact/experience calculations then match original 8127: clock153-to151, elapsed1-to3, owner18 status5 and defender5 experience plus1. Three more supplied random/travel pairs match original E406 before C606 at random index240. Private evidence: `../Captain-Majid-analysis/third-return-result-wait-20261110`, `../Captain-Majid-analysis/third-return-contact-clock-20261110.cmjsession` and `../Captain-Majid-analysis/third-return-postcontact-wait-20261110`. Tests cover sampled ordering, atomic rejection, clock/experience and snapshot roundtrip. Windows build and 62 registered checks passed. Timing/presentation remain supplied; loose-ball handoff and a complete match remain unsupported. Overall progress is unmeasured. Next compare the original C606 loose-ball position. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third-return defender result2

The pre-classification defender comparison now advances separately to original result2 at the next 8107 boundary. Native phase416 stores result2 and matches represented fields and score; the comparison state remains unchanged on rejection. Private evidence: `../Captain-Majid-analysis/verify-third-return-defender-result-20261110.py` and `../Captain-Majid-analysis/third-return-defender-result-20261110.cmjsession`. Tests cover result, atomic input, repeat rejection and extended snapshot roundtrip. Windows build and all 62 registered checks passed. Subsequent presentation/contact remains unmatched; supplied timing and presentation are not an emulated original engine. Overall progress is unmeasured and a complete playable match remains unsupported. Next compare result2 continuation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third-return defender comparison

Shared player-profile, energy and field-comparison calculations now match the original 8132 pre-classification boundary: defender5 base8, cost70, stamina400-to330, strength4, loss1 and score64 against incoming power4. Native phase415 matches represented fields and score; the original CPU comparison value A=64 is independently checked. Its calculated result2 is guarded, but the original result byte is not committed at this pre-classification boundary. Private evidence: `../Captain-Majid-analysis/third-return-defender-probe-20261110.cpp`, `../Captain-Majid-analysis/verify-third-return-defender-20261110.py` and `../Captain-Majid-analysis/third-return-defender-compared-20261110.cmjsession`. Unit tests cover calculation, rejected content and extended snapshot roundtrip. Windows build and all 62 registered checks passed. Timing/presentation remain supplied; complete match unsupported, overall progress unmeasured. Next compare the original result2 classification and subsequent contact. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third-return attacker preparation

After the matched defender confirmation, the explicitly retained action2 choice and shared attacker preparation now match the original D568 and subsequent second 8107 boundaries. Captured player18 is type0, level0, stamina0; the shared preparation derives base9, cost40, flags0 and power4, preserves opponent stamina0 and clears the observed carrier-cell scratch. Represented fields and score match at both boundaries. Private evidence: `../Captain-Majid-analysis/third-return-action-choice-20261110.cmjsession`, `../Captain-Majid-analysis/third-return-action-prepared-20261110.cmjsession` and `../Captain-Majid-analysis/third-return-attack-probe-20261110.cpp`. The next original 8132 has defender5 stamina330 and comparison score64; it is original-only, not yet matched by a native session transition. Unit guards and all 62 registered Windows checks passed. Presentation/timing remain supplied, a full match remains unsupported and overall progress is unmeasured. Next compare defender preparation and outcome. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Timed registration and final defense confirmation

Connected the third-return defender registration to the captured sequence: one supplied random/travel pair matches E406 while cursor remains0, then the previously verified registration matches D3A6 with cursor1. Another 59 timed menu polls match the original, including final A confirmation and nine supplied presentation-wait pairs, through D568 at random index120. Represented fields, score and defensive context match each checked boundary. Evidence: `../Captain-Majid-analysis/third-return-registration-wait-20261110` and `../Captain-Majid-analysis/third-return-final-confirm-20261110`. Guarded tests cover both wait phases and final confirmation. Windows build and all 62 registered checks passed. Original timing and presentation remain supplied; downstream encounter action and a full match remain unsupported. Overall progress is unmeasured. Next compare the following action/preparation boundary. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Timed defender selection and controlled registration

An instrumented private original trace reproduced all 6,888 prior instruction records after filtering its additional menu probes. From the matched third-return prompt, 220 sequential menu polls now match represented state, score and defensive context, with supplied random/travel events including blink presentation waits, through the recorded right selection and A confirmation at original D3D4, frame18421, random index51. Separately, a controlled original preparation check shows no special option for captured defender5/action0 and matches native registration, cursor1, blink0, sprite12, score and represented fields. The preparation comparison is controlled and has not yet been connected to the timed presentation wait. Evidence: `../Captain-Majid-analysis/third-return-timed-defense-v5-20261110` and `../Captain-Majid-analysis/verify-third-return-defense-registration-20261110.py`. Windows build and all 62 registered checks passed. Original timing/presentation remain supplied; complete match unsupported and overall progress unmeasured. Next connect the captured registration wait and continue the encounter. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Controlled third-return defender selection

From the captured defender prompt, a controlled original poll with pressed right (1) selects action0 and requests sprite12 at slot0. The native menu poll matches represented fields, score, action/subtype array and blink13/flash192. This controlled poll was initiated from the original frame18186 prompt; it is not yet the continuous real-time wait to the later frame18302 input or the final confirmation. Private evidence: `../Captain-Majid-analysis/verify-third-return-defense-selection-20261110.py` and `../Captain-Majid-analysis/third-return-defense-selection-20261110.cmjsession`. Guarded validation and tests accept this observed choice and reject an unobserved direction atomically. Windows build and all 62 registered checks passed by direct execution. Original presentation/timing remain supplied; full match unsupported and overall progress unmeasured. Next compare the timed wait and confirmation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third return defender prompt

After original C600, 41 supplied random/travel pairs and the observed cursor160-to0 sample match represented fields and score at frame18175. Another 11 supplied pairs match the frame18185 boundary; acknowledging the captured presentation and blink13/flash192 initializes the defender prompt matching original frame18186. The sampled acknowledgement records the original boundary; it is not an emulated presentation engine. Private evidence: `../Captain-Majid-analysis/third-return-menu-cursor-v2-20261110`, `../Captain-Majid-analysis/third-return-menu-preblink-20261110` and `../Captain-Majid-analysis/third-return-defense-prompt-20261110.cmjsession`, compared with the extended choice trace. New guarded checks cover cursor input, rejection, prompt state and snapshot roundtrip. Windows build and all 62 registered checks passed by direct execution. The subsequent defender selection is original-only; timing and presentation remain supplied, and a complete match is unsupported. Overall progress is unmeasured. Next compare the observed right-direction selection and confirmation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Extended original encounter capture

Built an isolated private emulator trace through frame18850, leaving the prior capture intact. All 3,436 instruction records and RAM snapshots in the overlap match the earlier trace. A controlled right-direction selection followed by repeated A inputs advanced the original encounter; the captured screen showed a block selection and later action presentation. The first observed defender action boundary is original 8132 at frame18490; continuation changes are original-only, not yet matched by native code. Private traces: `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-postgoal-extended-20261110` and `../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-postgoal-extended-choice-repeat-20261110`. No engine code changed in this research increment. The recorded choice and presentation do not establish autonomous AI or complete match play. Overall progress is unmeasured. Next compare the encounter menu initialization and selection boundaries. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Arabic progress publication

Updated only the November 10 Arabic progress note in the separate website checkout, committed and pushed public main at `d44aa3d`. Inline JavaScript syntax and note structure passed; the live GitHub Pages HTTP response later contained the new note. This was a source/HTTP check, not a browser render verification. Overall engine progress remains unmeasured; a complete match is unsupported. Next verify the local demo's interactive input and continue the original encounter. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Windows demo runtime dependencies

The SDL scene initially failed to start with PATH restricted to Windows System32, because this Clang build also needs `libc++.dll` and `libunwind.dll`. Copied the matching local toolchain runtime DLLs beside the ignored `build/demo` executable. With the same restricted PATH, the private-pack capture succeeded. Updated the private-pack regression helper to use a restricted PATH on Windows and the demo runbook to list all three adjacent runtime DLLs. This verifies headless launch, not keyboard/window interaction. Overall progress is unmeasured; full match unsupported. Next verify actual interactive input when the scene can be brought forward. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third return encounter later wait

Using the existing observed sequence support, 74 additional recorded random/travel pairs carry the matched encounter from 85BF to original C600, random index19. Represented fields and score match at the boundary. Private evidence: `../Captain-Majid-analysis/third-return-contact-later-wait-20261110`. Timing and presentation remain supplied; C600 continuation remains unmatched. No further engine changes or new suite run were needed for this trace comparison. Overall progress is unmeasured and a complete playable match remains unsupported. Next inspect C600 and subsequent encounter state changes. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third return encounter presentation wait

From the matched E4BE encounter, 21 recorded random/travel pairs and sampled motion1-to0 and display2-to0 reproduce original 85BF represented fields and score at random index201. Guarded presentation-sample acceptance and atomic rejection checks cover the recorded owner18/defender5/clock153 context. Private evidence: `../Captain-Majid-analysis/third-return-contact-wait-v2-20261110`. This is supplied presentation state, not a running original presentation engine. Original continuation after 85BF remains unmatched. Windows build and all 62 registered checks passed via direct execution. Overall progress is unmeasured; full match and autonomous AI/timing remain unsupported. Next trace the encounter continuation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third post-return decision to encounter

The eleventh poll reaches a pending opponent decision; supplying the observed action2/subtype0 changes the preserved action3 to action2 and matches original E1E7 represented fields and score. The following captured clock tick matches 154 to153, including three random/travel pairs and one supplied mode sample. The subsequent 22-player round matches original E4BE encounter, random index180, including five supplied mode samples. Private evidence: `../Captain-Majid-analysis/postgoal-third-return-choice-20261110.cmjsession`, `../Captain-Majid-analysis/postgoal-third-return-clock-v2-20261110` and `../Captain-Majid-analysis/postgoal-third-return-world-20261110`. Guarded acceptance and atomic rejection checks were added. Windows build and all 62 registered checks passed via direct execution. AI choice, random/timing and presentation remain supplied; encounter continuation and a complete playable match remain unsupported. Overall progress is unmeasured. Next compare the encounter transition. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Third post-return control polls, partial

At clock154, the first ten subsequent original polls match the native represented fields and score, including supplied 192-to-128, 128-to-192 and 192-to-128 ball-mode samples. The guarded observed-poll path now accepts these sampled transitions in the recorded owner18/side11/1-0 context at clock154. Regression checks cover acceptance and rejection at unobserved clock153. Private evidence: `../Captain-Majid-analysis/postgoal-third-return-polls-v2-20261110`. The eleventh poll matches represented fields except action.id: native retains 3 while the original changes it to 2 at the E19B/E1B2 decision before E1E7. This boundary is partial, and has not been used for further continuation. Timing, presentation and choices remain supplied. Windows build and all 62 registered checks passed by direct execution. Overall progress is unmeasured; full match unsupported. Next trace the action2 decision and verify the relevant transition before proceeding. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Windows SDL capture repair

Fixed the Windows SDL bitmap capture by serializing to an SDL memory stream and writing those bytes in the executable's own C runtime. The prior executable used UCRT while its SDL DLL used msvcrt; passing a `FILE*` between them failed. Before the change, `tools/check_shot_scene_capture.py` reproduced `Error writing to datastream`; after rebuilding, it verified a valid bitmap and rejection without modification of an existing file. The check takes a private pack and private temporary directory and commits neither. A Windows build and all 62 registered checks passed (executed directly from the generated CTest file because CTest is unavailable). The actual keyboard interaction remains unverified; no gameplay changed. Overall progress remains unmeasured and a complete playable match remains unsupported. Next verify the input/reset loop and investigate the next original-backed match transition. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Windows demo retry

Relaunched the existing SDL scene with the private shot pack. The window was present in the app inventory, but its screenshot was covered by an active full-screen game, so this retry does not establish keyboard or visible shot behavior. The `--shoot --capture` path reached bitmap saving but twice reported `Save capture: Error writing to datastream`, leaving zero-byte private capture files. Import inspection shows the executable uses UCRT and the downloaded SDL2 DLL uses msvcrt; passing a `FILE*` via `SDL_RWFromFP` across these runtimes is a likely cause, still to be verified by a controlled fix. A missing pack reported `Unable to open local content pack`. `cmake --build build -j 2` reported no work; no test suite was run. No gameplay changes were made. Next investigate the capture write failure, then verify input/reset when the scene is visible. Overall progress is unmeasured; a complete playable match remains unsupported. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Windows shot scene launch checkpoint

Built the optional SDL scene on Windows using the existing x86_64 Clang/Ninja toolchain and a local official SDL2 2.30.11 MinGW SDK under ignored `build/`. CMake's default Visual Studio generator had no usable compiler; explicit Clang/Ninja configuration succeeded. Launched `captain-shot-scene.exe` with the private `japanese-shot-v1.cmjpack` and observed the real Windows field and controls. The shot interaction has not yet been verified: automated Space attempts did not produce a visible outcome, so input/focus requires investigation. No gameplay was changed and no test suite was run for this checkpoint. See `docs/DEMO_RUNBOOK.md` for local launch. Ruling: use ignored `build/demo` rather than the plan's unignored `build-demo` to avoid adding generated files. Overall progress is unmeasured, and a complete playable match remains unsupported. Next verify input, reset and repeat, then continue the demo plan. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Revised eight-hour demo plan

Created `docs/superpowers/plans/2026-11-10-eight-hour-demo.md`, superseding the seven-hour plan. Source inspection confirms that the optional SDL shot scene already has keyboard controls and calculated outcome display, with explicit reset-per-shot behavior and fixed development inputs. The revised schedule checks actual Windows launch in hour1, establishes repeatable interaction by hour4, conditionally advances one original-backed persistent transition in hours5-6, and reserves hours7-8 for verification and delivery. This is planning only: no engine changes, new test results or working-window claims. Overall progress is unmeasured; a complete playable match remains unsupported. Next execute the launch feasibility check. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Second post-return control cycle

The second post-return cycle now matches eleven further neutral original polls, including three supplied ball-mode changes; the shared clock then matches155 to154 with four random/travel pairs and a supplied mode change. The following 22-player field round matches original $E145, with five supplied mode changes and random index144. Represented fields and score match at each poll, clock and field-round boundary. Private evidence: ../Captain-Majid-analysis/postgoal-second-return-polls-20261110, ../Captain-Majid-analysis/postgoal-second-return-clock-20261110 and ../Captain-Majid-analysis/postgoal-second-return-world-20261110. A fresh Windows build and all 62 registered checks passed. Recorded inputs and presentation remain supplied; later play is original-only. Overall progress is unmeasured and a complete playable match unsupported. Next compare the following control cycle and opponent decision. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Post-return clock and field round

After eleven matched neutral polls, the shared control-clock kernel reproduces the original clock156 to155, keeper recovery, scheduling and observed energy updates with three recorded random/travel pairs. The following shared 22-player field round matches original $E145, random index107, including six explicitly supplied ball-mode samples at slots1,5,9,13,17 and21. All represented fields and score match at the clock and field-round boundaries. Private evidence: ../Captain-Majid-analysis/postgoal-return-clock-final-20261110 and ../Captain-Majid-analysis/postgoal-return-world-20261110. A fresh Windows build and all 62 registered checks passed. Original timing, input and presentation remain supplied; later control and decisions remain original-only. Overall progress is unmeasured and a complete playable match unsupported. Next compare following polls and the opponent decision. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Eleven neutral polls after action3 return

After the matched player-control setup, four recorded NMI random/travel pairs and a supplied ball-mode change produce the original first $E145 boundary. Eleven neutral polls then match each original boundary through $E1E7, clock156 and random index82. Three captured ball-mode changes occur after random and before travel on polls3,7 and11. The new `CMJPOLLNMI2` record carries an optional guarded mode sample at that point; `CMJPOLLNMI1` remains readable. Focused tests cover the format, valid sampled control and rejection outside the captured context. Private evidence: ../Captain-Majid-analysis/postgoal-first-return-poll-20261110 and ../Captain-Majid-analysis/postgoal-return-polls-20261110. A fresh Windows build and all 62 registered checks passed. The following clock and field round are original-only; input, timing and presentation remain supplied. Overall progress is unmeasured and a complete playable match remains unsupported. Next compare the clock and field round. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Return to player control setup

The captured action3 return now reconnects to the shared player-control setup after the sampled pre-setup presentation. At the original pre-NMI $C9C5 boundary, native represented fields and score match: owner18 at x117/y108, clock156, timer10, field side11 and nearest defender10. The original retains action3's scratch fields across setup; preserving these was necessary for parity. Old saved flight/defender contexts are retired at this transition, while original action fields remain. Private evidence: ../Captain-Majid-analysis/postgoal-return-control-final-20261110. A fresh Windows build and all 62 registered checks passed. Subsequent presentation and input polling are original-only. Timing and presentation remain supplied; a complete playable match is unsupported and overall progress unmeasured. Next compare control presentation and the first neutral poll. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Pre-control presentation samples

Following original $E0DF, another 62 captured random/travel pairs and ordered facing64 to0, motion0 to1 to0 samples take phase414 to the last $E406 before field-control setup. Native represented fields, score and trajectory match at random index67. Private evidence: ../Captain-Majid-analysis/postgoal-field-entry-wait-20261110. A fresh Windows build and all 62 registered checks passed. These are supplied presentation samples, not a running original presentation engine. Original control setup and subsequent polling remain unmatched. Overall progress remains unmeasured and a complete playable match is unsupported. Next compare the control setup boundary. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Return reception field entry

After player18 receives the action3 return, a guarded phase414 preserves possession and the flight context through 107 recorded random/travel pairs and one facing sample changing0 to64. Native represented fields, score and trajectory match original $E0DF at random index5. Private evidence: ../Captain-Majid-analysis/postgoal-reception-wait-20261110. A fresh Windows build and all 62 registered checks passed. This acknowledges captured presentation timing; it does not run the original presentation engine. The later field-control setup is original-only. Overall progress remains unmeasured and a complete playable match is unsupported. Next compare control setup. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Action3 return received by player18

After the matched defender cleanup, the original transfers ownership from player14 to player18 and moves the ball from x116 to the receiver's x117, y108, retaining fractional bytes. Guarded phase413 uses the saved recipient's position and matches the original bank26:$8107 represented fields, score and trajectory. The extended isolated original trace agrees with the earlier capture at overlapping checkpoints and continues into presentation, control and later decisions; those later events are original-only. Private evidence: ../Captain-Majid-analysis/postgoal-return-reception-20261110 and ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-postgoal-continuation-20261110/trace. A fresh Windows build and all 62 registered checks passed. Timing and presentation remain supplied; overall progress remains unmeasured and a complete playable match is unsupported. Next compare the reception presentation and return to control. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Defender result0 clock and cleanup

Following the supplied result0 wait, the original sets defender8 status6 and advances clock157 to156 before the next NMI; the shared contact prelude reproduces those effects and elapsed1. Three recorded random/travel pairs then reach random index154. The original increments the defender cursor, clears the selected prefix and resets elapsed; phase412 matches $888A with cursor1 and defender status retained. Native represented fields, score and trajectory also match the original before the first NMI and at $8882. Private evidence: ../Captain-Majid-analysis/postgoal-defender-clock-20261110. A fresh Windows build and all 62 registered checks passed. Timing and presentation remain supplied; subsequent flight continuation is original-only. Overall progress remains unmeasured and a complete playable match is unsupported. Next trace the continuation following $E7CF. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Defender result0 presentation wait

The captured action3 defender result0 now continues through the original $88AD boundary. Phase410 retains the comparison and ball trajectory while accepting 141 recorded NMI random/travel pairs and one recorded facing change from64 to0; native represented fields, score and trajectory match the original, with final random index151. This is a supplied presentation wait, not execution of the original presentation engine. Private evidence: ../Captain-Majid-analysis/postgoal-defender-wait-20261110. A fresh Windows build and all 62 registered checks passed. The original next changes defender8 status to6, advances the clock and clears the selected defender list; those changes remain original-only. Overall progress remains unmeasured, and a complete playable match and autonomous timing/AI remain unsupported. Next compare those changes. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Action3 return defender comparison

One recorded NMI random/travel pair takes the centered arrival to original $8845, random index10 and bytes104/186. Defender8's action2 preparation gives base8 and cost50. The original action3-specific comparison table has range128, offset128 and eligibility251: shared arithmetic gives strength4, loss1, score192 and result0. A controlled isolated original CPU comparison matched 513 cases against the native calculation. The action0 flight-content row is a different table and must not be used for this action3 encounter.

Guarded phase409 uses explicitly supplied comparison-table bytes and the existing player-profile and defender kernels. All represented fields, trajectory and score match the original at $8845 and $8874; the restored ball variant and selected defender are retained. Private evidence: ../Captain-Majid-analysis/defender3-contact-compare-20261110.py and ../Captain-Majid-analysis/postgoal-defender-compare-final-20261110. Focused checks cover recorded timing, comparison, invalid input and snapshot roundtrip. A fresh Windows build and all 62 registered checks passed. Result presentation and later clock/defender handling remain original-only; recorded timing and presentation are supplied rather than simulated. Overall progress remains unmeasured and a complete playable match is unsupported. Next compare the result0 continuation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Return-flight arrival center snap

The captured return-flight arrival now has a separate guarded phase408 matching original $E7CC. The original changes ball x119/y110 to the cell center x116/y108 and preserves fractional bytes 248/152, the flight context and selected defender8. The native replay matches represented fields and trajectory; private evidence is in ../Captain-Majid-analysis/post-kickoff-returnflight-snap-20261110. Focused coverage checks the transition, snapshot roundtrip and rejection of invalid replay. A fresh Windows build and all 62 registered checks passed. Original defender processing is not yet native-compared. Timing and presentation remain supplied, and a complete playable match is unsupported. Overall progress remains unmeasured; next compare defender8's captured zero-range calculation. Suggested model GPT-6.1 Terra High.

## 2026-11-10 - Action3 return flight reaches defender8

After the matched first arrival, 151 recorded random/travel pairs lead to the original inter-flight boundary. Shared byte arithmetic moves player18 from x141 to117 while preserving the low byte; carrier and recipient then exchange, with player14 owning the ball and player18 selected. The return target derives from the moved recipient as cell99. Shared geometry/velocity reproduce direction137, velocity -246/-49 and the first flight's retained coordinate fractions. The original's stale distance18 remains explicitly preserved.

Five further recorded random/travel pairs carry the return flight through cells136,124,112,111 and99. Shared defender selection adds player8 at cell112 and retains that single prefix. Phase407 matches original E7C0 with unsnapped x119/y110 and selected defender8; original snaps to the cell center before the later defender handler, which is not yet native-compared. All represented fields and trajectory bytes matched at the exchange, return setup, four callbacks and arrival. Private evidence: ../Captain-Majid-analysis/post-kickoff-exchange-final-20261110, ../Captain-Majid-analysis/post-kickoff-returnflight-final-20261110 and ../Captain-Majid-analysis/post-kickoff-returnflight-steps-final-20261110. Full Windows build and all62 registered checks passed. The recorded timing/presentation is supplied; autonomous AI and a complete playable match remain unsupported. Overall progress unmeasured; next compare the arrival snap and defender8 confrontation. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Recovered damaged local Git object

The first-flight commit initially could not be pushed because its kickoff test blob had a corrupt loose Git object. The working file, after the repository's configured CRLF normalization, hashed to the exact missing object ID. Preserved the corrupt compressed bytes privately at ../Captain-Majid-analysis/corrupt-git-object-59afe42d-20261110.bin, restored the exact verified blob, and ran `git fsck --full --no-reflogs` successfully (only dangling trees were reported). Commit f28402f then pushed successfully to the authorized work branch. No user files were removed. Next continue the action3 inter-flight exchange; overall progress remains unmeasured. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Action3 first flight reaches player14

The captured opponent action3 now matches original first-flight setup, intermediate callback and arrival. From saved player14, the shared cell calculation changes target161 to148 after field movement; shared direction and velocity calculations reproduce direction34, velocity162/185 and zero fractions. The preserved distance18 is explicitly supplied stale original scratch, not a new distance calculation. With two captured random/travel pairs, shared numeric stepping reaches cell147, an original-backed defender scan finds no defenders, and the next step reaches cell148; represented fields and trajectory bytes match the original at E773, E7B4, the resumed E773 and snapped E7CF. Final random index109. Private evidence: ../Captain-Majid-analysis/post-kickoff-action3-firstflight-final-20261110 and ../Captain-Majid-analysis/post-kickoff-firstflight-steps-final-20261110.

Original-only continuation later moves player18, switches the ball owner to player14 and starts a return flight toward cell99; that second flight selects defender8. Those events are not yet native-compared. A fresh Windows build and all62 registered checks passed; original timing and presentation remain supplied, not simulated. Overall progress unmeasured; a complete playable match remains unsupported. Next compare the intervening presentation, original player movement and return-flight setup. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Action3 reaches the original flight handler

After $C606, the captured original executes 22 ordered field-player updates with one random/travel pair per slot. Phase398 reuses the existing movement kernel and matches the original $E0DE state, elapsed0, region130 and random index72. The following presentation uses 35 recorded random/travel pairs and explicit motion0, facing64 and display0 samples. Phase399 retains the selected player14/cell161 and cost60, acknowledges the observed carrier marker128 and ball variant1, and matches the original $C621/$DFD9 flight-handler entry at random index107. Original/native represented fields and score/formation match at both boundaries. Private evidence: ../Captain-Majid-analysis/post-kickoff-action3-fieldmove-final-20261110, ../Captain-Majid-analysis/post-kickoff-action3-flight-handoff-final-20261110 and ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-post-kickoff-action3-route-20261110/trace.

Original DFD9 next derives a new target from the moved recipient and enters numeric flight; that calculation and flight are not yet native-compared. The recorded samples acknowledge presentation rather than executing it. A fresh Windows build and all62 registered checks passed; focused tests cover ordering, invalid handoff and snapshot roundtrip. Overall progress remains unmeasured; a complete playable match and autonomous AI/timing remain unsupported. Next compare action3 target/flight setup and its subsequent recipient exchange. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Opponent action3 reaches dispatch

The captured startup continues through the opponent action3 presentation and dispatch. Phase394 accepts ten recorded random/travel pairs and three ordered ball-mode samples; explicitly acknowledging the observed script/mode reset reaches phase395 and matches original bank26:$8027. The original executes D193 with elapsed1; phase396 reuses keeper recovery and match-clock arithmetic, matching the original clock158 to157 and elapsed1 to2 before the next NMI. Three recorded random/travel pairs then reach random index50, and phase397 clears the observed carrier-cell marker at original $C606. Represented state, score/formation and retained selected target/cost match at each compared boundary. Private replays: ../Captain-Majid-analysis/post-kickoff-action3-handoff-20261110 and ../Captain-Majid-analysis/post-kickoff-action3-dispatch-20261110.

A fresh Windows build and all62 registered checks passed; focused tests cover guards, replay rejection and snapshot roundtrip. The replay supplies original timing and explicitly acknowledges presentation; no autonomous AI, live presentation or complete playable match is established. The original later reaches ball flight, still unconnected. Overall progress unmeasured; next trace the ordered field movement after $C606 and compare the flight handoff. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Captured opponent action3 prepared in session

The continuously connected startup now matches original E1B2 choice and E1CB preparation after the 1-0 kickoff: phase392 records the explicitly supplied action3/subtype0, recipient14 and target161, and phase393 uses the shared attack kernel to reproduce base13, cost60, move flags3 and power12. Both represented states and score/formation fields match captured original RAM. The existing serialized target context preserves recipient/cell/cost; both new phases roundtrip without changing old snapshot layouts. Focused tests cover guards, source preservation, truncated/missing context, invalid content and replay rejection. The original arithmetic also matched14 independent controlled outputs, as recorded below. Evidence: ../Captain-Majid-analysis/post-kickoff-action3-session-final-20261110 and ../Captain-Majid-analysis/post-kickoff-attack-kernel-20261110.py.

Fresh Windows build and all62 registered checks passed. The choice is supplied, not computed by native AI; the original's later presentation, handler and flight remain unconnected, and autonomous scheduling and a complete playable match remain unsupported. Overall progress unmeasured. Next trace the action3 presentation and bank26:$8027/$C606 handoff, preserve timing and compare before flight. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Captured action3 calculation independently verified

At the captured E19B opponent decision, the original selects action3, clears subtype and sets player14/target161 at E1B2. From the actual captured RAM/registers, 484 original CPU instructions execute to E1CB and reproduce all original RAM outside the CPU stack. The existing native attack kernel matches all14 controlled original routine outputs for player18, team3, action3: selector2, skill index17, base13, cost60, move flags3, multiplier239 and resulting power12. Side11 stamina remains zero in this routine. This is an independent arithmetic comparison, not yet an integrated native AI choice or session continuation. The earlier commentary calling this a pass was premature; action3's full semantics are not yet established.

A second live original trace overlaps the earlier E19B state byte-for-byte and subsequently reaches bank26:$8027 at frame17393, fixed-bank $C606 at17396 and $E773 at17453 with target148, then a further $E773 at17454. These later observations are original-only; presentation and autonomous scheduling remain unsupported. Private scripts/evidence: ../Captain-Majid-analysis/post-kickoff-action3-original-20261110.py, ../Captain-Majid-analysis/post-kickoff-attack-kernel-20261110.py and ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-post-kickoff-action3-20261110/trace. The prior full Windows build/all62 checks passed; a subsequent focused observed-world test also passed after adding invalid sample coverage. Overall progress unmeasured; next connect the captured choice/preparation with explicit target context and trace action3 flight. Suggested model GPT-6.1 Terra High.


## 2026-11-10 - Post-goal kickoff reaches opponent decision

The captured startup goal continues through opposing kickoff, live control setup, 33 further neutral polls, two clocks and two 22-player field rounds to the original E19B AI decision. Original and native represented state and random indices match at every compared boundary, including clock159 then158 and final random index37. Guarded recorded ball-mode samples preserve the original's temporary 128/192 presentation toggles in poll, clock and new version2 field-round event streams; they are observations, not a simulated display scheduler. Version1 field-round inputs remain accepted. Focused tests exercised unsupported and ordered sample behavior; fresh full Windows build and all62 registered checks passed.

The original alone subsequently writes action3, clears subtype and selects player14 at E1B2; by E1CB its represented action power is12 and move flags3 (separate scratch byte $32 is13). This selection and preparation have not yet been native-compared or semantically classified; do not confuse scratch $43D with action $43B. Live presentation, AI decisions and complete match remain unsupported. Private evidence: ../Captain-Majid-analysis/post-kickoff-polls-20261110, ../Captain-Majid-analysis/post-kickoff-world-v2-20261110, ../Captain-Majid-analysis/post-kickoff-polls-cycle2-20261110, ../Captain-Majid-analysis/post-kickoff-clock-cycle2-20261110, ../Captain-Majid-analysis/post-kickoff-world-cycle2-20261110, ../Captain-Majid-analysis/post-kickoff-polls-cycle3-20261110 and ../Captain-Majid-analysis/post-kickoff-ai-decision-20261110. Next trace and compare this action3 preparation and its subsequent routing. Overall progress unmeasured; suggested model GPT-6.1 Terra High.


## 2026-11-10 - Live post-goal control polls compared

Extended the captured 1-0 restart through the live original control setup and eleven neutral polls: the first poll follows the original E145 control entry, and ten further polls reach the original E1E7 clock boundary. The trace overlaps the previous capture with all 2,048 RAM bytes equal. Every compared native boundary matches represented field/control/context and random index. The kickoff-to-control wait supplies 62 random/travel pairs and observed facing0, motion1 then0; the initial control entry supplies four more pairs and a ball-mode sample. During the following polls the original temporarily toggles ball mode between128 and192; explicit, guarded `B` samples retain the observed ordering. They acknowledge original presentation effects and do not reproduce a presentation engine. Final random index220. Private evidence: ../Captain-Majid-analysis/goal-restart-live-control-20261109, ../Captain-Majid-analysis/goal-restart-live-control-v2-20261109, ../Captain-Majid-analysis/goal-restart-live-control-v3-20261109 and ../Captain-Majid-analysis/post-kickoff-polls-20261110.

The first subsequent control clock reaches field-round pending at clock159 and matches the original except the original presentation's ball-mode bit (native128, original192); three recorded random/travel pairs were supplied. This is a partial comparison, not a complete clock match. The original toggles the same bit during clock and field rounds; preserve its recorded order before claiming further parity. Fresh Windows full build and all62 registered checks passed after the code changes. Overall progress unmeasured; a complete playable match and autonomous timing/AI remain unsupported. Next connect the observed clock and field-round presentation events and compare their boundaries. Suggested model: GPT-6.1 Terra High.


## 2026-11-09 - Captured goal restart reaches opposing control

The captured 1-0 startup goal continues through the recorded celebration, post-goal menu, restart and opposing kickoff. The original and native represented session fields match at the celebration overlap, menu exit, opposing kickoff setup, formation, receiver selection, ball placement, post-display handoff, and an independently executed original control setup. The receiver is player18 and the ball reaches 148/108 at control setup. The celebration and menu replay supply 1,108 random/travel pairs, facing0 and motions19/28/0; seven more pairs precede formation. The kickoff display supplies 189 pairs, facing64 and motions2/0, reaching random index143. Phase63 now accepts those supplied display samples for this guarded 1-0 state. The original control setup separately used cooperative host acknowledgements (64 host frames), not live NMI/presentation. The original live trace later advances into control; that later state has not yet been compared.

Evidence remains private: ../Captain-Majid-analysis/goal-restart-investigation-20261109-v2, ../Captain-Majid-analysis/goal-restart-investigation-20261109-v3, and ../Captain-Majid-analysis/goal-restart-display-20261109, with private replay scripts beside them. A fresh Windows CMake build and all 62 registered checks passed; the new focused test first failed at the unsupported event boundary and then passed, including invalid sample guards. The recorded presentation is not simulated, and autonomous timing/AI and a complete playable match remain unsupported. Overall progress is unmeasured. Next compare the live post-kickoff control sequence and continue toward a full match. Suggested model: GPT-6.1 Terra High.


## 2026-11-09 - Restart work in progress inspected

The working tree contains uncommitted continuation work for captured goal celebration samples, acknowledged restart and a narrowly identified 1-0 kickoff status reset. It was already present when this session resumed; it is not yet supported by a documented original/native restart comparison. Preserved it for continued investigation. A fresh Windows `cmake --build build -j 2` completed and `build/kickoff-session-tests.exe` passed, including the new synthetic checks. These checks do not establish original equivalence or execute presentation. The previously documented goal registration remains the last verified end-to-end boundary.

Next compare the original celebration/menu and restart checkpoints against this working tree, including timing, status-reset ordering and snapshot fields, before relying on or generalizing the changes. Overall progress remains unmeasured; a complete playable match is not yet established. Suggested model: a high-reasoning coding model.


## 2026-11-09 - Captured startup reaches goal registration

The continuously replayed startup now matches the original from the selected keeper result through the original $85AC goal handler and $85F0 score write: 0-0 becomes 1-0 and scorer8 receives 15 experience. During the 226-frame keeper presentation, the replay supplies 226 recorded random/travel pairs and one motion sample; phase390 does not render or execute that presentation. Phase391 reuses the existing scorer-reward and score code after explicit acknowledgement, rejects missing observed motion and preserves the preceding context. Represented state and random index119 match the original at the goal handler and score boundary. Full replay: ../Captain-Majid-analysis/live-return-area-volley-goal-scored-20261109; live follow-up trace: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-live-goal-continuation-20261109/trace.

A separate original run reaches the post-goal menu, accepts a supplied confirm and returns the opposing side to play. This later route is original-only, not yet connected or native-compared. Full Windows build and all62 registered checks passed via direct invocation (`ctest` unavailable). Rendering, audio, autonomous timing/AI, restart in this continuation and a complete playable match remain unsupported. Overall progress unmeasured; next compare celebration/menu and the restart handoff. Suggested model Terra High.


## 2026-11-09 - Captured goalkeeper result before presentation

The captured startup now continues from the selected opposing goalkeeper through the original result write at bank26:$8599. The original $856B state, continued for ten CPU instructions with the captured registers and RAM, writes result0 and cursor0; team3/game-mode0 does not trigger the story override. Phase390 reuses the shared prepared-keeper and story calculations with explicitly supplied action0/subtype0/team3, rejects other choices or nonzero results, and retains the keeper decision before presentation. Represented native state, keeper fields and random index149 match the isolated original $8599 checkpoint and the subsequent live presentation entry. The full captured replay matched 75 returned menu polls through this boundary. Private evidence: ../Captain-Majid-analysis/live-return-area-volley-keeper-result-20261109 and ../Captain-Majid-analysis/area-shot-prep-investigation-20261109/live_keeper_result.py.

The full Windows build and all62 CMake-registered checks passed, run directly from their registered commands because `ctest` is unavailable on this machine. Presentation, scoring/restart and live AI/frame scheduling remain unsupported in this continuation; a complete playable match remains unverified. Overall progress is unmeasured; next trace outcome presentation, goal registration and restart. Suggested model Terra High.


## 2026-11-09 - Live goalkeeper comparison arithmetic

The observed live bank26:$856B boundary is after the goalkeeper comparison and before story override/result write. For the captured action0 input, original scratch/registers and the independent native goalkeeper kernel match cost20, base11, strength6, score255 and threshold classification0. This is a live-input arithmetic comparison, not an integrated native result or goal. Private reproducible check: ../Captain-Majid-analysis/area-shot-prep-investigation-20261109/live_keeper_check.py. Two intentional keeper-prefix/elapsed mutations were also reported with boundary and field. The prior full Windows build and all62 tests passed; overall progress remains unmeasured. Next verify live result write/presentation and scoring handoff; suggested model Terra High.



## 2026-11-09 - Controlled selected-keeper calculations

At captured selected-keeper input, the existing native preparation/comparison matches isolated original $852F..$8599 for action0: cost20, base11, stamina0 and result0. Sixteen constructed cases across four actions and four random pairs match cost, base, stamina and result; the supplied actions and frozen timing do not establish autonomous live choice or a scored goal. Existing oracles independently matched69,632 skill-index, 7,456 action-power and 5,248 keeper cases. Private script: ../Captain-Majid-analysis/area-shot-prep-investigation-20261109/keeper_check.py. Overall progress unmeasured; next compare live keeper presentation and result handoff; suggested model Terra High.



## 2026-11-09 - Selected goalkeeper timing boundary

The same-cell volley replay matches active defender11 and one recorded random/travel event through bank26:$856B, random index149. Original keeper-cost scratch changes to20 and is not yet compared; keeper outcome remains unsupported. Full Windows build and all62 tests passed. Private replay: ../Captain-Majid-analysis/live-return-area-volley-keeper-precompare-20261109. Overall progress unmeasured; next compare keeper preparation and outcome; suggested model Terra High.



## 2026-11-09 - Same-cell volley goalkeeper arrival

The captured startup matches shared shot approach and the direct unstunned goalkeeper selection: distance18, cell234, elapsed1 to0, keeper11, with the previous defender reference retained. Same-cell and unstunned guards remain; goalkeeper decision is not yet connected. Full Windows build and all62 tests passed. Private replay: ../Captain-Majid-analysis/live-return-area-volley-keeper-arrival-20261109. Overall progress unmeasured; next compare keeper preparation/outcome; suggested model Terra High.



## 2026-11-09 - Recovered volley clocks and handoff

The captured startup now matches the unopposed volley's two clock steps, 22 ordered field movements and shot handoff, including the original timing of status6 and captured presentation samples. Final random index148; evidence: ../Captain-Majid-analysis/live-return-area-volley-handoff-v5-20261109. Shared clock, keeper recovery, contact prelude and movement kernels are reused; this sequence and event timing remain guarded and sampled. All62 Windows tests passed. Goalkeeper outcome and a complete match remain unsupported. Overall progress unmeasured; next investigate the direct same-cell keeper entry; suggested model Terra High.



## 2026-11-09 - Recovered goal-area volley preparation

The controlled startup chain matches48 route1 menu polls, shot confirmation, explicit observed `U 1` volley subtype and shared attack preparation: player8 stamina416 to326, cost90, power11 and random index7. Original later reaches a goalkeeper comparison and goal, not yet connected or native-compared. The independent banked attacker oracle passed3,882 cases; full Windows build, all62 tests and phase317 replays (576/1152 boundaries) passed. Evidence: ../Captain-Majid-analysis/live-return-area-volley-prepare-final-20261109. Overall progress unmeasured; next compare attack clocks and goalkeeper handoff; suggested model Terra High.


## 2026-11-09 - Recovered opponent goal-area menu

The startup replay now matches both goal-area defender scans, the no-keeper branch and route1 menu after152 captured NMI/travel pairs; player8 owns the ball and final random index is203. The original keeper entry rule was checked against12,320 controlled original cases using private extracted content. Goalkeeper-selected continuations and menu input remain guarded. Full Windows build, all62 tests and phase317 replay sets (576/1152 boundaries) passed. Private evidence: ../Captain-Majid-analysis/live-return-shot-area-menu-final-20261109 and ../Captain-Majid-analysis/goal-area-keeper-original-check-final-20261109. Overall progress unmeasured; next trace goal-area menu input and keeper-selected routing; suggested model Terra High.


## 2026-11-09 - Returned blocked-shot recovered owner

Connected result2 recovery through original-backed jitter,22 ordered field-player updates and nearest eligible player. The single captured startup matches represented state at $8BE5, $8C10 and $8C31: side0 player8 at188/124, stale cell234 and random index51. Opposing winner and boundary jitter reject; timing and earlier facing are supplied observations. Original later enters the opponent goal area, not yet native-compared. All62 Windows tests and phase317 replays (576/1152 boundaries) passed. Private evidence: ../Captain-Majid-analysis/live-return-shot-recovered-owner-final-20261109. Overall progress unmeasured; next compare area detection and menu; suggested model Terra High.


## 2026-11-09 - Returned blocked shot recovery entry

The controlled startup now matches the result2 handler through explicit presentation-facing sample, defender position, two experience calls, two-tick contact clock and cleared defender prefix at $8BDF. The original retains cell234 while ball x/y become164/132; native phase373 preserves that ordering. Final random index29. Original-only continuation jitters to188/124 and later chooses player8; it is not yet native-compared. Full Windows build and all62 tests passed; phase317 replays matched576/1152 boundaries. Evidence: ../Captain-Majid-analysis/live-return-shot-recovery-entry-final-20261109. Overall progress unmeasured; next compare jitter, interleaved field movement and owner selection; suggested model Terra High.


## 2026-11-09 - Returned shot-flight defender comparison

The startup chain now matches the selected defender sequence through result2: cursor reset, captured wait, player14 profile power8/cost70, strength6/loss1/score85 and restored variant1. Shared numerical preparation/comparison are reused; only the observed first action0/result2/variant1 path is exposed. An earlier separately guarded loose-shot set still matches12 original cases. All62 registered Windows tests and phase317 replays (576/1152 boundaries) passed. Recovery remains original-only. Private replay: ../Captain-Majid-analysis/live-return-shot-flight-defender-final-20261109. Overall progress unmeasured; next compare result2 handler and recovery; suggested model Terra High.


## 2026-11-09 - Returned shot continuous numeric flight

Connected the returned target to shared numeric flight, six cell steps and defender-selection callbacks, and E7CC arrival. One original startup matches every represented boundary through destination234 with candidates14/12 and random index88; timing and one motion sample are supplied evidence. A changed arrival coordinate reports the first differing field. All62 Windows tests passed and phase317 replays matched576/1152 boundaries. The result2 interruption and recovery remain original-only; full match and autonomous timing/AI are not established. Evidence: ../Captain-Majid-analysis/live-return-shot-flight-arrival-final-20261109. Overall progress unmeasured; next compare the selected-flight defender and recovery; suggested model Terra High.


## 2026-11-09 - Original blocked-shot recovery observation

After original defender14 result2, bank26:$8BDF was reached at frame15327 with selected count0 and ball164/132. At frame15350 the original snapshot shows side0 owner8 at188/124. These original-only boundaries do not verify native recovery, a control handoff or a complete match; instrument and compare the intervening recovery/race. Evidence: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-shot-interruption. Overall progress unmeasured; suggested model Terra High.


## 2026-11-09 - Returned shot flight numerical comparisons

Compared the shared native setup and ball/selection kernels to the private live continuation: direction/velocity255/256/0 matched at $E773; seven individual cell steps matched through234; six intermediate selections matched additions [], [14], [12], [], [], []. Inputs came directly from each original snapshot, so these are component comparisons, not a continuous native replay or verification of defender14 result2. Drivers and notes: ../Captain-Majid-analysis/shot-flight-setup-live-20261109. The integrated phase364 target and full62-test Windows suite remain the preceding verified checkpoint. Next connect flight events and compare full state; overall progress unmeasured; suggested model Terra High.


## 2026-11-09 - Returned shot target and interruption trace

Reused the shot-approach calculation in guarded phase364 and compared the original $DD08 target: distance63, destination234, counter255, non-loose and final random index81 matched. The replay verifies exact RAM equality at the handoff between two private traces and reports the first changed byte; a deliberate alteration reported RAM[0x635]. The continued original selected14 and12 in flight, then defender14 produced result2 at $88A8 before a keeper comparison. This interruption remains original-only, with native flight and resulting possession unverified. Full Windows build and all62 tests passed; previous phase317 sets matched576/1152 boundaries. Private evidence: ../Captain-Majid-analysis/live-return-shot-target-final-20261109. Overall progress unmeasured; next compare flight/interruption; suggested model Terra High.


## 2026-11-09 - Returned shot field movement

Reused the pass field-player movement kernel after the separately guarded returned shot dispatch. Twenty-two original random/travel pairs and player slots match the $C618 handoff in the live startup; full replay covers75 menu polls through that handoff, final random index81. A rerun's RAM matched the earlier run at all five sampled frames14900-15300. The original-only continuation reaches shot destination234/distance63 and later selects two players in flight; it does not establish native flight parity or a goalkeeper outcome. Full Windows build and all62 registered tests passed; phase317 replay sets matched576/1152 boundaries. Private evidence: ../Captain-Majid-analysis/live-return-shot-movement-final2-20261109. Overall progress is unmeasured; next compare target/flight and its selected-player interruption; suggested model Terra High.


## 2026-11-09 - Returned encounter shot through defender dispatch

The controlled startup continuation matched 75 action-menu polls through ordinary shot confirmation, explicitly supplied action1 defender choice, profile-based attacker/defender preparation, direct-table nonterminal comparison and contact/dispatch clocks. Player3 stamina385 to305, shot power8, defender16 strength1 and score255 matched; final random index59. The original resets its retained defender cursor before comparison and increments it at dispatch, so captured samples preserve that order. The separate earlier home-shot defender17 original set still matches24/24. A changed source identified the first differing boundary/field. Full Windows build, 62 CTest tests and phase317 replays (576/1,152 boundaries) passed. Only one defender/action1/result0 is supported; goalkeeper, scoring, live AI/presentation and a complete match remain unsupported. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Returned encounter action menu

Connected the directed startup's second encounter to the shared action-menu opening via phase355, preserving original cursor residue. The original/native chain matched 58 boundaries through the menu and held-input wait; its lead-in uses 136 recorded random/travel pairs and explicit motion/facing/display samples. No defender choice or frame scheduler is inferred. Full Windows build, 62 CTest tests and phase317 replays (576/1,152 boundaries) passed. Menu choice and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Directed returned possession reaches encounter

Captured a controlled rightward input after the startup free pass. From the saved returned-control point, 45 polls, five clock updates and five field rounds matched the original through a new encounter: player3 moved from x132 to x149 and met defender16; final random index247. Twenty-two overlapping control snapshots matched the shorter original directional run byte-for-byte. Phase354 preserves the retained cursor at the encounter without relaxing older guards. The input and NMI/travel schedule are supplied, not generated. Full Windows build, all 62 CTest tests and both phase317 replays (576/1,152 boundaries) passed. New encounter handling, AI/presentation and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Returned possession through three field rounds

Extended the live startup replay through ten neutral input polls and then 23 additional polls, three clocks and three field rounds (29 subsequent boundaries, final random index169). A second instrumented original trace had all 35 overlapping full RAM control snapshots byte-identical to the first. New phases352/353 preserve the original defender cursor while reusing control kernels with captured event ordering; unsupported menu/encounter continuations reject. An initial broadening of the old cursor guard admitted an invalid earlier scenario, and was removed after its regression caught it. A changed source correctly reported its first differing boundary and field. Full Windows build, 62 CTest checks and phase317 replays (576/1,152 boundaries) passed. Directed return input, autonomous timing/AI and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Startup free pass through control-ready

Composed the existing ball-step and loose-ball race kernels in bounded phase351 for an unopposed free pass with a home winner within two rounds. The startup original's two-round race selected players3/16 and player3 won; a complete represented-state comparison matched. The captured sequence from confirmed target through defenders, field movement, free pass, contest and control_ready matched using explicitly supplied original event ordering, speed/content, choices and presentation samples; final random index55. A changed player coordinate correctly reported the first differing boundary and field. An initial one-round guard rejected the observed two-round race and was corrected using the trace and earlier two-round race evidence. The 24 teammate cases also still matched through control_ready. Full Windows build, 62 CTest tests and phase317 replays (576/1,152 boundaries) passed. Free-pass timeouts/opponent wins/longer races, autonomous AI/timing and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary teammate pass through resumed control

Extended phases345-350 through empty flight callbacks, teammate arrival, possession, positioning and acknowledged control entry; the existing setup reaches control_ready. Twenty-four earlier original encounters matched every boundary through resumed control with explicitly supplied choices, fractions, timing and facing. Free-recipient arrival and defended callbacks reject pending evidence. Original cursor residue2 at the independent startup control entry and3 in the earlier teammate cases support bounded side-zero cursor preservation at control entry/ready; later phases retain their guards. Full Windows build, all 62 CTest tests and phase317 replays (576/1,152 boundaries) passed. Autonomous AI, timing/presentation and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary pass target and flight setup

Added phase344/version151 for resolved recipient/cell/distance and phase345 for original-backed initial numeric flight with explicitly sampled ball fractions. The startup free pass and 24 earlier teammate-pass cases matched original target and flight setup. An initial reuse of the existing flight phase relaxed an older scenario guard; its regression caught the mistake, and a dedicated phase restored that guard. Same-cell flight and continuation of the new flight phase remain unsupported. Full Windows build, 62 CTest tests and phase317 replays (576/1,152 boundaries) passed. AI, autonomous frame/presentation behavior and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Event-fed ordinary pass field movement

Extracted the atomic per-player field movement kernel and added phase343 for ordered player/random/travel events after the ordinary-pass dispatch clock. The startup matched all represented fields and random index167 at $C612 using 22 interleaved captured event pairs; 24 earlier original encounters matched the handoff without NMI events. No frame schedule is inferred. Full Windows build, all 62 CTest checks and phase317 replays (576/1,152 boundaries) passed. Target/flight continuation from this new phase, AI and a complete match remain unsupported. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary pass dispatch clock

Added guarded phase342 for the one-unit post-defender pass dispatch clock and keeper recovery, after all recorded defenders have completed result0 contact. One startup and 24 earlier original cases matched dispatch; startup matched three subsequent supplied random/travel pairs. The following original field movement is interleaved with those events per player, so a bulk step cannot establish live parity. All 62 Windows CTest checks and phase317 replays (576/1,152 boundaries) passed. Live AI, scheduling and a complete match remain unsupported; overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Repeated ordinary pass defender contacts

Added phases338-341 for observed nonterminal contact clock/experience and sequential defender preparation/comparison using shared kernels and explicit original inputs. The startup matched two defenders and their contact clocks with eight supplied intervening random/travel pairs; 24 earlier original cases matched all three defenders through their clocks. The live second comparison exposed a verifier error: X at $80D9 is not the strength; RAM $71 at $80D6 is. The first reported strength4 remains correct, and the second is2. Presentation later reuses $71. Unsupported results and clock continuations reject, and AI and presentation remain unimplemented. Full Windows build, all 62 CTest checks and both phase317 replays (576/1,152 boundaries) passed. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary pass first defender comparison

Extended the general ordinary-pass state to first-defender preparation and observed nonterminal comparison with explicit original ball variant, presentation samples and direct comparison row. The startup original matched defender19 (power8, strength4, score192) and 24 earlier original cases matched defender17 (power18, strength0, score255). An initial use of the different $8D95 selector coincidentally matched startup but failed the earlier zero-power case; the $8D93 direct row corrected it. Both free-pass variants1 and2 were observed, so neither is inferred from recipient. Defender choices remain supplied; additional defenders, other outcomes and live scheduling remain unsupported. Full Windows build, 62 CTest tests and phase317 replays of 576/1,152 boundaries passed. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary pass choice recording and preparation

Added score-independent phases333–335/version150 for ordered, explicitly supplied defender choices (currently action1/subtype0) and existing profile-based attacker preparation. The captured startup matched its two choices and preparation, owner9 stamina733→713 and power12; 24 earlier original cases with three defenders matched the same general operations. No defender AI is generated, and defender processing from the new phase remains pending. All 62 registered Windows tests and the 576/1,152-boundary phase317 regressions passed. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Ordinary target confirmation

Added phase332/version149 for an explicitly supplied A-only confirmation of a known free/opponent-only or single-teammate target. Multiple teammates stop at the original chooser, and old unknown candidate states reject. Six full original controlled confirmations matched across two contexts; two chooser cases rejected atomically. The captured startup additionally matched 25 polls and confirmed cell126 as free recipient255. Full 62 Windows tests, both phase317 regressions and the focused forged-recipient check passed. Live presentation/AI, cancellation, chooser and a complete match remain unsupported. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Persist original ordinary-pass candidate scan

Added a side-zero original candidate scan and version148 target context distinguishing an unscanned initial list from a scanned prefix. Older version146 targets remain readable with candidate state unknown. Two independent original target seeds matched 2,048 constructed CPU scans; 106 moved polls and 160 full cursor checkpoints matched, and the captured startup still matched 55 polls with the recorded opponent-only prefix. Confirmation and recipient chooser remain guarded. All 62 Windows CTest tests and both phase317 regressions passed. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - General ordinary-pass target movement

Added score-independent phase331 cursor movement, leaving confirmation/cancel guarded. Original D80F arithmetic matched 122,880 cases across two seeds/all cells/all held bytes; 160 full original/native cursor checkpoints matched. The captured startup replay matched 55 consecutive target polls including one move, using recorded NMI/travel and one explicit presentation motion-request sample. The sample is not an emulated presentation engine. All 62 registered Windows tests and the 576/1,152-boundary phase317 regressions passed. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - General ordinary-pass target initialization boundary

Added score-independent phase331 for an explicitly acknowledged ordinary pass type, using the carrier-position target kernel and existing extended snapshot format. The startup replay continued through 571 captured NMI/travel pairs and acknowledged submenu to the original initial target; 16 full saved-session comparisons over eight positions in this and an earlier encounter matched. Cursor movement and confirmation in the new phase remain unsupported, as do live presentation, frame scheduling and a complete match. All 62 Windows CTest tests and both phase317 regressions (576/1,152 boundaries) passed. Evidence remains private. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Captured ordinary pass selection and target arithmetic

Continued the startup's 68-boundary chain with 95 consecutive original menu polls through ordinary pass confirmation, with captured button, random and travel events. The next original pass-type submenu is not yet natively replayed. Added the pure ordinary target initializer: carrier high-byte coordinates refresh ball position and determine cell/cursor; ordinary mode/subtype become 129/0. Across the startup and an earlier independent encounter, 32 constructed original CPU cases matched eight positions and two fraction patterns with intentionally stale ball state. This kernel has no generic saved-session continuation yet. All 62 registered Windows tests passed; an initial build/replay concurrency attempt hit an executable lock and was rerun successfully in sequence. Earlier phase317 replays matched 576/1,152 boundaries. Evidence remains outside Git. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Original encounter to menu with captured NMI events

Added explicit ordered random/travel sequence handling at field-contact and human-menu boundaries and extended the private-path continuous verifier through acknowledged menu entry and neutral wait. One startup now matches 68 sequential original boundaries without reseeding, including 197 supplied random/travel pairs through the menu interval. A separate original input probe selected pass target with down then confirm; native comparison beyond the neutral menu remains pending. All 62 registered Windows tests and the 576/1,152-boundary phase317 regressions passed. Evidence stays in ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/control-chain-menu-neutral-20261109. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Captured control chain through encounter

Split the existing normal control-clock continuation into logical clock, field-cycle and stamina milestones, and added an evidence-fed operation that applies explicitly ordered random/travel events between them. It rejects unverified clock and goal-area branches; it does not generate frames, sound, presentation or interrupt individual kernels. The new private-path verifier continuously feeds 55 polls, five clocks and five field rounds from one startup through the first two-defender encounter, checking 65 original boundaries, represented fields and random index without reseeding. This is not yet connected to the separately verified phase317 pass/reception sequence. All 62 registered Windows tests passed, 12 earlier clock outputs remained byte-identical, and phase317 replays matched 576/1,152 boundaries. Evidence stays in ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/control-chain-verifier-20261109. Overall progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Explicit NMI control poll and later field-round boundaries

Composed supplied NMI random inputs, travel and control polling into a bounded reusable operation and added a private-path original trace verifier. Fifty-five independently seeded observed polls matched 322 original-derived bytes, canonical context and random index. Added one-based field-round selection and encounter-boundary comparison: five consecutive original rounds individually matched, with round five reaching an encounter. These are not a continuous replay. Captures show random and travel events occur during the clock and stamina routines, so correct interleaving is the next investigation. Windows build and 62/62 registered tests passed; phase317 possession replays matched 576/1,152 boundaries. Private evidence remains outside Git. Progress is unmeasured; suggested model Terra High.

## 2026-11-09 - Calculated field-round random from captured NMI inputs

The four existing directional field-round traces each contained22 original random calls, one before each observed player poll. In all four, the random index/bytes entering each call followed directly from the prior original player-step boundary. The native random kernel matched all88 captured calls and carry outputs using the original player/sound/mirrored-tail RAM samples and incoming carry. These four runs share a startup context and do not prove this ordering for all matches.

Added a separate `advance_observed_nmi_field_round` API and `--advance-observed-nmi-world-file FILE INITIAL_INDEX`. Each strict `CMJWORLDNMI1` record supplies buttons, the three original random memory samples, incoming carry and an optional travel event. The existing native random kernel calculates random before each poll; target-selection changes to the first random byte feed the following call. The final index is returned/printed separately and is not saved in the existing session snapshot. The mode does not derive event timing, memory/sound values or carry and does not emulate NMI, audio or presentation. The verifier `tools/verify_observed_nmi_world_round.py` checks private trace ordering/random continuity, compares original-derived session bytes and canonical context, checks final index and identifies the first differing field. Usage and format: docs/OBSERVED_FIELD_ROUND.md.

All four traces matched322 original-derived bytes, canonical context and final random index after22 polls: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/nmi-world-verifier-{right,up,down,left}. A deliberately altered random memory sample produced a named mismatch at motion[3].target. Nine invalid/truncated/conflicting input probes rejected atomically without source changes or output. Unit tests cover random-index wrap, early menu, interleaved target updates and parser/trace errors. The standalone original random oracle matched3,584 cases across all indices, both carry values and RAM mirroring. Forty-eight previous legacy world-round outputs were byte-identical; both existing phase317 pass replays matched576/1,152 boundaries: ../Captain-Majid-analysis/legacy-world-round-nmi-recheck and ../Captain-Majid-analysis/e2e-phase317-{25,36}-nmi-world. Full Windows CMake build and all62 registered CTest tests passed. Linux Make check remains unclaimed due to the previously documented unstable environment. Automatic scheduler, live sound/random inputs and presentation, general AI, full initialization and a playable match remain unsupported; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Compared an evidence-fed original field round

Traced the first original-emulator field round after kickoff through `$E407`,22 `$E412` polls and the next `$E145`. A native frozen-random batch using the captured input masks differed in37 represented fields at the final boundary, including random bytes and player targets/positions. The original updated random through NMI and ran the separate travel task during the multi-frame round. In the inspected capture each poll followed a recorded NMI and travel-task invocation; per-frame event inputs and order were checked, but this one ordering is not generalized into an automatic scheduler.

Added a separate `advance_observed_control_field_round` / `--advance-observed-world-file` path that reuses the existing round logic with explicitly supplied per-poll random bytes and optional observed travel events. The file parser requires exactly22 records, validates byte ranges and unused travel fields before execution, and rejects trailing data. It retains the existing menu early stop, does not alter old batch semantics or snapshot formats, and does not simulate NMI, frames, audio or presentation. The private-path verifier `tools/verify_observed_world_round.py` checks captured event ordering and reports the first differing represented field. Details and file contract: docs/OBSERVED_FIELD_ROUND.md.

Four direction probes from the same startup each matched322 original-derived bytes,80 canonical context bytes and the trailing context at the final control boundary across22 polls with captured random, speed, travel and inputs: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/observed-world-strict-final-{right,up,down,left}. All four share one initial player/context and cannot prove a general scheduler or complete match. Six malformed input probes rejected without modifying the source or creating output; registered tests cover parser errors, named mismatch, old batch equivalence and early menu behavior. Forty-eight earlier verified legacy world-round outputs remained byte-identical: ../Captain-Majid-analysis/legacy-world-round-recheck-20261109. Both existing phase317 pass replays still matched576/1,152 boundaries: ../Captain-Majid-analysis/e2e-phase317-{25,36}-observed-world. Full Windows CMake build and all62 registered CTest tests passed. Local Linux Make remains unclaimed because of independently reproduced toolchain instability. Overall progress is unmeasured; automatic scheduler, live audio/random/presentation, general AI, complete initialization and a playable match remain unsupported. Suggested model: Terra High.

## 2026-11-09 - Resolved control-travel speed from player profiles

Connected the existing control-travel arithmetic to the existing original-backed player profile mappings and selector32 speed content. `advance_profiled_session_control_travel` loads the ball carrier's speed on either side and applies the bounded travel step. Side0 with no held direction skips profile loading entirely; invalid active profiles reject atomically. The separate CLI `--advance-profiled-control-travel HELD` reports the loaded speed, or that no speed was loaded. The existing explicit-speed command remains available. Neither command schedules a frame, polls input, advances random, or simulates audio/presentation. Unit tests cover loaded speed, input preservation, invalid profiles, neutral short-circuit and side11 behavior.

Added `tools/verify_profiled_control_travel_original.py`, accepting explicit private ROM, selected-team pack, captured RAM and native driver paths. It executes the original `$E3CA..RTS` including the actual banked `$CE08` speed loader, and matched4,082 native/original speed and travel fixtures spanning ordinary types, alternate types and overrides, generic opponent slots, multiple levels, both sides/facing values and neutral side0 masks. This uses one supplied team context and constructed player records; it does not establish all real teams or live scheduling. An initial oracle draft read the speed after the original's division and correctly found a mismatch; it was corrected to capture the actual loader output at `$E3DE`. The profiled and explicit captured-speed session results were byte-identical for36 live directional polls in the existing startup; intervening random changes remain separately unsupported. Four malformed/repeated/mixed CLI operation probes rejected without mutating the source or producing output. Private evidence: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/profiled-direction-travel-20261109. See docs/CONTROL_TRAVEL.md.

The full Windows CMake build and all60 registered CTest tests passed. Both existing phase317 possession replays still matched576 and1,152 boundaries: ../Captain-Majid-analysis/e2e-phase317-{25,36}-profiled-travel. Local Linux Make check remains unclaimed due to previously documented toolchain instability. Automatic frame scheduling, live AI/random/audio/presentation, full native initialization and a complete playable match remain unsupported; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Derived explicit original control-travel step

Four controlled directional original-emulator probes initially exposed differences in the native travel accumulator between consecutive `$E145` control polls, in addition to expected random-byte changes. The original CPU poll with frozen frame work matched all322 represented native bytes for each direction, showing that the remaining live difference arose outside the foreground poll. A private original-emulator write trace located the accumulator updates at `$E3FC/$E403` in the `$E3CA..$E406` task. The original gates home-side updates on any directional held bit, loads a speed via `$CE08`, divides it by4, then adds the result if facing bit6 is set or its 16-bit one's complement if clear. Side11 does not use the home input gate. The reverse operation is one's complement, not arithmetic negation.

Added a reusable supplied-speed `advance_control_travel` kernel and an explicit control-ready session/CLI step. No frame, random, audio, input polling or presentation work is implied by that operation; its caller supplies the speed and input and chooses when to invoke it. The original CPU comparison executes the gate and arithmetic while explicitly substituting the speed-loader result. It matched132,608 fixtures, including all65,536 speed words in both facing directions plus input/side/travel boundaries. Four live-emulator direction probes captured13 speed loads and13 travel results each; all52 loaded speed928 and all52 native arithmetic results matched the instruction-boundary outputs. From four actual direction runs, nine consecutive polls each matched every represented non-random field after applying native poll plus the explicit travel step; random bytes remained different due to actual intervening NMI. This is one player and startup context; it does not validate automatic speed sourcing or a general frame scheduler. Private evidence: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-travel-trace and ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/live-direction-travel-20261109. Documentation: docs/CONTROL_TRAVEL.md.

The compact original control trace emitted the same six presentation requests, in order, as the native prepared-control result for this case. An initial verbose diagnostic was halted after100,367 visits to the wait address and47 NMI entries because repeated busy-loop visits are not frame completions. The compact capture recorded one wait-address visit per frame plus the62 NMI/random calls for this interval; this does not establish a universal scheduler rule. The instrumented control setup and same-toolchain uninstrumented run matched per-frame RAM, serialized-state hashes and input-callback hashes; separate directional comparisons matched per-frame RAM. No ROM, emulator or capture data was committed. Focused unit/session tests and invalid CLI probes passed; full Windows CMake build and all60 registered CTest tests passed. Existing phase317 replays still matched576 and1,152 boundaries. Linux Make check remains unclaimed due to the separately documented unstable environment. Complete native initialization, live AI/random/audio/presentation and a playable match remain unsupported; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Traced live control setup and per-frame random calls

In the original-emulator kickoff run, captured $E0DF control entry and the first $E145 control-ready boundary. Native `--setup-control` from the actual entry matched320 of322 represented snapshot bytes; random bytes $E2/$E3 differed, from66/222 to107/69 in the original while native preserved the entry values. Running the original CPU from the same entry with the existing cooperative presentation observer and no NMI matched all322 represented native bytes, including unchanged random. This separates an actual frame/presentation difference from the numerical control setup; it does not validate the original presentation engine in native code.

Instrumented a separate private build of the same original-emulator source to log frame IDs, $C500 NMI entry, random routine entry/return and captured 2,048-byte RAM. Between the original $E0DF and first $E145, the single observed run had62 calls to $C9C5, once in each of62 consecutive emulated frames; each was preceded in that frame by $C500 and returned to $C7E7. Feeding the recorded index, random bytes, incoming carry and three actual RAM samples to the existing native random kernel matched all62 original index/first/second/carry outputs. Source bytes varied:9 distinct player-region,17 sound-region and38 mirrored-tail values; incoming carry was1 in these62 calls. This is not evidence all game frames follow the same path or carry. The instrumented and same-toolchain uninstrumented runs matched per-frame RAM, serialized-state hashes and input-callback hashes across13,600 observed frames; no physical hardware or all hidden internal states are proven equivalent. Private evidence and binaries: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-nmi-trace, ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/live-control-comparison-20261109.

The next general missing mechanism is live frame/presentation scheduling with random index, carry and evolving memory/sound inputs represented or explicitly supplied. Do not infer random values from frame count or claim full playback from a captured acknowledgement. The phase317 replay and defender consolidation remain at their verified scope; no native code changed in this investigation. Last full Windows build and all59 registered CTest tests passed at the prior code checkpoint. Complete native initialization, live AI, timing/audio/video and a playable match remain unsupported; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Generalized prepared kickoff flag and pre-placement ball validation

The original startup trace showed kickoff flag0 and ball coordinates0,0 at $DAF5, before selection and placement; prior saved post-goal states used flag4 with an already interior ball. On the captured original CPU contexts, independently executed the original formation, selection, placement and post-presentation handoff with all256 flag values. Formation matched the88 player coordinate bytes and preserved the flag (other RAM changed across the real traced interval and was not claimed); selection, placement and handoff each matched all2,048 original captured RAM bytes with the substituted flag preserved. This tests those isolated instructions only, not arbitrary validity of every possible flag in a whole match.

Changed the general phase61..64 validation to accept the two observed flag values0 and4 without checking score, and to allow out-of-field ball coordinates in phases61/62 before placement. Phase63 retains its original requirement that the ball be at the carrier in field; unsupported flags and invalid placed-ball states still reject. Focused tests first failed on each prior rejection, then passed with the generalized guards, check flag preservation and saved transitions, and retain negative validation. No new score-specific path was added.

From the actual instrumented original startup, receiver selection, numeric ball placement and handoff each matched322 represented native session bytes at the original instruction boundaries. The handoff was seeded with a separate actual $DB13 capture after presentation; it is not an emulation of the intervening display or a continuous end-to-end startup. Private evidence: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/live-kickoff-boundaries-v3-20261109 and ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-trace-regs. Prior verified kickoff data remained byte-identical across144 transitions in48 cases; both phase317 replays matched576 and1,152 boundaries: ../Captain-Majid-analysis/kickoff-earlier-guard-recheck and ../Captain-Majid-analysis/e2e-phase317-{25,36}-post-kickoff-guard. Full Windows CMake build and all59 registered CTest tests passed. The local Linux toolchain remains unreliable as previously documented. Complete native startup, live AI/timing/presentation and a playable match are unsupported; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Original instruction-boundary kickoff capture

Built the recorded official FCEUmm source revision privately on Windows, then added private CPU-boundary logging and 2,048-byte RAM captures at selected fixed-bank kickoff addresses. The stock and instrumented same-toolchain builds matched all13,600 frame RAM buffers, serialized-state hashes and input-callback hashes in this one run; this does not prove equivalence of every hidden emulator field or physical hardware. The official prebuilt binary and local builds have separate private hashes. Initial local build failed because the Windows linker rejected the upstream GNU version-script flag; a private build retry omitted that flag and succeeded. No emulator source, compiled core, ROM or captured data was committed.

The original emulated CPU trace observed $DB4B->$DB61 with team3/random151 yielding side0, then $DAF5->$DB0B with side0/random171 yielding receiver9. Both existing native selection kernels returned those same values. $DB0B->$DB0E placed the ball at124,124/cell113 while initial owner8 remained; after the original presentation interval, $DB13->$DB1E handed owner9 control and set controlled player1. These are narrow original observations, not a native comparison of that entire sequence or actual native presentation. Source diff, binary hashes, trace log and boundary RAM are private: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/built-trace-ram.

Tried exporting the actual $DAF5 capture into saved native phase61 and running receiver selection. The current native phase validation rejected it because actual startup retains kickoff flag0 and the prepared restart path requires4. Kept the guard; do not change the captured flag or introduce a score-specific exception to manufacture a pass. The existing formation and selection kernels have narrow live-emulator evidence; a general startup/session contract, authentic presentation, full timing and complete playable match remain unsupported. Last full Windows build and all59 registered CTest tests passed at the preceding code checkpoint. Overall progress is unmeasured; suggested model: Terra High.

## 2026-11-09 - Compared emulated startup formation with native

Two fresh original-emulator runs and two independent resumes from the same earlier savestate provided frame12960 before formation placement and frame12961 after it. Used the captured side0, both formation0 values, captured prior raw X/Y words and the private formation table as inputs to the existing native kickoff formation kernel. All44 raw coordinate words matched per capture, including the two keeper aliases:176 comparisons. The four captures are not four independent initialization conditions, and frames are not instruction-boundary traces. This validates one observed emulated placement only; it does not validate recipient selection, native complete initialization, physical-console timing or a complete playable match.

Added `tools/compare_kickoff_formation_capture.py`, a read-only private-evidence comparator with explicit paths, side and native batch driver. It validates exact RAM and table sizes, cell range and stable formation metadata, and reports the first mismatching player/axis. An altered private copy reported player3.x correctly; focused tests cover matching, named mismatch, truncated RAM, invalid table and changed formations. The new test is registered in CTest and Make check. Full Windows CMake build and all59 registered CTest tests passed; the independently unstable local WSL environment still prevents claiming a full Linux Make check. Private evidence: ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/formation-compare-20261109.

A separate repeatability probe restored the same original-emulator savestate in two fresh processes: the subsequent8,477 frame RAM captures, serialized-state hashes and input-callback hashes matched. Comparing one resumed run with its uninterrupted source first differed at frame13187; the reason remains unresolved, so repeated savestate results do not prove fresh-run determinism or identical uninterrupted behavior. No emulator code or game-derived data was committed. Overall progress is unmeasured; next inspect captured formation/ball and control handoffs without adding score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Narrowed emulated startup repeatability difference

To investigate the prior frame-sampled variation, ran additional fresh processes saving all13,600 internal-RAM frames, then probes recording serialized emulator-state hashes and input-callback histories per frame. One stream pair first differed in RAM at frame2935; another pair had identical input-callback history hashes across all frames but differing state hashes and RAM. A full serialized-state pair was byte-identical through frame5123, then differed at frame5124; the recorded input-callback hashes matched there. The same saved frame5123 state was independently restored and advanced1,000 times: every run reproduced the first original run's state/RAM hashes, not the second run's state hash. Another checkpoint also produced one outcome over1,000 restores. Double-reading RAM immediately after each frame showed no reported differing read in the sampled pair. Separately,100 Windows Python one-million-iteration product counts passed; this does not rule out other emulator/host/environment defects.

These experiments establish a reproducible difference in fresh runs under this private harness, not its cause or a defect in the game reconstruction. Serialization may omit relevant state; host and environmental effects have not been ruled out. Do not discard differences by assuming they are stack-only or claim exact frame replay. Core source for the exact recorded build and all generated evidence were kept outside Git in ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee; the core's documented RAM initialization alone does not explain the observations. Stage0/team3/$BAF6 remained corroborated only at the previously sampled frames. No engine changes; last full Windows build and58 registered tests passed at the preceding code checkpoint. Native live initialization and a complete playable match remain unsupported. Overall progress is unmeasured; suggested model: Terra High.

## 2026-11-09 - Confirmed one emulated stage/team context

Read the stage byte $26, opponent-team byte $2B and retained pointer $38/$39 in each sampled frame12900..13600 (10-frame spacing) of all four independent emulated startups. All284 samples held stage0, team3 and $BAF6. This matches the previously isolated original stage-to-team/pointer instruction traces for stage0 and establishes the context in this specific emulated startup. It does not show when or how the pointer was assigned, validate other stages, identify the keeper loader at a later decision, or compare native whole-match initialization. The four captures have the separately documented RAM and picture variability; selected bytes matching does not make the complete run deterministic. Evidence remains private in ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee/capture-{c,d,e,f}. No engine change. The last full Windows build and all58 registered CTest checks passed at the preceding code checkpoint. Overall progress is unmeasured; suggested model: Terra High.

## 2026-11-09 - Isolated emulated original startup evidence

Ran the user's original Japanese ROM in a newly isolated headless FCEUmm/libretro process, using an official prebuilt Windows core obtained from the libretro buildbot; private provenance includes hashes. A private scripted input sequence passed the title, story and meeting, selected kickoff and reached field play and a first player encounter. The emulator advanced frames including its normal CPU/PPU/NMI processing; this is an emulated observation, not evidence from physical hardware. The existing separate emulator session was not accessed. The ROM and all generated RAM, pictures, runtime binaries and scripts remain outside the checkout at ../Captain-Majid-analysis/libretro-isolated-6d1ef9ee.

Four fresh processes were sampled every ten frames from12610 through13600 and at ten picture frames. In one pair,45 of100 RAM snapshots differed, one of10 pictures differed and only $053F among addresses at or above $0300 differed in one frame. In another pair,67 RAM snapshots and one picture differed, while sampled bytes at or above $0300 matched. Differences also occurred below $0300. These measurements do not establish why the runs differ, whether lower addresses are irrelevant, or full-run deterministic behavior. An initial capture attempt collected only pictures because its insertion pattern did not match; the corrected capture produced the stated RAM samples. No match state is claimed to be synchronized to an instruction boundary.

A spot comparison of frame13000 against a constructed stage0/team3 post-keeper control checkpoint found many differences in roster, energy, positions and possession. The checkpoints are at different boundaries and the constructed state uses synthetic roster data; the differences do not prove a native defect or parity. Next establish matching startup or formation boundaries and compare named represented fields, while retaining frame/timing uncertainty. The last full Windows build and all58 registered CTest checks passed at the preceding code checkpoint; no engine behavior changed in this investigation. Overall progress is unmeasured, and native live initialization and a complete playable match remain unsupported. Suggested model: Terra High.

## 2026-11-09 - Shared recipient target-cell resolution

Added `resolve_pass_target_cell` for explicitly supplied field recipient IDs1..10/12..21 and sentinel255. It reuses the existing field-coordinate mapping, returns255 for out-of-field positions, preserves the previous byte for sentinel255 and rejects unsupported IDs. `tools/verify_pass_target_original.py` compared original `$E059` against native for all65,536 coordinate-byte pairs for player7,2,000 boundary fixtures across all20 field IDs and all256 prior target bytes for sentinel255:67,792 matches. Low position bytes were deliberately varied in the original observations. This checks target-cell output only and does not model original scratch/register effects or choose a recipient.

Added opt-in `--resolve-sampled-field-pass-target` and replay `--resolve-recipient` to combine the resolver with the private-table distance kernel and existing guarded capture, preserving earlier explicit/calculate commands and snapshot formats. The two existing phase317 replays matched576 and1,152 original/native boundaries and their1,656 generated states were byte-identical to the prior calculated-distance runs. Twenty-four earlier no-recipient same-cell pass targets also matched: ../Captain-Majid-analysis/mixed22-p3target-resolved-771ca9f2. Unsupported recipients and corrected out-of-field probes failed without changing inputs or creating outputs; the field-recipient path ignored a different previous target and matched the original checkpoint. The first out-of-field diagnostic changed the Y fraction byte instead of X high byte and therefore did not test an out-of-field position; the corrected private probe is ../Captain-Majid-analysis/pass-target-corrected-probes-d8c0006e. All58 registered Windows CTest checks passed.

The resolver is shared within its stated ID domain; recipient/AI choice and the saved encounter transitions remain supplied and scenario-constrained. Original CPU fixtures are constructed, not authentic startup or live presentation/timing; no complete playable match is established. Local WSL tooling remains independently unstable as documented in the previous entry. Overall progress is unmeasured. Continue general evidence-backed work without expanding to new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - General bounded pass-distance calculation

Implemented the original-backed `$DF8B` pass-distance calculation as a pure C++ kernel accepting an explicit replaceable 20-byte private table. For an in-field ball and target cell, it uses the target center, larger unsigned absolute coordinate difference and an eight-pixel distance band. Exhaustive controlled original/native CPU comparison matched all3,686,400 combinations of ball x48..207, y80..175 and target cell0..239. `tools/verify_pass_distance_original.py` supports reproducible sampled and streaming exhaustive checks; these compare the distance output, not unrepresented original scratch registers or live frames. The private exporter verifies ROM identity, writes `CMJPDST1` only outside this checkout and uses exclusive creation; a strict native reader rejects bad magic, truncation and trailing bytes. The new extension is ignored by Git. No ROM bytes or derived table were committed.

Added an optional `--calculate-sampled-field-pass-target` phase-session operation and `--distance-table` replay option. The operation calculates distance but still uses the existing guarded captured recipient/target and does not generalize AI selection. Both existing phase317 replays matched576 and1,152 original/native boundaries; all552 and1,104 generated snapshots respectively were byte-identical to the earlier explicit-distance runs. Missing, malformed and incompatible table probes rejected without changing the input session or creating output. Previous explicit-distance operations and pack/snapshot formats are unchanged. Original data: ../Captain-Majid-analysis/pass-distance-20261109.cmjpdist; private replay evidence: ../Captain-Majid-analysis/e2e-phase317-25-calculated-distance and ../Captain-Majid-analysis/e2e-phase317-36-calculated-distance; failure probes: ../Captain-Majid-analysis/pass-distance-failure-probes-a5dd648d. Focused C++/Python tests and all56 registered Windows CTest checks passed. The first CTest attempt exposed a py65 dependency leaking into the table exporter; reference identity was separated into a lightweight module and the full suite passed afterward.

An isolated local WSL Make attempt exposed unstable system tooling. GCC15 at `-O2` internally crashed compiling the unmodified baseline CLI; its `ld.bfd` failed4 of30 independent trivial C++ link probes. A fresh `-O1` build passed serially after retry, but Make check did not complete reliably: Python3.14 failed8 of20 independent product counting probes and1 of12 plain-range probes, with inconsistent counts or corrupted local variables. Ten equivalent Windows Python3.12 probes passed. No project logic was altered to hide these failures, and no Linux full-check success is claimed. Further diagnosis or a stable Linux toolchain is needed. Overall progress is unmeasured; recipient and AI selection, real initialization, live audio/video/timing and a complete playable match remain unsupported. Next continue general evidence-backed work without new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Reverified shared action, keeper and field kernels

Ran existing isolated original-ROM comparison harnesses using the private verified ROM and current Windows build. Results matched: action strength6,656 cases; skill-index69,632, action-power7,456 and goalkeeper5,248; energy9,728, keeper-story6,812, keeper-state5,632 and random-step3,584 (including all256 random indices, both carries and $0800 RAM mirroring); shot goalkeeper-outcome384; defender result-three handoff144; ball recovery512; defender selection1,024; field interruption27,904; and banked attacker3,882 cases. The interruption fixtures exercised1,173 resets,5,571 field routes and1,150 goal-area routes; the banked attack checks14 represented outputs and bank restoration. These are controlled constructed routine inputs, not a verified live game, AI choice, presentation scheduler or match startup. No engine code changed; the full53 registered Windows CTest checks passed at the preceding code checkpoint. ROM and derived private material remain outside Git.

The shared numerical kernels have broader isolated evidence than the two narrowly guarded phase317 replay routes. A complete playable match remains unsupported; overall progress is unmeasured. Continue reproducibility, verification and evidence-backed shared work without expanding into new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Reverified shared defender calculations against original routines

Using the private verified original ROM and current Windows build, reran four existing non-scenario-specific original CPU comparison harnesses. `verify_field_defender_preparation.py` matched16,064 cases for selected skill/index, strength, effect, cost and player pointer. `verify_field_player_profiles.py` matched91,392 synthetic ordinary/alternate profile-selection and preparation fixtures. `verify_field_comparison_selection.py` matched550 selection, arithmetic and classification fixtures. `verify_defender_contact.py` matched all eight outputs for19,200 cases, with6,511 special,4,584 weakened,3,093 zero-strength and1,111 loss-branch cases. These are controlled constructed routine inputs, not proof of which choices or profiles occur in a live match. No engine code changed; all53 registered CTest tests passed at the preceding checkpoint. Original ROM and any derived private data remain outside Git.

The numerical/profile routines have broader evidence than the bounded phase317 encounter replay; arbitrary AI decisions, live presentation, hardware timing, authentic initialization and a complete playable match remain unsupported. Overall progress is unmeasured. Next continue evidence-backed verification and shared work without expanding to new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Replay integrity safeguards

Hardened the existing phase317 replay to reject truncated, invalid-magic and unsupported-layout snapshots before reading fields; improved pending-contact field names; tested that output cannot be created inside the checkout or overwrite a nonempty evidence directory. An optional `--expected-cases` rejects an incomplete input set before any output is created. Focused tests cover these safeguards and replay diagnostics. Both established replays remain unchanged at576 matching boundaries for24 cases and1,152 for48; private outputs: ../Captain-Majid-analysis/e2e-phase317-25-complete and ../Captain-Majid-analysis/e2e-phase317-36-complete. Two separate earlier runs generated exactly matching SHA-256 hashes for all552 and1,104 native checkpoints respectively. All53 Windows CTest checks passed.

The verifier remains bounded to the two controlled encounter shapes and does not claim live AI, timing/audio/video, authentic match initialization or a complete playable match. Overall progress is unmeasured. Next continue verification and evidence-supported shared work without expanding to new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Replay metadata and reception audit

Extended the phase317 replay to compare saved flight distance, destination, counter, loose-ball flag, target recipient/cell and full trajectory during both owner transfer and recipient placement, and to check consumed flight contexts are cleared at resumed control. The original reuses its scratch bytes after field movement, so the retained defender contact at handoff and target is now compared against the corresponding original third-defender preparation snapshot, not a hardcoded value. Added focused diagnostics tests for a changed flight counter and retained contact cost. The stronger replay still matched576 boundaries in24 existing 2-5 cases and1,152 in48 existing 3-6 cases: ../Captain-Majid-analysis/e2e-phase317-25-evidencebased and ../Captain-Majid-analysis/e2e-phase317-36-evidencebased. All53 registered Windows CTest checks passed.

This is an audit of the existing constructed traces, not additional gameplay coverage. Supplied choices, target, comparison and fractional inputs remain constrained; live AI, timing/audio/video, authentic initialization and a complete playable match remain unsupported. Overall progress is unmeasured. Next strengthen verification and shared evidence-backed components without expanding into new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Phase 317 replay and evidence-backed defender consolidation

Added `tools/verify_player7_pass_end_to_end.py`, documented in `docs/REPLAY_PLAYER7_PASS.md`, to read private evidence/content paths and replay continuously from phase317 through the next two defender preparations, comparisons and clocks, dispatch, field handoff, target, numeric flight, reception, placement and resumed control. It compares represented session fields and relevant score, formation, contact, flight flag, target and trajectory fields, stops at the first mismatching case/boundary/field, and refuses output inside the checkout or into a nonempty directory. A deliberately changed private copy of the phase317 original ball X stopped with `case 0, boundary phase317, field.ball_x`; no original evidence was modified. The first draft of the comparator mistakenly reused original scratch bytes after the field update had overwritten them; it now checks the previously verified retained contact values there. The unmodified original evidence passes.

Consolidated duplicate profiled defender preparation for the existing first, second and third sampled defenders while preserving original phase guards, state transitions, byte arithmetic and error paths. After the refactor, the end-to-end replay matched576 boundaries across24 previously verified 2-5 cases and1,152 boundaries across48 previously verified 3-6 cases. A separate earlier 2-2 player3 encounter matched24 original/native defender preparations and24 comparisons: ../Captain-Majid-analysis/mixed22-p3defnative-863d1d03. Private replay outputs: ../Captain-Majid-analysis/e2e-phase317-25-refactored and ../Captain-Majid-analysis/e2e-phase317-36-refactored. Focused tests cover comparison naming, phase-byte offsets, truncation, discovery and preparation consistency. The Windows CMake build and all53 registered CTest checks passed, including CLI, adapter and loopback checks. CTest was installed in the local private Python environment because the existing Windows CMake bundle lacked it; no credential or private data was added to Git.

Shared calculations do not imply general gameplay. The replay remains constrained to observed participants, choices, target, comparison inputs and saved phases, with captured fractions and acknowledged presentation. Arbitrary AI choices, live timing/audio/video, authentic match initialization and a complete playable match remain unsupported. Overall progress is unmeasured. Next continue work for the authorized eight-hour interval on verification, durable replay and evidence-supported consolidation, without expanding into new score-specific scenarios. Suggested model: Terra High.

## 2026-11-09 - Controlled four-defender shot reaches 4-6

The sampled player7 shot at 3-6/clock269 captures the original four-defender choices and uses profile-calculated preparation: stamina140 to60, power22. All48 constructed cases match choice capture and preparation; four defender preparations/comparisons/contact clocks, four observed prefix boundaries and dispatch match816 boundaries. Dispatch reaches clock263. Numerical field handoff and target match48 each; target is destination233/distance99. Fractions223/75, velocity and 12 cell crossings match1,248 setup/crossing/callback/arrival boundaries. Keeper entry and preparation match48 each. With team3 explicitly set and supplied keeper choices0/1, all96 original/native comparisons and goal registrations match; score4-6. The shot continuation reuses the existing numerical algorithms but remains narrowly guarded to the observed score, clock, participants and selected choices. Original shots: ../Captain-Majid-analysis/mixed36-p7shotcontinue-original-all and ../Captain-Majid-analysis/mixed36-p7shotflight-original-all; native defenders: ../Captain-Majid-analysis/mixed36-p7contacts-native-f5e4ab15; flight: ../Captain-Majid-analysis/mixed36-p7shotflight-native-81153ae4; keeper: ../Captain-Majid-analysis/mixed36-p7keepercompare-native-146c270c; goals: ../Captain-Majid-analysis/mixed46-goal-native-8d6f53e0. One initial trace summary looked for an unexecuted $DD17 hook and reported missing files; the actual $DD08 target snapshots exist and all48 match. Focused regressions and full Windows CMake build/all45 tests passed.

Next compare goal cleanup, restart and kickoff. Original states are constructed; both keeper choices and presentation acknowledgements are explicit inputs. No live match initialization, real AI choice, audiovisual timing or complete playable match is established. Overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - 3-6 player7 pass and next contact

Matched48 player5 pass field handoffs and saved targets. Target distance3 was observed in a representative original CPU trace; native saved recipient7, cell103 and counter255 in48 cases. Numerical fractions82/109 and trajectory/velocity calculations match480 setup, four crossing/callback pairs and arrival boundaries. Receipt, recipient placement and control handoff each match48; setup matches48, with player7 at113,143. The native path preserves cursor3 through receipt. Another 5,280 neutral polls, 480 clock advances and 480 field rounds match48 up to the four-defender player7 contact at clock269, selected12,15,16,21. Menu opening, ordinary-shot selection and confirmation match48 each. A representative original-only trace continues through all four defenders to dispatch at clock263; native shot continuation for this situation remains unimplemented. Evidence: ../Captain-Majid-analysis/mixed36-fieldhandoff-native-cc93ff4d, ../Captain-Majid-analysis/mixed36-target-native-5b1508ac, ../Captain-Majid-analysis/mixed36-flight-native-check1, ../Captain-Majid-analysis/mixed36-reception-native-2c5ee1a2, ../Captain-Majid-analysis/mixed36-player7ready-9c33e669, ../Captain-Majid-analysis/mixed36-p7path-4888583e and ../Captain-Majid-analysis/mixed36-p7menu-ceab6d1d. The initial target comparator mistakenly read the final distance byte as flight flag; corrected version147 offset checks pass48 and the failed assertion is retained as private diagnostic, not claimed as parity. Full Windows CMake build and all45 test executables passed.

Next compare shot preparation and all four contacts using controlled original traces. Constructed inputs, explicitly supplied choices and acknowledged presentations are not live initialization, audio/video or frame timing. A complete playable match remains unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - 3-6 restart to three-defender pass dispatch

Matched48 original/native goal presentations and restarts, 48 each kickoff formation, receiver, ball placement, handoff and control setups. From kickoff, 5,280 neutral polls, 480 clock advances and 480 field rounds match to player5 contact at clock283. The original action menu opens in48 cases; seven button masks and the explicitly selected pass confirm match48 each. Target opening, five cursor moves and teammate7 confirmation match48 each. With observed supplied defender choices [1,1,1], capture and numerical pass preparation match48 each. Each of three defender preparations, comparisons and contact clocks plus dispatch match48 (480 boundaries). Crucially, player5 starts with10 stamina here, so preparation now writes the computed zero stamina rather than relying on the previous zero input. Private evidence: ../Captain-Majid-analysis/mixed36-restart-690f73cc, ../Captain-Majid-analysis/mixed36-kickoff-native-085afe4b, ../Captain-Majid-analysis/mixed36-p5path-93589ba8, ../Captain-Majid-analysis/mixed36-teammate-native-07317f3c, ../Captain-Majid-analysis/mixed36-contacts-native-a1fc40a4. Focused regressions cover the updated score/stamina and malformed goal states; full Windows CMake build and all45 test executables passed.

Next compare pass field handoff and numeric flight. These are constructed states with supplied choices and acknowledged presentation, not a live match or authentic timing, initialization or audiovisual output. A complete playable match remains unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Controlled player19 shot and 3-6 goal

Continued 48 constructed 3-5 states through 1,584 neutral polls, 96 clock ticks and 96 world rounds to player19's observed shot decision. The captured decision/preparation, presentation acknowledgement, clock, calculated field movement and handoff each match48 original boundaries. Target distance36/destination5 and fractions66/124 match48 target and 768 numeric flight setup, crossing, callback and arrival boundaries. The prompt, explicitly supplied ordinary goalkeeper action, confirmation, overlay acknowledgement, comparison and goal each match48; goalkeeper stamina falls from84 to64 and score becomes3-6. Original shot flight: ../Captain-Majid-analysis/mixed35-p19shotcells-original-a956ff5a; native flight: ../Captain-Majid-analysis/mixed35-p19shotcells-native-0054cda9; original keeper/goal: ../Captain-Majid-analysis/mixed35-p19keeperresult-original-9e341ef0; native keeper: ../Captain-Majid-analysis/mixed35-p19keeperresult-native-8861cf14; native goal: ../Captain-Majid-analysis/mixed36-goal-native-5d0e58d5. Narrow validation rejects unsupported later phases and malformed keeper stamina. Full Windows CMake build and all45 test executables passed.

Next trace goal presentation, restart and kickoff. Original input states are constructed; AI/keeper choices are supplied and presentation acknowledged, not live timing, real audio/video or authentic match initialization. Complete playable match remains unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Controlled 3-5 kickoff and opponent pass

- After the 3-5 goal, compared original post-presentation cleanup in48 cases (24 constructed states, keeper choices0/1 supplied). Opponent kickoff formation, receiver, ball, handoff and control setup each match48; presentation is acknowledged rather than rendered. Another4,752 polls,384 clock ticks and384 world rounds match, reaching player16 AI decision at clock296/cell140. Evidence: ../Captain-Majid-analysis/mixed25-p7postgoal-native-dd1848cc, ../Captain-Majid-analysis/mixed25-p7kickoff-native-44d7ddba and ../Captain-Majid-analysis/mixed35-opponentpath-3059a8c5.
- Captured the original action1 choice and preparation (subtype128/power21), acknowledged presentation, dispatch, numeric field movement and handoff each match48. The original handoff facing is0 rather than the previously sampled64; corrected the bounded branch with a focused regression. Target player19/cell70/counter255/distance4 matches48. Numeric pass starts fractions99/120 and matches768 setup, crossing, callback and arrival boundaries; receipt, positioning, control and setup each match48. Evidence: ../Captain-Majid-analysis/mixed35-p16ai-native-b75f9ecd, ../Captain-Majid-analysis/mixed35-p16field-native-4052781f, ../Captain-Majid-analysis/mixed35-p16handoff-native-0e388cda, ../Captain-Majid-analysis/mixed35-p16target-native-477b8e38, ../Captain-Majid-analysis/mixed35-p16flight-native-81661cc8, ../Captain-Majid-analysis/mixed35-p16receive-native-1ce3c68a and ../Captain-Majid-analysis/mixed35-p19ready-native-d44b1811.
- Full Windows CMake build and all45 tests passed. Website-only Arabic progress note was pushed to main at86b5f08; remote ref and hosted UTF-8 HTML verified, no browser render claimed. Next trace player19's no-input control and next interaction. AI choices were supplied; live timing/audio/video, authentic match initialization and a complete playable match remain unverified. Overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Player7 numeric shot and controlled 3-5 goal

- Added phase327 target and phases328..330 numeric flight using existing shot approach, ball velocity and cell-step routines. All24 constructed original/native target and setup states match; twelve cell crossings/callbacks and the final arrival match600 boundaries. Target233/distance99, starting fractions52/187, original flight flag0 and retained cursor5 were checked. Evidence: ../Captain-Majid-analysis/mixed25-p7shottarget-native-82ace1f8, ../Captain-Majid-analysis/mixed25-p7shotflight-original-12bb329c, ../Captain-Majid-analysis/mixed25-p7shotflight-native-ef8cc82c and ../Captain-Majid-analysis/mixed25-p7shotcells-native-30bdf0b3.
- Connected arrival and goalkeeper entry using existing routines;48 boundaries match across24 cases. For stage0/team3 with choices0 and1 explicitly supplied,48 original/native goalkeeper comparisons and48 resulting goal registrations match at score3-5. The first comparison exposed an overlooked original cursor reset during goalkeeper resolution; a focused regression caught it and the sampled branch now clears cursor5 to0. Evidence: ../Captain-Majid-analysis/mixed25-p7keeperentry-original-605e7143, ../Captain-Majid-analysis/mixed25-p7keeperentry-native-583c38bb, ../Captain-Majid-analysis/mixed25-p7keepercompare-native-bca87984 and ../Captain-Majid-analysis/mixed25-p7goal-native-4d157303.
- Full Windows CMake build and all45 tests passed. Next compare goal presentation, restart and kickoff. Goalkeeper choices were supplied, not generated by AI; original observations use constructed states and cooperative presentation rather than live NMI/audio/video timing. Authentic initialization and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Five-defender shot through numeric field handoff

- Reused defender prefix, profile preparation, direct comparison, contact clock and dispatch routines for player7's ordinary shot. All five defender slots12,14,15,16,18 compare as observed; each contact advances the clock one unit, then dispatch advances two. The original cursor progresses0..5 and remains5. The existing version142 saves the defender choices and intermediate context.
- Reused numerical field advancement after dispatch. Across24 constructed original/native cases, 528 compared boundaries match represented session bytes11..323, cursor and flag from defender prefix through $80FE field handoff; original $C618 has the same represented state. Evidence: ../Captain-Majid-analysis/mixed25-p7shotcontinuation-original-0e1183b5, ../Captain-Majid-analysis/mixed25-p7shotpostdispatch-original-664d8dd4 and ../Captain-Majid-analysis/mixed25-p7shotreplay-native-1cc815a2. Focused tests cover all five contacts, dispatch, numeric field advancement, rejection and saved prefix. Full Windows CMake build and all45 tests passed.
- A representative original-only continuation reaches destination233, distance99, then flight arrival. Next compare target/flight and trace goalkeeper outcome. AI choices remain supplied and presentation acknowledged; real initialization, live audiovisual timing and a complete playable match are unverified. Overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Player7 field contact and ordinary shot preparation

- Continued24 constructed player7 control cases with no input: 2,640 original/native polls,240 clock updates and240 field rounds match through a five-defender contact at clock311/cell103. The original retains cursor3 and flight flag1. Corrected validation to preserve this observed cursor through field, contact and action-menu states. Original/native menu opening, shot selection and confirmation each match24. Evidence: ../Captain-Majid-analysis/mixed25-p7path-f2a1d041 and ../Captain-Majid-analysis/mixed25-p7menu-native-cf2c311a.
- Captured the original five defender choices [1,1,1,1,1] and reused the profile-based ordinary shot preparation. All24 original/native boundaries match each operation: player7 stamina210 to130, cost80, power22 and subtype128. Existing version142 saves the five choices. Focused tests cover cursor preservation, choice roundtrip, rejected changed choices and shot preparation roundtrip. Evidence: ../Captain-Majid-analysis/mixed25-p7shot-original-bd15dda1 and ../Captain-Majid-analysis/mixed25-p7shot-native-585d5d51.
- Full Windows CMake build and all45 tests passed. An intermediate attempt to relink while the comparison repeatedly launched the executable was blocked by Windows file locking; rebuilding after the comparison ended succeeded. Next compare the five original defender results and subsequent shot route. AI choices are captured, presentation is cooperatively acknowledged; live timing, actual initialization and a full playable match remain unverified. Overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Numeric player7 flight and return to control

- Reused the numeric pass-flight calculation for the sampled home-side player7 route, guarded by score, owner, cursor, target and flag. All24 original/native setups match starting fractions167/221, direction and velocity; four cell crossings and the final arrival match across the cases. Recipient selection, ball positioning, player-control entry and control-ready setup each match24; player7 receives at117,139/cell103. The original retains defender cursor3 and flight flag1 through these states. Earlier defender AI choices remain supplied, presentation acknowledged.
- Added `tools/verify_player7_pass.py`, which replays setup, four crossings/callbacks, arrival, receipt, positioning, control and ready, comparing336 boundaries against private original snapshots across24 cases. It requires an empty output directory and does not embed private data. Private evidence: ../Captain-Majid-analysis/mixed25-arrival-original-f724c54d, ../Captain-Majid-analysis/mixed25-receive-original-70bc035f, ../Captain-Majid-analysis/mixed25-player7ready-native-b569c752 and ../Captain-Majid-analysis/mixed25-player7-replay-final-20261109.
- Full Windows CMake build, all45 tests and the replay passed. A replay attempt using an existing output directory was safely rejected by the CLI; the verifier now checks for an empty directory before starting. Next compare input polls and field rounds from player7 control. Real initialization, live timing/audio/video and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Player7 pass target saved

- Phase326 saves recipient7, destination103, counter255 and distance3, plus the existing cursor and step count. Added version147 to retain both contexts while preserving previous snapshot layouts. All24 constructed original/native target boundaries match represented session bytes11..323, target, cursor and flag. Focused tests cover roundtrip, wrong version, truncation, replay and changed recipient. Private evidence: ../Captain-Majid-analysis/mixed25-target-native-10821ce7 and ../Captain-Majid-analysis/mixed25-corrected-flight-original-c7504f3f.
- Full Windows CMake build and all45 Windows test executables passed. Next calculate flight and compare the five original callbacks and subsequent control; original flight remains unpaired. AI choices are supplied and presentation acknowledged, not live audiovisual timing. Real initialization and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Corrected post-pass flight observer

- Diagnosed the earlier bounded observer: it checked bank30 for $E773 and $E0DF, but those fixed addresses are in bank31. After correcting the hooks, all24 constructed original cases reach five $E773 callbacks and $E0DF; target recipient7 and retained flag1 were observed. The earlier 10,000-wait trace missed those events and does not establish an actual scheduler wait. Original-only evidence: ../Captain-Majid-analysis/mixed25-corrected-flight-original-c7504f3f; corrected observer: ../Captain-Majid-analysis/trace_mixed25_afterdispatch_corrected_20261109.py.
- Native comparison ends at phase325 field handoff; target, numeric flight and reception remain unimplemented. Next preserve the target in a compatible saved context, calculate flight and compare every original callback and return to control. The original runs use constructed states and cooperative presentation, not live hardware timing. A playable match and authentic initialization remain unverified; overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Numeric field movement after player7 pass

- Extended the existing numerical field advancement to phase325 after the three-defender dispatch. All24 constructed original/native cases match represented session bytes11..323, defender cursor and flight flag at original $C612; target cursor and step count are retained in version146. Focused tests exercise advancement, snapshot roundtrip and replay rejection. Private evidence: ../Captain-Majid-analysis/mixed25-fieldhandoff-native-d0fe5255 and ../Captain-Majid-analysis/mixed25-afterdispatch-original-6bff3082.
- The original reaches $DE64 with player7 as target in all24 cases. A representative observer did not reach expected flight/setup or control hooks after10,000 cooperative waits; this is not proof of an original-game stall or of its cause. Next retain target state compatibly and investigate scheduler/flight behavior using controlled observations. AI choices are supplied and presentation acknowledged; authentic initialization, live audiovisual timing and a complete playable match remain unverified. Overall progress is unmeasured. Suggested model: Terra High.

## 2026-11-09 - Player7 pass clears three defenders

- Extended the existing numerical defender preparation, comparison and contact-clock operations to defender19 and defender20 and advanced the pass dispatch. New phases318-324 use the existing version146 snapshot layout; earlier snapshots remain supported. Across24 constructed cases, original/native represented session bytes11..323, saved contact, cursor and flag match seven new boundaries (168 comparisons). Focused tests cover progression, replay rejection, cursor validation and snapshot roundtrip, wrong-version and truncation. Private evidence: ../Captain-Majid-analysis/mixed25-seconddefender-native-31bb9679, ../Captain-Majid-analysis/mixed25-secondcontact-native-57084032 and ../Captain-Majid-analysis/mixed25-thirddispatch-native-d5255acc.
- Original-only continuation reaches $C612 and $DE64 in all24 cases; a representative observer reached its 2,500 cooperative-wait limit thereafter. No pass flight or reception parity is claimed. The scenario is constructed, defender choices are supplied, and cooperative acknowledgements are not live timing, rendering or audio. Authentic initialization and a complete playable match remain unverified; progress is unmeasured. Next compare numeric field advancement and the target boundary, then investigate the observer wait. Suggested model: Terra High. Evidence: ../Captain-Majid-analysis/mixed25-afterdispatch-original-6bff3082.
- An overbroad validation edit initially caused keeper-save-session-tests to fail; the unrelated condition was restored, then all45 Windows test executables passed. Full Windows CMake build also passed.

## 2026-11-09 - Baseline and remaining defender checkpoint inspection

- Windows CMake build completed with no work to do; all 45 existing Windows test executables passed. `ctest` is not on PATH, so these were run directly. Re-ran the private first-contact comparison: all 24 original/native comparison and clock boundaries matched.
- Inspected all 24 existing original-only continuations. Defender19 preparation/comparison occurs at cursor1, clock324, elapsed2; its contact reaches clock323, elapsed3. Defender20 preparation/comparison occurs at cursor2, clock323, elapsed3; its contact reaches clock322, elapsed4. Dispatch occurs at cursor3, clock321, elapsed5. These are original observations only; no native implementation or comparison for these boundaries has yet been completed.
- No engine changes were made. Next implement and compare the two remaining defenders and dispatch, preserving the explicit captured AI choices and snapshot compatibility. Full playable match, real initialization and live presentation remain unverified; overall progress is unmeasured. Suggested model: Terra High. Evidence: ../Captain-Majid-analysis/mixed25-player7pass-original-16857a86 and ../Captain-Majid-analysis/mixed25-firstcontact-native-d3f77cb2.

## 2026-11-08 - teammate pass and first defender

- At the 2-5 three-defender contact, the selected pass cursor follows 87,99,100,101,102,103 to player7. Across 24 constructed original/native cases, target entry, five movements and recipient confirmation all match (168 boundaries). Captured defender choices 1,1,1, exhausted numerical pass preparation, first defender17 preparation and comparison, and first contact clock/status/experience each match all24. The first comparison uses original row5 inputs 153,0,0 and thresholds 154,96,68,0; defender17 status becomes6, clock 325 to324, elapsed 1 to2.
- Private evidence: ../Captain-Majid-analysis/mixed25-teammate-original-d490290b, ../Captain-Majid-analysis/mixed25-teammate-native-3d85ca1d, ../Captain-Majid-analysis/mixed25-player7pass-original-16857a86, ../Captain-Majid-analysis/mixed25-player7prepare-native-90ec0f89, ../Captain-Majid-analysis/mixed25-firstdefender-native-0e9de415, ../Captain-Majid-analysis/mixed25-firstcontact-native-8b17a06d.
- Original-only tracing follows defender19 and defender20 and the subsequent dispatch; these have not yet been connected or compared natively. These are constructed traces with cooperative presentation waits, not live NMI/video/audio. AI choices are explicit captured inputs. Full Windows CMake build and all45 test executables passed. Complete playable match and real initialization remain unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-08 - two numeric opponent shots and 2-5 restart

- Player19's next shot from 2-3 now calculates its destination and trajectory rather than capturing the flight. All24 constructed original/native cases match choice, profile preparation, acknowledged presentation, clock, field movement and handoff. Target/setup match24; seven numeric cell steps and six callbacks match168 and144 cases respectively, crossing cells51,52,40,28,16,17,5. The original flag changes from1 to0 at flight setup, and the original defender cursor1 is retained. All24 goalkeeper prompts, selections, confirmations, overlay acknowledgements, comparisons and resulting 2-4 goals match; keeper stamina42 becomes22. Evidence: ../Captain-Majid-analysis/mixed23-nextshot-original-f26fb59b, ../Captain-Majid-analysis/mixed23-nextshot-native-d1bda2d9, ../Captain-Majid-analysis/mixed23-nextshot-field-c5f22848, ../Captain-Majid-analysis/mixed23-numericflight-b67e49c7, ../Captain-Majid-analysis/mixed23-keeperprompt-native-c2580c42, ../Captain-Majid-analysis/mixed23-keeperresult-native-766f9caf and ../Captain-Majid-analysis/mixed24-goal-native-d67a74ed.
- All24 2-4 goal presentations, restarts, formations, receiver/ball placements, kickoff handoffs and control-ready states match; formation clears the observed player17 status. Another1,584 polls,144 clock updates and144 field rounds match through a single-defender contact. Choosing an explicitly supplied dribble/defense action, all24 original/native attack and defender preparations, row9 comparison, terminal clock, numeric movement, possession transfer to player17 and opponent control match. Evidence: ../Captain-Majid-analysis/mixed24-restart-731e12e1, ../Captain-Majid-analysis/mixed24-kickoff-1b0ba9b9, ../Captain-Majid-analysis/mixed24-control-e62e9e7e, ../Captain-Majid-analysis/mixed24-p5path-f64e5e7d, ../Captain-Majid-analysis/mixed24-p5menu-309cca7a, ../Captain-Majid-analysis/mixed24-dribble-original-cd4dcaf3, ../Captain-Majid-analysis/mixed24-dribble-native-7ded728d and ../Captain-Majid-analysis/mixed24-terminal-native-e4dc9a4d.
- Player17 control matches1,056 polls,72 clocks and72 field rounds; the explicitly supplied dribble choice and one following clock/field round each match24. Another528 polls,24 clocks and24 field rounds reach the next AI shot. Reused the numeric shot path with player17's distinct fractions116/105, direction111, cleared flag and cursor0: all24 choices, preparations, movement and handoffs match, as do24 target/setups,168 steps and144 callbacks. The goalkeeper prompt and choice/confirmation/comparison match24; keeper stamina44 becomes24 and the 2-5 goal, presentation and restart match24. Evidence: ../Captain-Majid-analysis/mixed24-p17path-96027b90, ../Captain-Majid-analysis/mixed24-p17ai-d2e380bc, ../Captain-Majid-analysis/mixed24-p17next-c1fe7afc, ../Captain-Majid-analysis/mixed24-p17shot-native-2f92f6a8, ../Captain-Majid-analysis/mixed24-p17flight-988bc7bd, ../Captain-Majid-analysis/mixed24-p17keeperprompt-82b6d948, ../Captain-Majid-analysis/mixed24-p17keeperresult-native-451d17f5, ../Captain-Majid-analysis/mixed25-goal-native-b5b6ad36 and ../Captain-Majid-analysis/mixed25-restart-bcaf330c.
- All24 2-5 kickoff formation/receiver/ball/handoff and ready states match. A further2,640 polls,240 clocks and240 field rounds match through player5's next contact with defenders17,19,20 at clock325; the action menu opens identically in24 cases. Next trace user-selected action and all defender outcomes from ../Captain-Majid-analysis/mixed25-p5path-2c148e14 and ../Captain-Majid-analysis/mixed25-p5prompt-94071eee. Evidence also includes ../Captain-Majid-analysis/mixed25-kickoff-302d9bba and ../Captain-Majid-analysis/mixed25-control-3fae5d22.
- Corrected the previously published November 8 note: the earlier commit accidentally stored its new Arabic text and count as question marks. Separate website commit b116e69 fixes the UTF-8 note and updates current progress; remote main and hosted HTML were verified, not a browser render. Full Windows CMake build, all45 test executables and diff checks passed at this checkpoint. AI choices and presentation acknowledgements remain explicit inputs; cooperative traces are not live NMI/video/audio. A complete playable match and authentic initialization remain unverified; progress is unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-08 - 2-3 contact, exhausted pass and opponent dribble

- At the next home contact (clock355), player5 has zero stamina. All24 original/native ordinary-shot rejections returned to the same menu; 168 subsequent pass-menu polls and24 confirmations matched. The pass-target cursor matched24 initial states and168 first no-input/directional/confirm/cancel polls. Only the initial cell can presently be confirmed or cancelled on this route, and only one directional move is supported. Snapshot version146 saves the extended phase and target cursor; its move counter is native bookkeeping, not an attributed original variable. Tests cover roundtrip, truncation, wrong version and invalid cursor. Evidence: ../Captain-Majid-analysis/mixed23-shotreject-original-ea903444, ../Captain-Majid-analysis/mixed23-shotreject-native-0765c569, ../Captain-Majid-analysis/mixed23-passmenu-e566b09f, ../Captain-Majid-analysis/mixed23-passtarget-ef3bc500 and ../Captain-Majid-analysis/mixed23-target-native-06095183.
- All24 cases matched the zero-power pass preparation, defender17's profiled preparation and comparison, contact and dispatch clocks, and calculated field movement. Original same-cell timeout uses candidates1 and19 for two rounds; native numerical contest and stationary recovery match24 original owner19 states, including flag1, retained defender cursor1 and the possession-sound request. The timeout and presentation are acknowledged, not live-emulated. Evidence: ../Captain-Majid-analysis/mixed23-passprepared-cd9c94ee, ../Captain-Majid-analysis/mixed23-defender-a1797fed, ../Captain-Majid-analysis/mixed23-contact-fbbed1bc, ../Captain-Majid-analysis/mixed23-handoff-9bb8cfe7, ../Captain-Majid-analysis/mixed23-race-original-6452a120 and ../Captain-Majid-analysis/mixed23-recovery-native-7bbea0c3.
- All24 opponent recovery-to-control and ready states matched after acknowledging the observed facing change. Then 792 input polls,48 clock updates and48 field rounds matched through player19's cell75 AI decision at clock351. The observed choice action2 is supplied explicitly, not calculated by native AI;24 choice states and24 subsequent clock/field states matched. Evidence: ../Captain-Majid-analysis/mixed23-p19control-402da788, ../Captain-Majid-analysis/mixed23-p19path-a86f42c4, ../Captain-Majid-analysis/mixed23-p19choice-3965b3be and ../Captain-Majid-analysis/mixed23-p19resume-cac98b4a.
- After the action2 continuation, another528 polls,24 clock updates and24 field rounds matched through the next player19 AI decision at cell63/clock349. The retained defender cursor survives a saved-session roundtrip, and the observed clock guard rejects untraced earlier time. A first original-only follow-up hit 2,000 cooperative waits; diagnostics identified a goalkeeper input loop at $D2B6, not an unresolved scheduler failure. All24 original continuations reach the goalkeeper prompt after an action0 shot, with clock348, ball52,124/cell5 and flight flag0. This prompt and preceding shot are original-only; no native parity is claimed. Evidence: ../Captain-Majid-analysis/mixed23-p19next-84b2c17e, ../Captain-Majid-analysis/mixed23-nextkeeper-original-559c2c1d and ../Captain-Majid-analysis/trace_mixed23_p19next_keeper_case_20261108.py.
- Full Windows CMake build and all45 test executables passed. Original traces use cooperative host acknowledgements, not live NMI, rendering, audio or hardware timing. The earlier shot flight-to-keeper remains captured, the match is not completely playable, and overall progress is unmeasured. Next compare player19's cell63 action0 shot and goalkeeper prompt from the paired boundary snapshots; compare every new transition and eventually replace sampled choices/captures. Suggested model: Sol 6.1 High.

## 2026-11-08 - numeric recovery, extended phase IDs and 2-3 restart

- Replaced an earlier captured same-cell human-pass recovery with calculated behavior. All24 original races select player3 and player17 and time out in two rounds; native field snapshots match. Numeric field advancement and stationary-ball recovery then match24 original recovered-owner sessions. The original timeout presentation is still explicitly acknowledged. Evidence: ../Captain-Majid-analysis/mixed22-earlierrace-original-cd935c79, ../Captain-Majid-analysis/mixed22-earlierrace-native-60f56d59 and ../Captain-Majid-analysis/mixed22-earlierrecovery-native-79aad822.
- Extended `FieldSessionPhase` to16 bits and added version143 for two-byte phases,144 for extended pass targets and145 for extended goalkeeper choices. Existing versions preserve their byte layout. Added invalid-phase, wrong-version, truncation and roundtrip regressions. All24 player14 pass choice, profile preparation, presentation, dispatch, numeric field movement, target, flight setup, one-cell flight, reception, positioning and player19 control states matched the constructed original. Evidence: ../Captain-Majid-analysis/mixed22-p14ai-original-3e9733ae, ../Captain-Majid-analysis/mixed22-p14ai-native-33a8451b, ../Captain-Majid-analysis/mixed22-p14pass-original-a15040a1, ../Captain-Majid-analysis/mixed22-p14pass-native-28042fcb, ../Captain-Majid-analysis/mixed22-p14target-native-fda0148c, ../Captain-Majid-analysis/mixed22-p14flight-native-176a4c11, ../Captain-Majid-analysis/mixed22-p14cells-c6df10bf and ../Captain-Majid-analysis/mixed22-p19return-289d248c.
- The next player19 shot and prepared approach matched24; shot field movement is calculated, while the subsequent flight-to-keeper prompt is captured. All24 keeper selections, confirmations and comparisons matched (keeper44 to24), followed by a 2-3 goal and restart. The original celebration retains facing0 and changes the selected opposing player to5; earlier sampled goals used different values, so the new route preserves the observed values. Kickoff formation, receiver, ball, handoff and ready matched24;1,320 following polls,120 clocks and120 field rounds matched through player5 contact at clock355. Evidence: ../Captain-Majid-analysis/mixed22-p19return-path-7cff3653, ../Captain-Majid-analysis/mixed22-p19ai-native-2f64e155, ../Captain-Majid-analysis/mixed22-p19shot-original-904a580c, ../Captain-Majid-analysis/mixed22-p19shot-native-d54cdfbd, ../Captain-Majid-analysis/mixed22-p19keeper-native-dedb6d36, ../Captain-Majid-analysis/mixed22-p19keeper-original-f29c1756, ../Captain-Majid-analysis/mixed22-p19keeperchoice-native-09d0d93d, ../Captain-Majid-analysis/mixed22-p19goal-native-37e8f789, ../Captain-Majid-analysis/mixed23-restart-native-46bd6ea9, ../Captain-Majid-analysis/mixed23-kickoff-native-87373117, ../Captain-Majid-analysis/mixed23-control-8ca9be03 and ../Captain-Majid-analysis/mixed23-p5path-845d0650.
- Separate website checkout published Arabic note on public main3f9bb6b; remote ref verified, and a later hosted HTML check returned the new dated note. No browser render claimed. Full Windows CMake build, all45 tests and `git diff --check` passed. Next trace the 2-3 contact; authentic initialization, live audiovisual timing and a complete playable match remain unverified. Overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - five-defender dribble and player14 possession

- Player5's next five-defender menu matched168 original/native initial polls and24 action2 confirmations. In24 original cases, all five defenders chose action1/subtype0; captured choices and native profile-based dribble preparation matched at $804C/$804F. Stamina10 becomes0, raw cost40 and action power0. Added version142 snapshots carrying all five actions/subtypes and checks for truncation, corruption, wrong version, roundtrip and rejected unsupported inputs.
- Original/native prefix initialization and defender14 preparation/comparison each matched24: cursor resets1 to0; defense action1 has base18, raw cost60, stamina100; direct comparison row index9 yields rolled strength4, score0 and result3. The original's $80D9 classification separately confirms result3 in24 cases. Other defenders are not processed on this terminal path. The direct comparison values are supplied from private original content, not committed.
- After result presentation, the original awards defender-type selector2 experience +1, sets player5 status5 and advances elapsed1/clock367 to366; all24 native clock states matched. Calculated E074 field movement matched24 field/session snapshots. Player14 possession and opponent control-ready each matched24; the selected five stay recorded until control setup clears them. Side change exposes a music request. Evidence: ../Captain-Majid-analysis/mixed22-p5menu-2d840c66, ../Captain-Majid-analysis/mixed22-p5dribble-original-b9a48718, ../Captain-Majid-analysis/mixed22-p5dribble-native-f91a9828, ../Captain-Majid-analysis/mixed22-p5prefix-native-7167bb00, ../Captain-Majid-analysis/mixed22-p5result-original-70e170ef, ../Captain-Majid-analysis/mixed22-p5defender-native-80c57577, ../Captain-Majid-analysis/mixed22-p5terminal-original-3e5f7252, ../Captain-Majid-analysis/mixed22-p5terminal-native-0bc4845b, ../Captain-Majid-analysis/mixed22-p5field-native-0ee4c191 and ../Captain-Majid-analysis/mixed22-p14control-native-1216efd5.
- The following three field rounds matched1,056 polls,72 clock ticks and72 field rounds across24 cases, ending at player14 AI decision x100/y107/cell75, clock363/extra13 (../Captain-Majid-analysis/mixed22-p14path-e841ac4d). One original-only probe chooses pass action1/subtype0 and prepares subtype128/power21; next compare that decision and subsequent path. Full Windows CMake build, all45 tests and `git diff --check` passed. Earlier captured movement/recovery, cooperative presentation acknowledgements, unverified authentic initialization and incomplete playable match remain explicit; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - calculated pass contest and player5 control

- Traced the original same-cell pass contest to bank30:$DEDC in24 constructed cases. All selected player5/player19 and player5 won the first movement, with remaining counter2. Extracted a reusable field-level numeric race calculation from clearance handling and added a narrowly guarded session route; all24 field snapshots and represented sessions matched. The same-cell setup clears the selected-defender count, while the saved active prefix is normalized to zero.
- The original positions player5 and transfers ownership at $DF0D; 24 native states matched, retaining coordinate fractions. After85 cooperative acknowledgements in the representative original, all24 returned to $E0DF and all24 control-ready states matched. The original leaves $0616=1; native now retains it through these states, the following polls/clock/movement/contact and menu. Flight flag $061B also matches. Snapshot roundtrips and invalid outcome/atomicity were tested.
- All24 next no-input routes matched1,320 polls,120 clock ticks and120 field rounds, ending at clock367 with player5 against [14,16,18,19,20]; all24 action menus matched. Evidence: ../Captain-Majid-analysis/mixed22-p3race-original-60d81e57, ../Captain-Majid-analysis/mixed22-p3race-native-ef0493f6, ../Captain-Majid-analysis/mixed22-p3race-session-8a21ed63, ../Captain-Majid-analysis/mixed22-p3winner-original-b3cd0590, ../Captain-Majid-analysis/mixed22-p3winner-native-d4ca15d8, ../Captain-Majid-analysis/mixed22-p3control-native-7ac70f4d, ../Captain-Majid-analysis/mixed22-p5path-ca9506d3 and ../Captain-Majid-analysis/mixed22-p5prompt-f41bc7cc.
- Full Windows CMake build, all45 test executables and `git diff --check` passed. Next trace this menu's selected action and compare its defender sequence. The earlier player5-to-player3 recovery remains captured, as does player3's preceding handoff movement; acknowledgements are not live presentation. Authentic initialization, live timing/audio/video and a complete playable match remain unverified. Overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - player3 pass defender and same-cell target

- All24 original/native constructed cases matched defender17 action1 preparation (base18, raw cost60, stamina100), comparison (rolled strength0, score255, result0), contact status6/clock/experience and dispatch clock. At the latter boundary the score remains2-2, clock372/extra13 and elapsed3. Phase snapshots retain defender choice and comparison; phase255 roundtrips.
- Investigated a mismatch when applying the existing field selector: that selector models original caller $834C and the table at $8D95; this confrontation's $80C2 caller indexes a different table referenced at $8D93. Added an explicitly supplied direct comparison selection rather than using the wrong table or committing original table data. All24 comparisons matched after supplying the private original's row. The direct content is not yet exported as a general pack section.
- All24 captured field handoffs at $C612 and same-cell target states at $DE64 matched: recipient255, destination87, counter255, distance2. Previously reserved phase IDs111/112 now save these states with version141; the movement remains captured. Original-only traces reach loose-ball handling at $DE96; the contest outcome has not been compared. Evidence: ../Captain-Majid-analysis/mixed22-p3bounds-c583db28, ../Captain-Majid-analysis/mixed22-p3defnative-792fc905, ../Captain-Majid-analysis/mixed22-p3bounds-52f51614, ../Captain-Majid-analysis/mixed22-p3contact-b141a3c4, ../Captain-Majid-analysis/mixed22-p3dispatch-441e9a1e, ../Captain-Majid-analysis/mixed22-p3bounds-261b8829, ../Captain-Majid-analysis/mixed22-p3handoff-37a771a6 and ../Captain-Majid-analysis/mixed22-p3target-fc709601.
- Full Windows CMake build and all45 test executables passed; `git diff --check` passed. Next trace and calculate the same-cell loose-ball contest, preserving any required random/cursor state. Live timing/presentation, authentic match initialization and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - keeper flight flag and player3 weakened pass

- Fixed the reported 2-1 shot-to-keeper flag mismatch: the captured keeper prompt now clears $061B in the exact observed context. All24 original/native prompt, keeper-choice, comparison, goal and kickoff boundaries matched including the flag. Version139 now writes its required saved-flight flag even when zero, and a focused test verifies zero-flag snapshot roundtrip. Twenty-four kickoff control states,144 initial polls,24 menu openings,168 menu polls and24 shot confirmations also matched with the flag included. Target entry now requires the correct zero value rather than overwriting stale input.
- In player3's confrontation with defender17 at 2-2, all24 cases matched target entry, empty-cell confirmation and profile-calculated pass preparation. Player3 has stamina0 and the original accepts action1/subtype0, yielding weakened power0; this differs from the earlier out-of-contact ordinary-shot affordability rejection. Original defender17 action1 is implicit in the sampled prepared phase;24 original-only traces reached $80D6 with defender stamina100 and raw cost60. No native comparison or outcome is claimed for that defender yet.
- Evidence: ../Captain-Majid-analysis/mixed21-keeperprompt-7e1f1c00, ../Captain-Majid-analysis/mixed21-keeperchoice-4ead7238, ../Captain-Majid-analysis/mixed21-keeperresult-9a4bed81, ../Captain-Majid-analysis/mixed21-equalizer-59b15a1b, ../Captain-Majid-analysis/mixed22-kickoff-aa690675, ../Captain-Majid-analysis/mixed22-control-0f767a0c, ../Captain-Majid-analysis/mixed22-menu-8e7792d4, ../Captain-Majid-analysis/mixed22-targetnative-88bcadf8, ../Captain-Majid-analysis/mixed22-player3menu-5efd834e, ../Captain-Majid-analysis/mixed22-player3choice-9ab36b33, ../Captain-Majid-analysis/mixed22-player3pass-a613c8db and ../Captain-Majid-analysis/mixed22-player3defense-0eeea454. Full Windows CMake build and all45 test executables passed. Next compare defender preparation and outcome; replace the earlier captured loose-ball recovery with calculated behavior. Live timing/presentation, authentic match initialization and a full playable match remain unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - human empty pass recovery and player3 contact

- Reconstructed the menu caller from original $E38A/$8003, eliminating the synthetic return-stack failure. All24 original/native player5 empty-target pass preparations matched at $804F: stamina31 to11, power21/subtype128. All24 captured handoffs matched at $C612. The original same-cell pass enters loose-ball recovery and reaches player3 at $8C31 in all24 cases. An explicit captured-field native recovery matched all24 represented sessions and the original flight flag1; this does not calculate the intervening loose-ball race or movement.
- Player3 control handoff and control-ready matched24 each. No-input continuation matched1848 polls,168 clock ticks and168 field rounds, reaching player3 versus defender17 at cell87, clock374/extra13. The ensuing human action menu matched24. Evidence: ../Captain-Majid-analysis/mixed22-passprepare-00905695, ../Captain-Majid-analysis/mixed22-passprepared-native-99c168a4, ../Captain-Majid-analysis/mixed22-passc612-6e926825, ../Captain-Majid-analysis/mixed22-passhandoff-native-8fc42d31, ../Captain-Majid-analysis/mixed22-pass8c31-4b990def, ../Captain-Majid-analysis/mixed22-passrecovery-6d83d5e0, ../Captain-Majid-analysis/mixed22-player3control-0b28a98f, ../Captain-Majid-analysis/mixed22-player3path-80d46dc2 and ../Captain-Majid-analysis/mixed22-player3prompt-16d32e79.
- Found an omitted flag comparison: original $061B changed from1 at shot handoff to0 at the following keeper prompt in all24 cases, and remained0 at the goal, kickoff and home menu while native retained1. Earlier represented-state comparisons did not cover that appended flag. Human pass target entry now explicitly captures0 and recovered-owner capture restores original1; target, cursor, confirmation, preparation and handoff were rechecked with the flag included. Next fix the earlier shot-to-keeper transition and compare it, trace the confrontation and replace captured loose-ball recovery with calculated behavior. Full Windows CMake build and all45 test executables passed. Complete playable match, authentic match initialization and live timing/presentation remain unverified; overall progress unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - sampled human pass target cursor

- Traced the 2-2 player5 pass confirmation to the original target cursor at cell87 and ball mode129. Added a guarded native cursor phase and version141 session snapshot. All24 initial states and120 first no-input/directional polls matched the original; a further480 comparisons matched a second no-input/directional poll from the four adjacent cells. Two-move tracking is native evidence-bound bookkeeping, not a rule attributed to the original game.
- At the initial cell, original/native comparisons matched24 empty-target confirmations at $D84B (original recipient byte255) and24 cancellations at $D83E before return. Saved pending phases do not claim completion of the pass or cancellation. An attempted continuation after $D716 encountered an invalid synthetic caller stack, so further work must reconstruct the parent before interpreting later behavior. Snapshot roundtrip, truncation and wrong-version checks were added. Evidence: ../Captain-Majid-analysis/mixed22-passtarget-b7f78932, ../Captain-Majid-analysis/mixed22-targetnative-25443cc3, ../Captain-Majid-analysis/mixed22-second-6a51715d, ../Captain-Majid-analysis/mixed22-secondnative-33d00b6a and ../Captain-Majid-analysis/mixed22-passconfirm-1211bd81.
- Next reconstruct the original target menu caller and trace pass/cancel continuation. Cooperative acknowledgements do not verify live presentation/timing; real match initialization and a complete playable match remain unverified. Overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - 2-2 kickoff and unaffordable home shot

- All24 original/native constructed cases matched the 2-2 home kickoff formation, player5 receiver, ball placement, handoff and control-ready. Six initial input patterns matched144 comparisons. The home action menu matched24 openings,168 polls and24 ordinary-shot confirmations.
- Player5 had31 stamina and ordinary shot cost80. Following confirmation through the original menu path returned to the menu after102 cooperative acknowledgements in all24 cases, without spending stamina. The original reset attack subtype and cursor; added a narrowly guarded native acknowledgement with calculated affordability and unchanged error semantics. All24 original/native returned menu states matched. Tested wrong stamina and wrong content cost rejection and snapshot roundtrip.
- From the returned menu,168 input polls and24 pass confirmations matched original/native. Evidence: ../Captain-Majid-analysis/mixed22-kickoff-c2f80c2b, ../Captain-Majid-analysis/mixed22-control-2563a5db, ../Captain-Majid-analysis/mixed22-input-1eda4595, ../Captain-Majid-analysis/mixed22-menu-322e8409, ../Captain-Majid-analysis/mixed22-reject-a3472d24, ../Captain-Majid-analysis/mixed22-rejectnative-90890b40 and ../Captain-Majid-analysis/mixed22-passmenu-a63f426f. Next trace confirmed human pass targeting. Live timing/presentation and authentic match initialization remain unverified, as does a complete playable match; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - player19 shot and 2-2 equalizer

- Compared the sampled 2-1 player19 shot across24 constructed original/native cases: presentation, clock, captured movement and shot handoff matched, followed by keeper prompt, ordinary action0 selection, confirmation and precomparison. The original prompt had jitter control144 and keeper stamina30; calculated action0 spent20 stamina and left10, with score255/result0. Added narrowly guarded support and failing-first regressions; all24 comparison states matched.
- Original/native 2-2 goal, presented state and restart boundary each matched24; goal scoring includes the checked experience award. Original observer used459 cooperative acknowledgements at goal and788 at restart. This is captured presentation, not live timing or a general shot simulation. Evidence: ../Captain-Majid-analysis/mixed21-shothandoff-7a9d067c, ../Captain-Majid-analysis/mixed21-keeperprompt-ce9fdddc, ../Captain-Majid-analysis/mixed21-keeperchoice-44dc2383, ../Captain-Majid-analysis/mixed21-keeperresult-afe21f50 and ../Captain-Majid-analysis/mixed21-equalizer-277e1819.
- Next follow the 2-2 home kickoff and next field play. Authentic match initialization, live timing/audio/video and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - 2-1 return pass and player19 shot choice

- Captured movement, target and flight setup for 24 constructed player12 return passes matched original/native. The pass targets player19/cell79 with distance3, counter255 and fractions8/255. All168 flight steps,144 callbacks,24 receptions and24 positioned states matched. Added narrowly guarded handoff and flight variants with a focused failing-first regression; player19 arrival x99/y141/cell79, facing change64 to0, control handoff and control-ready states matched24 each.
- The next no-input play matched792 polls,48 clock ticks and48 field rounds, reaching player19 x92/y141/cell67 at clock382/extra13. Added guarded shot choice and saved-phase support after a failing test; all24 original/native action0 choices and profile-calculated power22 preparations matched. One original-only constructed trace reached the keeper prompt at x52/y124/cell5 and clock381/extra13. The new shot presentation and keeper continuation still require native comparisons.
- Private evidence: ../Captain-Majid-analysis/mixed21-return-6dc45f28, ../Captain-Majid-analysis/mixed21-returnflight-66f804d8, ../Captain-Majid-analysis/mixed21-returncontrol-7e13f078, ../Captain-Majid-analysis/mixed21-player19-path-a6923c2e and ../Captain-Majid-analysis/mixed21-shotcalc-8c5f1c0c. Next compare the shot presentation, flight and keeper boundaries. Live timing/presentation, authentic match initialization and a complete playable match remain unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - post-goal player12 decision and pass dispatch

- From 24 matched 2-1 player12 control-ready states, compared nine input masks per case (216 initial polls), then no-input continuation through 528 polls,24 clock ticks and24 field rounds. Every case reached player12 x133/y161/cell130, clock384/extra13, elapsed1, at the AI decision boundary, with represented original/native sessions equal.
- Original action1/subtype0 was retained, and attack preparation yielded subtype128/power21. The old 1-0 player12 guard rejected the 2-1 case. Added a focused failing test, then extended choice and saved-phase validation for the exact observed coordinates, score and clock. All24 original/native choice and preparation comparisons matched. Original/native presentation acknowledgement and dispatch also matched24 each, after nine cooperative acknowledgements per case.
- Evidence: ../Captain-Majid-analysis/mixed21-input-cc625d05, ../Captain-Majid-analysis/mixed21-path-57b238ec, ../Captain-Majid-analysis/mixed21-decision-56f712bb and ../Captain-Majid-analysis/mixed21-dispatch-95af4734. Next follow the player12 pass movement, handoff and target. These constructed observations do not establish live timing/presentation or authentic match initialization. A complete playable match remains unverified and overall progress is unmeasured. Full Windows CMake build and all45 tests passed. Suggested model: Sol 6.1 High.
## 2026-11-08 - result4 mixed pass to player12 control

- Traced 24 constructed original 2-1 player19 result4 passes to target player12/cell142 (distance3, counter255). Native target state matched all24. The original flight began with fractions232/255 at bank31:$E773; a narrowly guarded native variant matched all24 flight setups and trajectories.
- Continued the numeric flight across168 ball-cell steps and144 callbacks; all24 receptions and24 positioned states matched. Arrival was player12 at x137/y164/cell142, clock385/extra13, with zero selected defenders. Original/native control handoff and control-ready comparisons each matched all24 represented sessions. An initial comparison mismatch was caused by mismatched numeric versus lexical case ordering in the private comparison script; corrected pairing matched24/24.
- Added focused regression and roundtrip checks for the observed target, fractions and control handoff. Full Windows CMake build and all45 test executables passed. Evidence: ../Captain-Majid-analysis/mixed-pass-original-c10c23ca, ../Captain-Majid-analysis/mixed-target-native-4bb6aa3d, ../Captain-Majid-analysis/mixed-flight-native-5fdf91d7, ../Captain-Majid-analysis/mixed-flight-all-99a3a648, ../Captain-Majid-analysis/mixed21-control-b30f6f7b.
- Next trace player12 control and decision from this 2-1 route. Cooperative presentation acknowledgements are not live timing/audio/video, authentic match initialization remains unverified, and a complete playable match remains unverified. Overall progress is unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - post-goal player19 result4 and pass handoff

- Diagnosed the earlier bounded trace: the original branches from bank26:$80D6 via $8132 to $8E6A, bypassing the observer's requested $80DC/$80ED. In 24 constructed cases, result byte4 appeared at $8E6A after nine cooperative acknowledgements with score2-1 and defender3 stamina0. Extended the existing native result4 saved-state guard only for this observed context; all24 represented comparisons matched.
- Continued the original to fixed-bank $C612 in all24 cases after 550 cooperative acknowledgements. The original handoff kept side11/player19 and score2-1, with clock385/extra13, elapsed19, action1/subtype0/power24 and facing0. Added a guarded native variant to the existing result4 pass handoff; 24 original/native represented comparisons matched. A focused regression failed before the correction, then passed, including save/load roundtrip. Evidence: ../Captain-Majid-analysis/mixed-result4-51603d81, ../Captain-Majid-analysis/mixed-result4-native-eab309d9, ../Captain-Majid-analysis/mixed-pass-handoff-162c96e5 and ../Captain-Majid-analysis/mixed-pass-native-fbc34673.
- One representative original-only continuation reached player12 control at cell142. Next compare the pass target, flight and reception for all24 before native extension. Full Windows CMake build and all45 test executables passed. Original cooperative tasks do not verify live presentation/timing or authentic match initialization; complete playable match unverified and overall progress unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - player19 mixed contact through defender3 comparison

- Continued the prior 2-1 player19 contact from defender3's registered ordinary action0. In 24 constructed cases, defender8 and defender9 each chose direction4/action6; selection, confirmation and registration matched native at each boundary. Final [0,6,6,255] confirmation matched24. The original reached bank26:$804C after nine cooperative acknowledgements with attacker action2/subtype0/power21; native captured choice matched24.
- Original/native comparisons matched 24 calculated attack preparations at $804F (subtype128/power24), 24 mixed defender preparations at $80B6 (defender3 stamina30), and 24 comparisons at $80D6 (defender3 stamina0, attack power24). Extended saved-phase guards only for the observed 2-1 contact through phases205 and215..218. Focused regressions first failed at the unsupported phases, then passed after the guarded extension. Evidence: ../Captain-Majid-analysis/mixed-choices-246f25e9, ../Captain-Majid-analysis/mixed-ai-eb044cf6, ../Captain-Majid-analysis/mixed-native-8164db5c, ../Captain-Majid-analysis/mixed-prep-c2bba69c, ../Captain-Majid-analysis/mixed-defender-96579625 and ../Captain-Majid-analysis/mixed-comp-59ecf702.
- Bounded original tracing reached $804F/$805A/$8074 and $80B6/$80D6, but not $80DC or $80ED within 500,000 instructions. The following branch and outcome are unknown. Full Windows build and all45 test executables passed. Next investigate scheduler state after comparison, then trace the outcome before native continuation. Complete playable match and live timing/presentation remain unverified; overall progress unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - post-goal player19 defensive prompt and first registration

- Traced 24 constructed original cases from the matched player19 contact to a defensive prompt with blink13. Original/native represented state matched all24 prompts, 264 first polls across pressed masks0,1,2,4,8,64,128,129,130,132,136, and 24 direction1/action0 confirmations for defender3.
- Original registered defender3's ordinary action0 and advanced the cursor to1 in all24 cases. Native initially rejected this state because the earlier roster guard assumed stamina110 for every other home field player. The observed 2-1 roster is type23 for IDs1..10, stamina30 at defender3, 25 at player5 and 140 elsewhere. Added a narrowly guarded roster case for this contact; all24 original/native registration comparisons then matched represented bytes. A regression failed before the correction and passed after; adjacent altered stamina is rejected. Evidence: ../Captain-Majid-analysis/prompt-0e7ed0bc, ../Captain-Majid-analysis/poll-cdcdaf93, ../Captain-Majid-analysis/confirm-03e91fdf, ../Captain-Majid-analysis/orig-register-8f18d22d and ../Captain-Majid-analysis/verified-register-a1a12b9e.
- Full Windows CMake build and all45 test executables passed. Next follow remaining defender choices and the attacking response. Live timing/presentation, authentic match initialization and a complete playable match remain unverified; overall progress unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-08 - player19 action2 contact continuation

- From the prior 24 constructed player19 action2 states at 2-1, compared the original and native control tick and no-input field round: 24 matches at each boundary. All cases reached side11 player19 x111/y141/cell91, clock403/extra9 and elapsed1 against selected defenders3,8,9.
- Continued all 24 original contact states to bank26:$8003 with 96 cooperative acknowledgements each. The existing guarded native contact presentation acknowledgement matched represented session bytes11..323 in all24 cases. No engine code changed. Evidence: ../Captain-Majid-analysis/next-field-a56c2a25 and ../Captain-Majid-analysis/next-contact-fb65be45. Recreated the Python research environment privately; used short private staging paths to avoid the Windows C++ CLI path limit.
- Windows CMake build reported no work to do and all45 test executables passed. Next trace the contact prompt, defender input and attacker response. Real-match initialization, live presentation/timing and a complete match remain unverified; overall progress unmeasured. Suggested model: Sol 6.1 High.
## 2026-11-07 - post-goal pass to player19 and next decision

- Connected the guarded 2-1 player16 pass through presentation, dispatch, captured field movement, target and numeric flight. Each new boundary matched24 original/native cases. Target player19/cell103, fractions143/27; 96 flight steps, 72 callbacks, 24 receptions and 24 positioned states matched. Original and native control handoff and control-ready states also matched24 each. Clearing stale native defender-sequence bookkeeping was required to enter the new flight. Presentation movement remains explicitly captured.
- After player19 receives, 24 no-input cases matched792 polls, 48 clock ticks and 48 field rounds, reaching x111/y141/cell91 at clock404/extra9. Original selected action2/subtype0/power21 and resumed control-clock pending; 24 captured-choice native states matched. Guarded this exact scenario; it is not general AI. Original traces use reconstructed cooperative tasks and do not verify live timing or real-match team initialization.
- Full Windows CMake build and all45 test executables passed. Next follow player19 action2 through further field play and encounters. Complete playable match unverified, progress unmeasured. Local work preserved; separate cloud backup not integrated. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/postgoal-pass-player19-9ce083bf511a4c5d8d8ba97f67efbf17. Suggested model: Sol 6.1 High.
## 2026-11-07 - player16 post-goal pass decision

- In 24 constructed original/native cases after the 2-1 kickoff, matched 144 initial held-input polls, then under no input 792 polls, 48 clock transitions and 48 field rounds. Player16 reached an AI decision at x141/y148/cell140 and clock406/extra9. Each of 24 original/native states matched the captured action1/subtype0/power22 choice and subtype128/power21 preparation. Narrowly extended existing phases194/195, with rejection of adjacent unverified clock and snapshot roundtrip tests; other follow-on phases remain unchanged. This is a captured AI choice/preparation, not generalized decision logic.
- Original-only constructed traces then reached player19 x118/y141/cell103, elapsed0, clock406, controlled player7 in 24 cases. Native pass dispatch, movement, flight and possession for this post-goal path remain unconnected. Original tasks and presentation acknowledgements are reconstructed; no live NMI/video/audio or hardware timing is claimed.
- Full Windows CMake build and all 45 test executables passed. Next compare the pass dispatch and flight to player19, then continue field play. Live original team initialization and full playable match remain unverified; overall progress unmeasured. Local work preserved; separate experimental cloud backup not integrated. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/postgoal-opponent16-d175d562a0ae4762b5df9ece6e8e3b5c. Suggested model: Sol 6.1 High.
## 2026-11-07 - opponent kickoff control and wider sampled reset

- Compared 24 original/native constructed cases through receiver16 selection, ball placement and kickoff handoff. The known presentation scratch/facing differences persisted at those boundaries. At control-ready, all 24 compared represented states matched bytes11..323 excluding phase; opponent16 owned the ball in cell152 and player6 was controlled. This is cooperative reconstructed CPU/task evidence, not live presentation or hardware timing.
- Original $CF4F isolated reset cleared all22 player statuses and both keeper Y low bytes. In 24 constructed restart cases, injected varied field statuses and keeper0 stun at $C636 were cleared before formation. Extended the existing narrowly guarded 2-1/clock408/extra9 native reset to clear field statuses and goalkeeper stun for that scenario; these comparisons matched represented fields apart from the known scratch/facing differences. Added a focused synthetic regression and retained the out-of-scope-score guard. The injection is experimental, not an organic match.
- Full Windows CMake build and all 45 test executables passed. Search of the existing private analysis directory found no emulator save-state files; actual live-match team initialization remains unverified. Next follow opponent16 to the next input/decision, investigate general reset contexts and obtain a valid original live match/team capture. Full playable match unverified; progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/home-opponent-control-20c31589d00941ab8368b7a84285100a. Suggested model: Sol 6.1 High.
## 2026-11-07 - goal restart status reset and presentation writes

- Traced 24 constructed original cases from home goal through an explicitly confirmed score menu to opponent restart and kickoff formation. Original reset clears defender17 status6 at bank30:$CF59 before formation; native retained it. Added an evidence-bounded fix for the sampled 2-1/clock408/extra9 case and tests for the matching and out-of-scope score. Original/native formation comparisons then matched represented bytes except original scratch opponent15 versus native11 and original facing0 versus native64; selected keeper11 was separately checked due to exporter restrictions.
- Corrected preceding log: byte304 is control motion request, not scheduling flags. Original presentation writes opponent scratch $442 at bank16:$8358, facing $517 at bank31:$ED0F, and motion $532 through presentation. The $BAF6 keeper team pointer with stage0/team3 remains unchanged throughout the constructed trace; prior independent initialization established that mapping, but no fresh live-match loader was observed. Presentation timing, real match selection and general reset behavior remain unverified.
- Full Windows CMake build and all 45 test executables passed. Next investigate the remaining presentation state, other reset cases and real-match team capture. Complete playable match unverified; overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/home-goal-restart-6ccac4a928fd452c960fcc37fe549bfb. Suggested model: Sol 6.1 High.
## 2026-11-07 - sampled opposing goalkeeper and goal integration

- Compared 24 constructed original/native cases at opposing goalkeeper action1/subtype0: preparation base12, rolled strength6, ratio255, score255, result0, and unchanged side0 keeper stamina. Existing native continuation produced the same 2-1 score and +15 scorer experience in all 24 cases. Original action is captured; team3 is assumed, and no general AI or authentic live-match team selection is claimed.
- Comparison found original versus native presentation differences: recovery scratch opponent15 versus11 and motion request3 versus0 at keeper result; goal facing0 versus64 and motion request19 versus0. Selected keeper11 was checked separately because the exporter does not support goalkeeper IDs. Do not claim full represented-state parity or live timing. Added a synthetic-content integration regression from constructed arrival through the existing keeper and goal APIs; no production behavior changed.
- Focused test, full Windows CMake build, and all 45 test executables passed. Next trace the presentation writes and compare the post-goal restart; verify actual goalkeeper team initialization. Full playable match unverified and progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/home-flight-comparison-6dba4710d74c4d918ff45f29c9e7f8c5. Suggested model: Sol 6.1 High.
## 2026-11-07 - home numeric shot flight and original goal evidence

- Controlled original/native comparisons matched 24 shot targets and 24 numeric flight entries, followed by 336 cell steps and 312 defender callbacks. All 24 paths crossed cells 99,111,123,135,136,148,160,172,184,196,208,220,232,233 with no selected defenders. Compared represented bytes exclude the phase; original cursor1 versus native generic cursor0 is not compared. The result does not establish complete RAM parity or live frame timing.
- In 24 original-only constructed traces with menu input released after the shot choice, the original reached goal presentation $85BC with result0 and score2-1. The later repeated observer wait includes an input loop at $98A7..$98C1 after the score; held input produces a different later execution. All 24 original-only traces also reached $8591 with comparison accumulator255, outcome Y=0, random bytes0/0, keeper index11, power22, distance108 and player-record pointer $0384. The native calculation and live generic keeper team context remain unverified; neither a native keeper continuation nor general goalkeeper behavior is claimed.
- No engine code changed. Next trace keeper calculation and branch, then compare a narrowly supported continuation. Full playable match unverified; overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107/home-flight-comparison-6dba4710d74c4d918ff45f29c9e7f8c5. Suggested model: Sol 6.1 High.
## 2026-11-07 - home shot flight setup and bounded keeper diagnostic

- Added guarded phase243 shot target and connected it to the existing numeric ball-flight setup, using the observed 116/92 fractional position. One constructed original trace gave distance108, destination233, counter255 at $DD08 and selected count0/direction6 at $E773. Focused tests cover calculated target, guards, flight setup and save roundtrips. Direct native/original comparison and intermediate flight cells remain outstanding; the internal defender cursor is cleared for generic flight although original RAM remains1 at $E773.
- A diagnostic releases the attack-menu input after confirmation. It reached opposing keeper entry at instruction323762, then exhausted two million instructions after 6,173 cooperative host acknowledgements without reaching the keeper comparison. The cause and actual keeper team context remain unknown. This is a constructed CPU/task observation, not live hardware timing. No keeper outcome is claimed. Scripts retained in the existing private stage0-home-shot-handoff-20261107 evidence directory.
- Full Windows CMake build and all 45 test executables passed. Next compare target and flight entry and subsequent cells against original cases, then investigate scheduler state or live match capture. Full playable match unverified; overall progress unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-07 - home shot clock, experience and handoff

- Traced the original result0 continuation: defender17 action1 sets status6, the contact clock advances one tick and attacker type23 receives selector1 experience (+3 in sampled slot0). Native phase240 uses existing contact-prelude and experience rules. Phase241 advances the original two-tick shot dispatch with keeper recovery; phase242 captures the observed presentation motion and handoff. Each boundary matched 24 original/native represented states and saved defender cursor/result. The captured movement is not a live animation simulation.
- Original-only traces of 24 cases reached $DD1C at x204/y124/cell233, no selected field defenders, after 431 cooperative waits. One reached opposing keeper entry $8006 with generic type0, stage0/team3 and retained pointer $BAF6 in this constructed run. Neither a fresh keeper loader pointer nor a live match was verified. Attempts to reach $8B49 or the keeper comparison hit a 2,000,000-instruction observer bound at $CC65; cause unknown. Native shot flight and keeper outcome remain unconnected.
- Focused tests verify content, status, experience, clock, captured-field protection and snapshot roundtrips. Full Windows CMake build and all 45 test executables passed. Original observations use constructed state and cooperative acknowledgements, not live NMI/video/audio or hardware timing. Next connect phase242 to numeric shot flight, then pursue a valid generic-keeper context. Complete playable match unverified; overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-shot-handoff-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - home shot versus opponent17

- Traced the original continuously from the home contact menu through the chosen ordinary shot. All 24 constructed cases selected defender17 action1/subtype0, then prepared the attack at power22. Defender profile was base18/cost60 and comparison score255/result0; the shot retained power22 and AI defender stamina remained100. The original later reached $80ED with clock408 and fixed-bank $C618 with the shot continuing; neither later boundary is native-compared yet. This is a captured AI choice, not generalized decision logic.
- Added guarded phase238 for the observed defender choice and phase239 for profile-calculated defender preparation and comparison, acknowledging the intervening ball-variant/motion presentation. Both boundaries matched 24 original/native represented states, saved defender arrays and the original result. Focused tests cover invalid choice/content, input preservation and snapshot roundtrips. Full Windows CMake build and all 45 test executables passed.
- The original trace uses reconstructed host tasks, fixed selected input and cooperative acknowledgements, not live NMI/video/audio or hardware timing. Next connect the post-comparison clock, experience and captured movement to the observed shot handoff; investigate subsequent shot/keeper outcomes and actual generic opposing team context. No full playable match verified; progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-home-defender17-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - alternate goalkeeper action and home confrontation

- Added guarded home-goalkeeper action1 for the sampled extra9/stamina16 shot. The original and native states matched across 24 cases each for direction8 selection, confirmation, overlay, goalkeeper comparison and goal. The original selected base12/cost40, stamina16 to0, weakened, score255/result0 and a 1-1 goal. Only this low-stamina scenario is implemented for the alternate action; action0's prior guarded paths remain intact. Focused tests verify content, roundtrip and rejection of unverified stamina.
- From the existing action0 goal restart, no-input home play matched 1,848 polls, 168 clock ticks and 168 field rounds across 24 cases, reaching player5 x108/y108/cell87 against opponent17 at clock411/extra9. Opening the human menu matched 24 states, and seven initial input masks matched 168 states. Direction1/confirm and an isolated synthetic ordinary-shot preparation each matched 24 states (prepared power22). The synthetic preparation bypasses the actual defender decision, so no confrontation outcome is claimed.
- Full Windows CMake build and all 45 test executables passed. Original observations use constructed states and cooperative host acknowledgements, not live NMI/audio/video or hardware timing. Next trace opponent17's actual defender choice and interaction after the human selection, investigate additional keeper stamina/choices, and pursue authentic opposing generic team context. Full playable match unverified; overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-alternate-keeper-home-contact-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - later player19 shot returns to home kickoff

- Connected the later sampled shot at x93/y140/cell67 and clock419/extra9 through existing guarded shot and home-keeper stages. Twenty-four original/native represented-state comparisons matched each of presentation, clock, captured approach, handoff, captured keeper prompt, explicit direction1 selection, confirmation, overlay acknowledgement and goalkeeper comparison. Home keeper stamina16 was insufficient for cost20; it reached0, weakened, and score255/result0. Only the selected action0 is resolved; shot flight and presentation movement are captured/acknowledged.
- For this selected keeper outcome, 24 original/native comparisons each matched the 1-1 goal, side switch and restart. Extended the existing drawn kickoff guard to the observed clock418/extra9 sample. Formation, receiver5, ball placement, handoff and control-ready each matched 24 original/native cases; six initial input pairs matched 144 polls. Full Windows CMake build and all 45 test executables passed. Focused tests cover state preservation and roundtrips.
- Original traces are constructed and acknowledge cooperative tasks, not live NMI/video/audio or hardware timing. Next follow home player5 through further play and investigate additional keeper choices/stamina and authentic opposing generic-team context. No complete playable match has been verified; overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-player19-shot-goal-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - calculated pass and player19 next shot

- Traced the previous pass preparation to bank28:$819D via $C54B: action1, ball variant/situation0, original profile base21 and cost20 in all 24 constructed cases, followed by subtype128/power21. The earlier cost60/power18 probe accidentally supplied the defense table; the attack table gives the matching values. The native sampled player12 preparation now uses `prepare_attacking_action` and the attack data; 24 original/native snapshots matched. This is a guarded sample, not general AI selection.
- From player19 control-ready, 216 input polls, 792 no-input polls, 48 clock ticks and 48 field rounds matched across 24 constructed cases. At ball93/140/cell67 and clock419, the original chose shot0 and prepared subtype128/power22/cost80. Added captured choice phase236 and calculated attack preparation phase237; 24 original/native snapshots matched each. Original-only traces of all 24 shots reached the home keeper prompt (keeper0, ball52/124/cell5) after 185 cooperative waits; native shot arrival and outcome are not yet implemented for this route.
- Focused tests cover input/content rejection and saved-state roundtrips. Full Windows CMake build and all 45 test executables passed. Original traces are constructed and use cooperative acknowledgements, not live audiovisual/NMI timing. Next connect the new shot to its goalkeeper prompt and compare choice/outcome; continue investigating authentic generic opposing keeper team context. Complete playable match unverified and overall progress unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-player19-next-shot-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - player12 passes back to player19

- Compared 216 input polls for nine held/pressed pairs across 24 constructed cases. The no-input path matched 528 polls, 24 clock ticks and 24 field updates to owner12/ball134,160/cell130/clock421. Original action1/subtype0 and prepared subtype128/power21 were then saved as captured phases230..231; acknowledged presentation, dispatch, captured movement/handoff and target phases232..235 each matched 24 original/native states. The target is player19/cell79/counter255/distance3. Captured choice and motion are not general AI or presentation emulation.
- Numeric flight with captured fractions120/132 matched 24 setup states, 168 steps and 144 callbacks through seven cells, plus 24 receptions, 24 placements, 24 player-control entries and 24 control-ready states. Player19 receives at x100/y140/cell79. Focused tests cover guards, saved-state roundtrips and captured-field protection; full Windows CMake build and all 45 test executables passed. Constructed original CPU traces acknowledge host tasks and do not emulate live NMI/video/audio or hardware timing.
- An exploratory profile preparation with unverified situation0 returned cost60/power18, whereas the original observed cost20/power21. Do not replace the captured value without tracing the original preparation inputs and table selection; this comparison alone does not establish a core bug. Next trace player19 control and original preparation inputs and pursue authentic opposing keeper team data. A complete playable match is unverified, and overall progress is unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-player12-next-pass-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - mixed result4 pass returns player12 to control

- Added guarded sampled target phase229 and saved-state marker140 to retain receiver12, destination142, counter255/distance3 alongside saved flight flag1. Earlier marker138 remains compatible with its original phase. Connected the observed starting fractions216/228 to existing numeric flight, clearing the prior selected defenders and elapsed, and added the sampled side11 player12 control continuation with facing64 retained.
- Original/native represented and relevant trajectory/context comparisons matched 24 target and 24 setup states, 120 field-cell steps, 96 callbacks, 24 receptions, 24 ball placements, 24 player-control entries and 24 control-ready setups. All 24 traveled cells104,116,117,129,142 without selected defenders and reached player12 at x138/y164. Focused tests include snapshot roundtrip and rejection of wrong-version, truncated and damaged snapshots. Full Windows build and all 45 test executables passed; original traces use constructed state, held input and cooperative acknowledgements, not live video/audio/NMI or hardware timing.
- Next compare input/clock and successive control/field cycles to the next decision or encounter; independently trace actual opposing generic keeper team context. No full playable match is verified and overall progress is unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-mixed-result4-pass-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - native mixed-defender result and pass handoff

- Added guarded native phase227 for the observed zero-random mixed defender result4 and phase228 for the acknowledged pass-presentation handoff. The latter records sampled action1/subtype0, facing64, elapsed19, clock422 and extra time9. It does not calculate or simulate general presentation or frame timing.
- Twenty-four controlled original/native represented snapshots matched each new boundary, including result4; regression tests cover invalid/replayed input, unchanged input and saved-state roundtrips. Full Windows CMake build and all 45 test executables passed. The private original observer uses constructed states, held confirm and cooperative acknowledgements, not live NMI/video/audio.
- Next capture original recipient12/destination142/counter255/distance3, connect numeric flight and compare receiver12 possession. Original flight setup clears selected defenders and elapsed; the observed original return reaches x138/y164 at cell142. The opponent generic goalkeeper's actual team context remains unresolved. A complete playable match is unverified; overall progress remains unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-mixed-defender-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - mixed-defender result4 pass traced

- Corrected the earlier $80DC scheduler interpretation: 24 original constructed cases transition from initial result0 to result4 at $8154 and dispatch through $81EA/$9366 to $C612 and pass-flight setup $E773. The old return boundary is bypassed after the original resets the stack. At $8154, $0612 is the only changed RAM byte in $0300..$06FF versus the initial comparison. At flight entry, all 24 cases had side11, receiver12, action1, prior carrier19 and 550 cooperative waits. All 24 reached original control handoff after 657 waits, with side11 player12 owning the ball at x138/y164, cell142 and no selected defenders.
- On one saved case, 11 explicitly supplied $E2 random values gave result4 at 0..63 sampled values and result0 at sampled values64 and above. Do not generalize that range to other situations. Confirm128 was held through host acknowledgements; NMI/audio/video and real frame timing were not simulated. This is original-only, not native continuation parity.
- The saved reproduction passed 24 classification/flight-entry cases and 11 random probes. No engine code changed; the full Windows build and all 45 tests passed at the preceding implementation checkpoint and were not rerun for this investigation. Next implement and compare the result4 transition and its pass handoff using evidence-backed or explicitly captured effects; opponent generic-team context is still unverified. Overall progress unmeasured; no complete playable match verified. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-mixed-defender-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - equalizer, kickoff and return shot

- Implemented sampled home-keeper confirmation, acknowledged overlay and native goalkeeper comparison. Confirmation, precomparison and comparison each matched 24 original states; sampled keeper stamina18 is insufficient for cost20, leaving0 and yielding score255/result0.
- Implemented the observed opponent goal and score/experience update, acknowledged side switch, and restart entry. Each matched 24 controlled original cases. Reused kickoff formation/receiver/ball/handoff/control with narrowly guarded 1-1 restart support; five boundaries each matched 24 cases. Scripted presentation and earlier opponent shot flight are captured/acknowledged, not simulated.
- After kickoff, compared 144 home inputs, 24 action-menu openings, 168 menu polls, and 24 each shot confirmation, preparation and clock, plus 72 dispatch/destination states. Found stale saved-flight flag at DD08 and fixed the sampled 1-1 restart-shot path with a focused regression. The subsequent ball flight and opposing keeper entry matched 720 field/clock/experience/trajectory checkpoints across 24 cases.
- The opposing keeper is generic type0; actual original selected-team pointer/profile is unverified, so the controlled team3 pack cannot establish outcome parity. Obtain authentic loader/initialization evidence before continuing its save. Other requested priorities and full match remain incomplete; no live NMI/audio/video parity. Overall progress unmeasured. Full Windows build and all 45 test executables passed. Evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-equalizer-loop-20261107. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled home goalkeeper prompt and input

- Corrected the earlier opponent-shot scheduler diagnosis: the constructed observer was waiting at a human goalkeeper menu. It reaches bank30:$D2B4 in all 24 cases after 185 cooperative waits; supplying a direction and then confirm lets the original continue. The mixed-defender continuation remains a separate unknown.
- Added a guarded captured shot-arrival/keeper prompt and a separate keeper-menu action in version139 snapshots. Twenty-four prompt and 168 initial input represented-state comparisons matched the original. Initial confirmation without a selection waits; direction1/2 selects action0, direction8 action1, and direction4 is unavailable for sampled keeper type1. Sample sprite requests were also checked. Icons remain explicit caller data; combinations and confirmation after selecting are not implemented.
- A constructed original-only direction1/confirm policy returned to side0/player5 control in 24 cases after 1044 cooperative waits; this is not a native comparison or a claim of live audiovisual parity. Full Windows build and all 45 test executables passed. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-goalkeeper-input-20261107. Next trace and implement confirmation and goalkeeper resolution; continue other requested priorities. Complete playable match unverified, progress unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-07 - mixed field defender comparison

- Implemented the observed first-defender action0/no-special case for its exact home roster and the following two action6 choices. Controlled original/native comparisons passed for 24 action0 registrations and 168 later mixed menu states, including presentation requests.
- Added guarded mixed opponent choice/preparation and active defender preparation/comparison (phases215..218). Each of four boundaries matched 24 represented-state snapshots. Native code calculates the opponent and defender strengths, then applies the observed insufficient-stamina comparison: defender3 stamina30 to0, cost70, score255/result0, attacker power24. The opponent choice remains captured.
- The original observer reaches bank26:$80D9 but not $80DC within its 500,000-instruction bound. Later presentation and possession are unknown. Investigate scheduler/task state; do not infer original live behavior or assume this is the same cause as the shot stall. Full Windows build and all 45 test executables passed. Evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-mixed-defender-20261107. Progress remains unmeasured; no complete playable match verified. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled opponent shot handoff

- Extended the sampled match from a post-decision field update through another opponent shot: 528 no-input poll snapshots and the associated clock/field update matched controlled original execution across 24 cases.
- Added guarded captured choice, native profile-based shot preparation, acknowledged presentation, clock, captured movement and launch handoff (phases209..214). Each of the six boundaries matched 24 original/native represented-state snapshots. The shot choice and presentation motion remain supplied captures; the preparation uses the existing native calculation.
- The constructed original observer reaches fixed-bank $C618 after 120 cooperative waits, but fails to reach $DD08 within 2,000,000 instructions; by wait200 it loops in scheduler/yield code with $0515/$0516 zero. The root cause and live behavior are unknown; do not infer shot flight or result. Diagnose scheduler/task state or obtain a valid original capture before continuing that branch.
- Full Windows CMake build and all 45 test executables passed. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-opponent-shot-20261107. Complete playable match remains unverified; progress unmeasured. Continue AI, movement/presentation, encounters, defender choices, outcomes, attack and goalkeeper routes, restarts, live match loop and full-match testing as evidence supports. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled field inputs and opponent power

- Compared 216 directional/defender-switch polls and 216 subsequent clock/field updates against controlled original execution; all represented snapshots matched and the sampled updates returned to control. Further 264 no-input poll snapshots reached the next opponent decision; an explicitly captured action2 continuation and the following field update matched 24 cases each. No new encounter occurred in these tested updates.
- Implemented guarded native computation of the earlier sampled contact action2 power using supplied player profile/move data and the existing power formula. Its 24 resulting snapshots matched the original. A constructed survey of 60 random-byte pairs shows action and power can change; the decision itself remains explicitly captured and other values are not claimed supported.
- Original presentation movement remains captured; scheduler acknowledgements are constructed and no live NMI/audio/video or complete playable match has been verified. Full Windows CMake build and all 45 test executables passed. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-field-input-ai-20261107. Next trace the new AI decision through later field encounters and broaden native AI/strength inputs with original evidence. Overall progress is unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled contact returns to field control

- Added a guarded captured presentation checkpoint including observed movement, elapsed/region changes and clock440→439; connected it to the existing opponent control setup and cleared the completed defender choices. Focused tests failed before each implementation and passed afterward, including serialization and changed-owner rejection.
- Controlled original/native represented-state comparisons passed: 24 presentation captures, 24 control setups, 264 no-input polls, 24 clock/field-round entries and 24 no-input field updates returning to control. Capture supplies the player movement; it is not live presentation simulation.
- Original scheduler/presentation acknowledgements are constructed and no live NMI/audio/video or complete playable match is verified. Overall progress remains unmeasured. The prior AI choice and strength remain captured; expand original evidence before implementing them.
- Full Windows CMake build and all 45 test executables passed. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-field-contact-control-20261107. Next extend subsequent field-play inputs/contacts and replace captured presentation/AI with verified live behavior. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled contact opponent dispatch

- Added narrowly guarded checkpoints for explicitly captured action2/subtype0 and prepared subtype128/power24 after the sampled all-action6 confrontation. Each matched 24 controlled original/native represented-state snapshots. Focused tests failed before implementation, then passed, including serialization and rejection checks. No native AI selection or full strength formula is claimed.
- Traced original selection into bank28:$875D and identified reads of player/field/random inputs. At preparation, original scratch $043F changes20 to40 and is not retained in the session format.
- In 24 original-only cases action2 dispatched through bank26:$8118 and $C60F to $E0DF and $E145, returning toward opponent field control. Several player positions changed during constructed presentation waits; the native handoff remains unfinished. This is not a live presentation or complete-match test.
- Full Windows CMake build and all 45 test executables passed. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-field-contact-ai-dispatch-20261107. Next establish movement/presentation and scratch requirements, then compare a supported control handoff; special choices also remain pending. Complete playable match unverified, progress unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-07 - three sampled field-defense choices

- Implemented narrowly guarded action6 registration for defenders3/7/8 with exact confirm128 and all-action6 prefix, and fixed the field-menu title on back navigation. Other field registration and special choices remain unsupported.
- Controlled original/native comparisons passed for 24 first registrations, 264 second-defender polls, 24 second registrations, and 96 third-defender/final confirmation boundaries; recorded presentation requests also matched. Focused regression tests were observed failing before implementation and passing afterward.
- Constructed original-only option surveys show options vary with player type and action. Original-only all6 continuation reached bank26:$80EA and $80ED in 24 cases, with action2 selected and the three action6 defenders skipped. Native continuation and attack outcome are not yet implemented; the scheduler and presentation waits are constructed.
- Full Windows CMake build and all 45 test executables passed. Evidence is private at ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-field-defense-registration-20261107.
- Next: trace attack action selection/dispatch inputs and the ensuing action2 outcome, then implement only verified behavior; separately establish and support action0/1/2 special options. Complete playable match remains unverified; progress unmeasured. Suggested model: Sol 6.1 High.

## 2026-11-07 - sampled field confrontation input

- Implemented a guarded acknowledgement of the sampled field-defense prompt and field-route directional/confirm polling; retained saved ball-flight/keeper context and snapshot validation.
- Controlled original/native comparisons passed for 24 prompt snapshots, 264 first-poll input snapshots and 24 action0 confirmation snapshots. These are represented-state comparisons, not live audiovisual/presentation validation.
- Original-only observations: action0 had no special option in all 24 sampled defender3 states, and registration advanced the cursor to defender slot1. Native registration for this field route and later outcomes remain unimplemented.
- Full Windows CMake build passed; all 45 test executables passed. Private evidence lives in ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-field-defense-20261107.
- Next: establish option eligibility and registration constraints, implement and compare the supported continuation, then trace subsequent defender selections and outcomes. Full playable match remains unverified; overall progress is unmeasured. Suggested model: Sol 6.1 High for tracing and C++ integration.

This is the running reference for work between chat updates. Entries distinguish implemented code, observed original behavior, and remaining gaps. Source commits stay local unless publication is requested; ROMs and derived content remain private.

## Current position

- **Full port: currently unmeasured.** The repeated7% guess has been retired following the user's correction. See [completion checkpoints](PORT_STATUS.md); historical entries below retain what was reported at the time. There is no playable match yet.
- **Available:** native shot calculations, private content loading, resettable shot lab, persistent field/clock phase-session labs, and an optional SDL2 scene with placeholder graphics.
- **Current milestone:** persistent shot approach, fractional flight, empty arrival, keeper decisions, rebound/catch/punch handoffs and score/scorer reward registration are connected at prepared boundaries. A full playable match, goal presentation/celebration/restarts, queued arrival encounters, keeper control and live presentation/frame scheduling remain unfinished.
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


## 2026-10-28 — reusable native build and next timing observation

- Replaced Make's repeated per-program compilation of all core sources with52
  reusable objects and build/libcaptain-core.a. Kept CORE override support and
  explicitly retained all as the default target. No gameplay behavior changed.
- Default make -j3 all and make -j3 check passed with clean diagnostics. A later
  make -n all reports nothing to do. Logs: /tmp/cm-native-archive-build.log and
  /tmp/cm-native-archive-suite.log. No temporary archive override was needed.
- Next-step original-only reconnaissance starts at8410 after prepared script8,
  reachesC618 with clock449→448, defender12 status0→6, elapsed0→1. Three disclosed
  host frame acknowledgements occurred. Captures/report are private at
  ../Captain-Majid-analysis/attack-prelude-recon-07goh9ab. This is not yet connected
  to native phase43 and not proof of complete script/frame equivalence.
- Arabic gameplay checkpoint note was pushed as public website commit1135e46;
  commit-pinned published source matches local index.html. Browser rendering was
  not checked. Engine work remains local/private.
- Build review found no material regression. Addressed its archive-maintenance
  concern by recreating the generated archive: inserted a temporary obsolete
  object, rebuilt it, and verified exactly52current members remain. Production
  build and full suite were rerun after that adjustment; see
  /tmp/cm-archive-final-build.log and /tmp/cm-archive-final-suite.log.


## 2026-10-29 — prepared attacking clock and unopposed continuation

- Continued phase43 after explicitly acknowledged script8 through8410/9095 and
  inactive847F toC618 shot approach. Uses original defender status/clock arithmetic;
  no extra reward, energy debit, movement or owner change. Active selected keepers
  and nonordinary menus remain pending. Added save phases44/45, session18I339bytes,
  readers1..17. Added-time resume changes phase only; period-end remains terminal.
- Extended unopposed shots using explicit captured043D action and existing0442
  defender context; missing/extra action rejects instead of guessing. Both selected
  and unopposed regressions failed before implementation, then passed. Full Make
  suite/build passed, clean compiler diagnostics; CMake unavailable.
- Seven repeatable original/native cases match324prior represented bytes, with
  native metadata checked separately. Prepared frame/display acknowledgements are
  disclosed; no claim of script8 or live frame/render equivalence. Save/resume and
  replay checks pass. Evidence: attack-clock-repeatable in private analysis.
- Commands, boundaries and remaining limits in NATIVE_ATTACK_CLOCK.md. Next:
  originalDCFD shot distance/destination, then flight/keeper/goal integration.
  Overall port remains unmeasured; no complete playable match or online match.


## 2026-10-29 — saved shot distance/destination

- Continued phase45 through originalDCFD..DD08 using the existing approach math.
  Saved phase46 counter255, distance, destination and loose/keeper route; only the
  original random-byte shift changes prior represented state. Ball coordinates
  and current cell remain distinct from the destination. Session19J344bytes,
  readers1..18, strict malformed/unused/truncated contexts and replay rejection.
- Real regression failed before implementation; full default Make suite/build
  then passed clean. Review found no material issue; added its malformed-tail and
  whole-state preservation regressions, also passing. CMake unavailable.
- Six original/native observations match324prior represented bytes and independent
  counter/distance/destination/carry, without skipped calls or frames. Repeatable
  command in NATIVE_SESSION_APPROACH.md; evidence shot-approach-repeatable.
- E73E reconnaissance shows fractional ball coordinates and smooth signed velocity
  are needed for flight. Preserve these explicitly; no zero-fraction assumption or
  player-boundary clamp. Next: numeric ball trajectory and cell callbacks, then
  keeper/goal integration. Overall completion unmeasured; full match unfinished.


## 2026-10-29 — exact ball velocity projection

- Added pure CE4A/CE4D signed16-bit ball velocity projection with replaceable
 64-word quarter-wave content. Preserves quadrant/sign/index and horizontal+64
 wrap; independent from player movement/clamping. No session/gameplay writes.
- Real regression failed with stub then passed quadrant boundaries, zero and
 signed-minimum wrap. Full default Make suite/build pass cleanly; fresh review
 found no material issue. CMake unavailable.
- Exhaustive original CPU/native comparison matches both axes for all256direction
 bytes. Repeatable checker and scope in NATIVE_BALL_VELOCITY.md; private evidence
 ball-velocity-repeatable. No skipped original instructions, calls or frames.
- Next: E778 ball coordinate advancement until next cell, preserving fractions
 and explicit cell/arrival callbacks. Overall port remains unmeasured.


## 2026-10-29 — numeric ball coordinate/cell advancement

- Added prepared E778 numeric ball loop: repeated same-frame16-bit additions,
  new-cell boundary, destination high-byte snap with fraction retention, and
  original outside-field forced destination. Already-at-destination is untouched.
  Coherent interior inputs required; stationary nonarrival rejects instead of
  hanging. Full cycle bound protects constructed calls. Live frame/callback and
  fractional session-state integration remain separate and unfinished.
- Real regression failed with stub then passed. Full default Make suite/build
  pass cleanly. Seventy original/native cases match both coordinates, cell,
  addition count, callback boundary and outside flag. No original calls or frames
  are skipped in this numeric comparison. Fresh review found no material issue.
  CMake unavailable. Evidence/command in
  NATIVE_BALL_STEP.md; private ball-step-repeatable report.
- Repeated earlier nonterminal4cases and attack clock7cases against finalJ saves;
  all pass. Added29October Arabic daily entry with concise summary/bullets; earlier
  dates remain manually selectable and approved roadmap/design preserved.
- Next: saved fractional trajectory, per-cell defender/frame callbacks and final
  keeper/goal integration toward one complete match. Overall port unmeasured.


## 2026-10-29 — composed trajectory verification

- Extended the ball-step oracle to feed approved projection content into native
  direction projection and then numeric stepping. All256direction cases match
  original composed projection/coordinate output; combined326cases pass. Private
  evidence ball-step-composed. This tests the math connection, not live callbacks.
- Added repeatable E73E setup comparison: existing native direction geometry and
  ball velocity projection together match seven original prepared setup cases.
  Original count/elapsed resets are observed but not yet native session behavior.
  Evidence ball-setup-repeatable; tool check_ball_setup_original.py.
- Updated29October note to326numeric movement comparisons; earlier dates and
  manual navigation preserved. Native next step remains saved fractional flight
  initialization and first-cell/arrival callback boundaries, with no guessed low
  coordinate bytes. No new playable-match or renderer claim.


## 2026-10-29 — saved fractional flight and first-cell boundaries

- Connected phase46 numeric setup using explicit fractional input, direction and
  private projection content; clears original prefix/elapsed, removes selected
  progress and saves full16-bit coordinates/velocity. Phase47 waits for explicit
  frame acknowledgement; numeric advancement stops48 before cell callback or49
  before arrival callback. Phase50 preserves original already-arrived early return
  without callback/snap. Session20K355bytes, readers1..19.
- Setup/unit regression failed before implementation. Already-arrived callback
  distinction separately failed RED before phase50 fix. Default Make suite/build
  pass cleanly. Fresh review caught unsigned stream negative-wrap parsing; actual
  CLI accepted -4294967295 as1 (RED), fixed with complete-token from_chars and
  malformed-content regression. Post-review suite and focused checks pass.
- Six original/native scenarios (eleven setup/body boundaries) match prior state
  and trajectory fields. Explicit post-wait entry does not emulate live frames or
  callbacks. Ruling: phase50 unused velocity normalizes0 because preflight does
  not capture original stale slots; oracle supplies nonzero stale values and does
  not claim equality for that metadata. Cost if future code needs stale velocity:
  capture before flight instead of reading normalized metadata.
- Added private CMJBALL1 exporter and strict CLI content/input handling, atomic
  replay, quoted interactive path/save, trajectory-tail validation and oldJ read.
  Evidence/commands: NATIVE_SAVED_BALL_FLIGHT.md and saved-ball-flight-reviewed.
  Prior clock7cases and approach6cases pass against finalK. Engine source private.
- Next:800F per-cell defender callbacks and800C arrival behavior, real frame/task
  scheduling, keeper/goal flow. Overall completion unmeasured; full match unfinished.

- Extended the same flight increment through original87E1: native cell callback
  appends eligible defenders while preserving prior entries/duplicates and capacity,
  then resumes47. Real regression failed before implementation and passed after.
  Seven original callback cases pass, including saturated and nonempty prefixes,
  borrow sample255/254 and stunned candidates. Review found no material issue.
- Two complete prepared numeric flights now match original at every native saved
  cell/callback to arrival, including queued defender accumulation. They freeze
  per-frame task/input state and stop before arrival encounters; no live-match
  claim. Evidence: full-prepared-flight-repeatable. Full callback Make suite and
  production build passed cleanly. Next is arrival8835/keeper/goal continuation.


## 2026-10-29 — empty arrival and loose-ball possession

- Added empty arrival49/early-return50 dispatch to51 loose presentation boundary or
  52 keeper entry; queued defenders reject atomically. Prepared loose recovery
  enters8BE5 without a second jitter/special-owner cleanup, clears flight contexts
  and saves generic recovered owner6. Either side continues to valid control or
  keeper/story routing. Snapshot21L355bytes retains readers1..20.
- Real branch/recovery tests failed before implementation, then passed. Side11
  possession regression requested in fresh review also passes. Full default Make
  suite/build pass cleanly (cm-loose-final-suite/build.log). CMake unavailable.
- Four original DD11 branches and three prepared8BE5 recoveries match, including
  side11 possession. Three whole frozen numeric trajectories match each cell and
  callback; free loose route also matches prepared recovery and native control-ready
  continuation. Evidence: loose-arrival-both-sides and
  full-flight-loose-recovery-corrected. Display45 is explicitly not executed or
  claimed equivalent; live scheduling and arrival encounters remain unfinished.
- Correction: earlier approach-repeatable loose/threshold fixture retained aerial
  subtype128. Its numerical comparisons matched but its labels overstated ground
  branch coverage. New canonical cases explicitly set subtype0; a seventh explicit
  aerial case and expected-loose assertion now pass. Evidence:
  approach-final-subtype-guard. Historical reports retained without rewriting.
- Ruling: reuse shared recovery with an explicit generic phase6 output for this
  caller, retaining clearance37 default for existing callers — original8BE5 is a
  generic owner handoff; using37 would require unrelated clearance metadata and
  reject opposing-side saves. Cost if caller semantics change: revise output route.
- Next: prepared keeper entry52, queued arrival encounters, keeper/goal continuation
  and live frame/input/presentation toward a complete match. Overall unmeasured.


## 2026-10-29 — saved keeper entry and stunned decision

- Added native52→53 normal keeper selection or54 stunned entry; source opposing
  keeper status selects the route. Prepared54→55 saves original random decision
  before display12. Normal53 narrowly allows keeper ID11 in selected prefix;
  ordinary field/flight queues remain field-player-only. NewM358bytes appends
  keeper cursor/result context, preserving readers1..L and strict phase invariants.
- Real adapter regression failed with unimplemented stub then passed. Original
  observer initially rejected the speculative three-threshold fixture; actual
  scan8928 contains two reachable bytes16,0, followed by opcodeA0. Corrected API
  and tests use exactly two thresholds ending0, result0..1. Ruling: expose that
  two-byte content explicitly — avoids treating code bytes as a third outcome —
  cost for another ROM with altered algorithm is a content/parser extension.
- Four original status routes and all256random decisions match represented saved
  bytes with every original instruction executed, no skipped presentation calls.
  Five complete prepared flights match every saved cell/callback, including ready
  and stunned keeper setup and the stunned decision. Evidence: keeper-entry-final
  and full-flight-keeper-final. Frozen frames/explicit waits remain limits; normal
  keeper comparison/outcome/rewards and queued arrival encounters still pending.
- Default Make suite and production build pass with no compiler warnings/errors.
  Fresh review found no material issue and independently rejected forged keeper
  prefixes, ownership, replay and mixed CLI operations. CMake unavailable. Detailed
  evidence and reproduction commands are in NATIVE_KEEPER_ARRIVAL.md.


## 2026-10-29 — persistent prepared normal keeper comparison

- Connected53 to56 before display9 using existing keeper profile/move preparation,
  energy/variation/comparison and story override. Caller explicitly acknowledges
  keeper choice and supplies action/subtype/opponent-team002B. Saved621 context,
  shot062B distance, opposing keeper stats and raw primary type reach comparison;
  story uses captured446/raw43C. No random, clock, score or ball writes inferred.
- Snapshot23N361bytes saves cursor/result plus defense action/modified subtype/
  effect; legacyM358 entry and stunned decision load with absent metadata0.56
  preserves the sole opposing keeper prefix and valid normal status;55 new move
  metadata must remain0. Invalid/replay/content/CLI and malformed tails reject.
- New adapter test failed with stub then passed.254 original852F..8599 observations
  match every represented save byte, covering4actions, types1/33/64, critical random
  boundaries, powers0/100/65535, timer/penalty and story cases. No calls skipped;
  stat-loader8, energy, comparison and story all run as original instructions.
- Five complete prepared flights match each saved cell/callback; free normal case
  additionally matches prepared keeper comparison. Prior4status and256stunned
  decisions pass again with N. Evidence: keeper-comparison-expanded,
  full-flight-keeper-comparison and keeper-entry-N-regression. Explicit frozen
  frames and acknowledged choice presentation remain scope limits.
- Next: commit keeper outcomes/rewards and score/possession/restart state, queued
  arrival encounters, then live frame/presentation toward a full playable match.
- Fresh review found no material defect. Expanded the session oracle to254cases
  with raw type64, defense subtype128 and contexts0/4; all pass. Final default
  Make suite/build pass cleanly; CMake unavailable. Engine remains private.


## 2026-10-29 — persistent keeper saves and recovery handoffs

- Added prepared56 result2/3 save resolution: raw keeper experience selector3 once,
  timer/penalty, rebound2 recovery, catch57 keeper ownership and punch58 no-jitter
  recovery. Resolved flight/decision contexts clear. Typed57 permits only own
  keeper11/side11/prefix1; field-player ownership validation remains strict.
- Punch58 explicitly retains elapsed1 and stored earlier ball cell. Native8BE5
  wrapper advances field recovery into generic6 for either side without duplicate
  jitter/cleanup/clock/reward. Correction in focused test: ball cell stays stale
  through recovery, as original comparisons show; later control setup refreshes it.
- Both real adapter and punch recovery tests failed with unimplemented stubs then
  passed.128 original non-goal handlers match represented save bytes, including
  reward saturation, raw keeper types1/33/64, timer wrap, penalty gates and punch
  directions/signs. Actual original reward/status/ownership/sound helper instructions
  execute unchanged, no calls skipped. Selected prepared punch recoveries match.
- Seven complete prepared numeric flights match each saved cell/callback, including
  normal keeper comparison and rebound/catch/punch handoffs plus punch recovery.
  Evidence: keeper-saves-final and full-flight-keeper-saves-final. Presentation9,
  keeper control and live audio/render/frame scheduling remain explicit limits.
- Snapshot24O retains361bytes/readers1..N. No score/goal capture inferred: guarded
  non-goal value helper does not use or commit score/kickoff placeholders. Goal/
  special turnover handling and persistent scoreboard remain next work.
- Review found an existing phase>=52 guard dereferencing absent flight contexts
  in new57/58. Default optimized tests masked the invalid access. Fresh full-core
  assertion build aborted on optional engagement (real RED); narrowed guard52..56,
  same assertion test passes GREEN. Final default Make check/all and original
  128handlers/7joined flights pass after fix. Eleven prepared punch recoveries
  match; joined punch also reaches native control-ready (separately checked,
  not claimed original live presentation equivalence).
- AssertionO1 build retains three pre-existing GCC maybe-uninitialized warnings in
  finish's optional-vector move, also seen before fix; defaultO2 suite/build are
  clean. Deferred diagnostic investigation; no demonstrated gameplay fault there.
  CMake unavailable. No other material review findings. Engine code stays private.


## 2026-10-29 — persistent score and scorer reward

- Added55/56→59 scored-goal preparation for result0 or result1 with subtype&63<3.
  Explicit0028/0029/0629 baseline avoids guessed scores; saved score must agree.
  Award scorer selector0 once, increment side0 byte with wrap, retain right score/
  kickoff/time/RNG/ball/possession/status and save before goal presentation.
- Snapshot25P365bytes adds4score metadata bytes and readers1..O.59 requires score,
  retained flight/decision and matching normal/stunned prefix. Replay/special
  turnover/invalid baseline/format errors reject. Other phases may retain scores.
- Real native test failed with stub then passed.72 original goal preparations match
  represented state/new score/reward bytes, including normal/stunned goals, raw
  subtype masks, types1/33/64, score wrap and reward saturation. Evidence:
  scored-goal-final. Eight full prepared numeric flights match each
  cell/callback and eligible normal/stunned goal registration: full-flight-scored-goal.
- Ruling: stop at85F0 after85ED score increment, before goal presentation1 — initial
 85B7 target blocks atC52A/CAAE and needs a presentation acknowledgement. All prior
  original instructions/reward calls execute; no skipped calls. Cost: presentation1,
  celebration30/987B, side flip, cleanup and kickoff remain explicitly pending.
- Final default Make check/all pass cleanly. Fresh assertion core builds pass both
  scored-goal and keeper-save tests; CMake unavailable. Review found no gameplay
  defect, but identified a fixed-owner scorer fixture in the comparison tool.
  Regraded as verification-important because other inputs would mislabel coverage:
  current-owner assertion fails on owner3 RED; compute0300+12*owner, then72cases
  pass for each owner2/3 (144total). Evidence: scored-goal-final and
  scored-goal-owner3-green. Persistent scorer-type assertion prevents recurrence.
  O1 assertion build retains previously logged GCC optional-vector diagnostics.

- Closing checkpoint: engine source is private/local through c2fbd94; Arabic daily
  note published separately as website-only main8fc4c2e. Pinned GitHub raw source
  matches local index.html; no browser/Pages rendering claim. Next original987B
  research shows it includes post-goal presentation/menu and sets062D ball mode0;
  do not skip that UI silently when adding85BF keeper-cleanup/kickoff continuation.


## 2026-10-29 — prepared post-goal kickoff handoff

- Added phase59→60 after explicitly acknowledged goal presentations and post-goal
  menu with captured ball mode0. Switches side, clears defending keeper timer/
  penalty/stun, retains penalty duration and score, sets kickoff4, preserves the
  original keeper prefix and saves the prior scorer until actual kickoff setup.
- Real focused regression failed with the unimplemented stub, then passed. Full
  Make check/build pass. Twelve original85BF..85DD prepared comparisons match
  all represented bytes; no original instructions skipped. Private evidence:
  post-goal-first-comparison. Snapshot26Q365bytes retains readers throughP.
- The original presentation/menu and actual kickoff setup remain unfinished.
  Overall port progress is unmeasured; no complete playable match exists yet.
- The user authorized pushing engine code to public GitHub branch work; main
  remains the website. ROMs, patches and derived private inputs remain outside Git.


## 2026-10-29 — kickoff team and receiver calculations

- Original kickoff DB4B..DB61 can choose a side from opponent team and random;
  team3 always chooses side0. Native pure selection reproduces that rule. Later
  DAF5..DB0B selects one of five caller-supplied receiver slots, with values5..7
  wrapping once. Both original setup phases remain separate from the saved session.
- Two real focused tests failed with stubs, then passed. Original CPU comparisons
  match all65,536 team/random combinations and512 receiver inputs for both sides,
  with all original instructions in each numeric segment executed. Evidence:
  kickoff-selections-final. Original formation/reset, post-goal menu, positions,
  presentation and live kickoff are not included; no match-completion claim.
- The full Make suite/build pass. Code is pushed to GitHub work; ROM and derived
  data stay outside Git. Overall progress remains unmeasured. Next: trace original
  formation/reset and connect kickoff selection to a coherent persistent session.


## 2026-10-29 — original kickoff formation placement

- Added native DC07 formation placement for all22 raw player coordinate pairs,
  explicit on-ball/off-ball content, four formations each side and both possession
  directions. Preserves fractional bytes and original keeper Y-low clearing.
- Focused regression failed with stub then passed.128 bounded original comparisons
  match every X/Y word: both sides, all16 formation combinations and four randomized
  original coordinate sets. Every DC07 instruction executes; no frame/presentation
  claim. Evidence: kickoff-formation-final. Full Make suite/build pass cleanly.
- Keeper coordinate Y high bytes are not represented by the existing persistent
  session. Raw22-slot API retains them explicitly; session integration must capture
  that state rather than silently discarding it. Actual reset, kickoff/UI and a full
  playable match remain unfinished; overall progress remains unmeasured.


## 2026-10-29 — saved kickoff formation and keeper coordinates

- Added phase60→61 at an explicitly prepared boundary after original earlier reset
  and team selection. Selected side is explicit, original DAE9 sets owner=side+8,
  DC07 places22 raw records and keeper Y high bytes are saved. The existing keeper
  timer/duration/penalty aliases are handled as original X/Y bytes; previous Y high
  bytes are overwritten by DC07 and need not be guessed. Score is retained.
- Snapshot27R368bytes appends presence plus two keeper Y bytes; readers throughQ
  remain supported. Tests cover both sides, aliases/fractions, Q legacy read,
  malformed/truncated metadata, invalid inputs, replay and immutable source.
- Real session test failed with stub then passed.64 original prepared DAE9..DAF5
  comparisons match all represented bytes for both sides/all16 formation pairs
  with randomized record words. Original CPU executes every instruction, with no
  skipped calls. The CLI supports quoted private content paths; exporter output
  matches original observed tables. Evidence: kickoff-session-first. Full Make
  check/build pass; CMake unavailable.
- Ruling: state is supplied after earlier original reset/team choice; those effects
  are not silently simulated. Cost: live reset/menu, second original side field,
  receiver/ball setup and playable kickoff are still pending. Overall unmeasured.

## 2026-10-29 — saved kickoff receiver selection

- Added phase61→62, saving the selected receiver while preserving the current
  carrier, formation, keeper coordinates and other session state. Explicit
  random byte and five candidate slots; no implicit ball positioning or display.
- Snapshot28S370bytes adds receiver presence/value; legacy R formation snapshots
  load, older versions reject new phase. Regression checks cover roundtrip,
  malformed/truncated data, invalid slots and replay without mutating input.
- Focused test failed with an unimplemented stub, then passed. All 512 combinations
  of side and random value matched original DAF5..DB0B with every instruction
  executed; compared the complete represented snapshots. Private evidence:
  kickoff-receiver-session-first. Earlier reset/side choice, ball placement,
  presentation and live kickoff remain outside this comparison.
- Overall completion is unmeasured; there is no complete playable match.

## 2026-10-29 — saved kickoff ball placement

- Added phase62→63: original DB0B/E6EC copies integer ball position from the
  existing side+8 carrier and computes the cell before kickoff presentation.
  Receiver remains saved and ownership is not transferred prematurely. Snapshot29T
  retains the same size and legacy readers; phase63 is rejected by older formats.
- Focused test failed on stub, then passed. 256 original comparisons cover both
  sides and 128 interior positions each, with complete represented snapshots and
  every original instruction executed. Out-of-field coordinates are rejected by
  existing session validation and are not claimed. Original fractional ball bytes
  stay unchanged but are absent from the saved model. Evidence:
  kickoff-ball-session-interior. Earlier reset, presentation and receiver handoff
  still need work; overall completion remains unmeasured.

## 2026-10-29 — prepared kickoff possession handoff

- Added phase63→64 after explicit caller acknowledgement of display36. Original
  DB13..DB1E assigns the saved receiver as owner and resets controlled player
  `$05FD` to1; ball position and kickoff formation context remain saved. Stops
  before stack reset and E0DF control setup. Snapshot30U370bytes reads through T.
- Focused test failed with stub then passed. 40 original CPU comparisons cover
  both sides, all five original receivers and four old control bytes, comparing
  full represented snapshots. Evidence: kickoff-handoff-first. 256 previous ball
  comparisons reverified with new snapshot version. Display itself is not run,
  and its effects are caller-supplied; full kickoff/match remain unfinished.

## 2026-10-29 — prepared kickoff into control setup

- Connected phase64 to existing E0DF..E145 control setup with saved keeper Y
  carried into phase9 and receiver metadata consumed. Snapshot31V keeps layout,
  supports phase9 keeper coordinates, and older version readers stay supported.
- Focused test first rejected the previously unsupported kickoff phase, then
  passed after integration. 32 original observations/comparisons cover both sides
  and all sixteen formation pairs, including complete represented snapshots;
  goalkeeper Y is unchanged in these cases. Evidence: kickoff-control-original-first
  and kickoff-control-native-first. The original display36 is acknowledged rather
  than executed; control displays use disclosed host acknowledgements.
- Deliberate boundary: following phase9 actions with saved keeper Y are rejected
  until their original effects are traced. Earlier reset/team choice, live kickoff,
  complete match and full presentation remain unfinished; progress unmeasured.

## 2026-10-29 — kickoff control polling with keeper positions

- Allowed saved keeper Y context through control phases9–12 (wait, clock, action
  menu and AI cell boundary), while keeping later phases guarded. Snapshot32W
  maintains the same layout and validates legacy version31 kickoff phase9.
- Focused saved-menu regression failed on the old validator then passed. 80
  original poll comparisons cover both sides, wait/timer and multiple input
  combinations including AI cell change: 38 wait, 28 clock, 4 menu, 10 AI;
  all represented bytes match. Original display36 assumed acknowledged; host
  frame acknowledgements, no actual NMI/PPU/audio. Evidence: kickoff-poll-final.
  The 32 control-setup comparisons were reverified in snapshot32. Later clock,
  menu/AI continuations with saved keeper positions are still guarded.

## 2026-10-29 — ordinary kickoff clock/energy

- Saved goalkeeper coordinates now survive phase10→13 via ordinary control clock,
  keeper recovery and energy updates. Snapshot33X allows the new keeper-context
  phase13; prior formats still load. Unsupported goal-area/added-time/period-end
  paths reject atomically instead of returning a state with unsupported context.
- Phase13 serialization regression failed on the earlier guard, then passed.
  Original E1E7 comparisons for both sides/all16 formation pairs matched all
  represented bytes in32 ordinary cases. Evidence: kickoff-clock-first. Earlier
  80 control-poll comparisons pass in the new format: kickoff-poll-snapshot33.
  Original display36 assumed acknowledged; no actual NMI/audio/PPU. The field
  round and exceptional clock routes still require original comparison.

## 2026-10-29 — first idle field round after kickoff

- The previously enabled phase13 can use the existing field-round operation and
  return to phase9 while retaining goalkeeper Y. Thirty-two original CPU
  comparisons match all represented bytes across both sides and all formation
  pairs with idle input at all22 polls. Evidence: kickoff-field-round-first.
- This covers one prepared idle cycle only, with kickoff display acknowledged
  and host frame acknowledgements. Other field inputs, contact, menu/story
  interruptions, exceptional clock branches, real presentation and live matches
  require further work. No additional gameplay behavior was invented.

## 2026-10-29 — chained kickoff movement and menu opening

- Verified 256 native sequences from prepared kickoff control setup with every
  native output feeding the next operation, across both teams/all16 formation
  pairs/eight directional and button patterns. All6,448 represented boundaries
  match original observations, covering496 field-round entries,480 returns to
  control and16 late menu requests. Evidence: kickoff-cycles-first. The sequence
  stops after two rounds or at menu/AI boundaries; no complete-match claim.
- Enabled saved keeper coordinates in phase18 for direct/late action-menu opening.
  Snapshot34Y retains370bytes and legacy readers. Regression failed at the old
  validation guard, then passed; roundtrip and old-format rejection covered.
  32 original menu-opening comparisons match all represented bytes. Evidence:
  kickoff-menu-first. Keeper context after menu confirmation remains guarded.
- Original display36 remains explicitly acknowledged; host frame acknowledgements
  do not reproduce NMI, PPU, audio or real presentation timing. Earlier kickoff
  reset, exceptional clock paths and a full playable match remain unfinished.
- Final Make check/all completed with no compiler diagnostics. Source-only Arabic
  daily note update accompanies the engine checkpoint; no browser rendering claim.

## 2026-10-29 — kickoff menu confirmation and ordinary shot preparation

- Enabled retained kickoff keeper coordinates in phases19/20, connecting the
  existing menu-confirmation and basic-shot operations to saved kickoff menus.
  Snapshot35Z370bytes retains readers throughY; legacy Y menu loads, Y claiming
  new confirmed/prepared keeper context rejects. Regression failed on old guard
  before passing; roundtrip/compatibility checks cover both new boundaries.
- 64 native sequences match320 represented original boundaries: all16 formation
  pairs, direct/late menu requests and combined/staged shot choices. Actual raw
  stamina cost80 and power38 match for these inputs, with goalkeeper coordinates
  retained. Repeating preparation rejects in all64 cases. Native output feeds
  the next command throughout. Evidence: kickoff-shot-choice-final.
- Original observer supplies explicit ordinary subtype0/variant0 with synthetic
  return after preparation and disclosed host frame acknowledgements. No actual
  kickoff display/NMI/PPU/audio is executed. Special moves, other actions and
  later defender/flight steps with keeper context remain pending; no full match.
- Final Make check/all pass with no compiler diagnostics. Routine Arabic daily
  note updated separately on website main; source verification is not a browser
  rendering claim. Source/ROM-derived private data remain separate.

## 2026-10-29 — kickoff unopposed shot through destination

- Retained keeper Y at27/43/45/46; snapshot36a370bytes explicitly follows Z and
  retains prior readers. No-field-defender kickoff shot joins final clock,
  ordered field movement, prepared attacking clock and saved destination.
- ROM comparison exposed two missing initial script1 effects (facing64→0,
  motion0→2). Watched original writes ED0F/ED32/bank24:8701 confirm source;
  regression failed before fixing the completed-presentation capture. Original
  observer includes script1 under host acknowledgements; script8 explicitly
  acknowledged later. No native rendering/frame-frequency claim.
- 16 formation-pair sequences match64 represented boundaries through destination.
  Evidence: kickoff-launch-final. Comparison helper's initial wrong flight-context
  offset was corrected to existing339..343 layout; original RAM is authoritative.
  Numeric flight and selected defenders with keeper context remain next work.
- Final Make check/all pass. Private ROM and derived inputs remain outside Git.

## 2026-10-29 — numeric kickoff ball flight

- Extended retained keeper coordinates to flight phases47–49, snapshot37b370bytes.
  Real setup regression failed on old validation, then passed. Compatibility,
  roundtrip, cell callback and arrival tests cover retained goalkeeper coordinates.
- 16 original CPU flight sequences match456 complete represented boundaries,
  using each native output as the next input. Captured original fractional ball
  bytes are explicit input. Evidence: kickoff-flight-first, sourced from
  kickoff-launch-flight-inputs. All16 examples reach keeper-bound arrival with
  no queued field defenders; actual defender encounter coverage is not claimed.
- Original numeric frame waits explicitly acknowledged; no NMI/PPU/audio or
  task/random updates. Arrival resolution and keeper continuation remain next.
- Final Make check/all pass; raw ROM and all generated observations stay private.

## 2026-10-29 — kickoff flight to normal goalkeeper entry

- Added retained keeper coordinates at52/53; snapshot38c370bytes supports legacy
  readers and rejects older markers claiming the new combinations. Regression
  failed on arrival guard before passing; roundtrip, legacy input and replay
  rejection covered. Actual keeper setup selects11 and preserves both keeper Y.
- Extended original flight comparison through empty arrival and normal keeper
  setup without skipped original calls. 16 sequences match488 boundaries up to
 8509 before presentation. Evidence: kickoff-keeper-entry-first. Stunned keeper,
  actual keeper choice/comparison and outcome are still pending with this context.
  Host frame acknowledgements remain explicit; no full hardware/match claim.
- Final Make check/all pass; ROM and private generated evidence remain outside Git.

## 2026-10-29 — prepared kickoff normal keeper comparison

- Retained keeper Y through phase56; snapshot39d370bytes reads previous formats.
  Real comparison regression failed on old guard, then passed; decision roundtrip,
  legacy c setup, forbidden old marker and replay rejection covered.
- Extended original flight verifier with explicit keeper-choice input after
  acknowledged presentation. 128 comparisons (4actions×2subtypes×16formations)
  match; with preceding flight/entry there are616 matched boundaries. All128
  original results are0 for these actual inputs. Do not claim save/rebound outcome
  coverage. Evidence: kickoff-keeper-decisions-first. Goal registration and
  outcome/presentation remain next work; no hardware/full-match claim.
- Final Make check/all pass. Source and private generated content stay separate.

## 2026-10-29 — saved goal from the kickoff shot

- Retained goalkeeper Y through score/scorer reward phase59. Snapshot40e370bytes
  reads earlier formats; old d decisions load, old d goal-with-context rejects.
  Regression failed on prior guard, then passed. Goal roundtrip and replay rejection
  cover saved state. Existing goal logic/reward arithmetic is reused.
- 128 goal registrations match original85AC..85F0 in the chained flight/keeper
  verifier; total744 represented boundaries across16 flights and128 keeper branches.
  Evidence: kickoff-scored-goals-first. Inputs all classify as original result0;
  save/rebound coverage is not implied. Display9 is explicitly acknowledged,
  and goal celebration/menu, original rendering/NMI/audio and full matches remain
  unfinished. No ROM/private generated content is included in Git.
- Final Make check/all pass without compiler diagnostics. Public Arabic daily
  note updated separately on main; no browser rendering verification claimed.

## 2026-10-29 — prepared post-goal restart with kickoff coordinates

- Preserved goalkeeper Y at60; snapshot41f370bytes retains prior readers and
  rejects old markers claiming this new combination. Regression failed before
  guard support; restart roundtrip, legacy e scored goal and replay rejection
  covered. Existing side switch/keeper cleanup preserves score and raw keeper Y.
- Original85BF..85DD compared for128 post-goal handoffs in chained verifier;
  total872 boundaries match. Evidence: kickoff-postgoal-first. This explicitly
  assumes completed celebration/menu and captured ball mode0, and does not run
  those presentations or the next kickoff's earlier reset/team selection.
- Final Make check/all pass. ROM and all generated private evidence stay outside Git.

## 2026-10-29 — controlled keeper save handoffs with kickoff coordinates

- Retained keeper Y through rebound2/catch57/punch58. Snapshot42g370bytes retains
  previous readers; real rebound regression failed under prior guard then passed.
  Roundtrip, old f decision input, forbidden old output and reward replay covered.
- Constructed post-comparison result2/3 and action inputs on captured kickoff
  positions. Original861C/8646 handlers match all128 cases:64 rebounds,48 catches,
 16 punches. Evidence: kickoff-save-handoffs-first. This does not assert naturally
  occurring non-goal classifications for the actual kickoff shots (all prior
  keeper comparisons resulted0). Keeper display9 explicitly acknowledged; later
  recovery/keeper play remains pending. No hardware/full-match claim.
- Final Make check/all pass without compiler diagnostics. Arabic daily summary
  updated on website main separately; no browser rendering claim. Private ROM,
  observations, derived content and state remain outside source control.

## 2026-10-29 — controlled punch recovery with keeper coordinates

- Extended saved keeper Y to recovered field-owner phase6, snapshot43h370bytes.
  Punch-recovery regression failed under the old phase guard, then passed.
  Recovered save roundtrip, legacy g punch handoff, old g output rejection and
  replay rejection covered. Rebound jitter recovery explicitly remains guarded.
- Original comparison found native motion flag2 versus original0. Traced original
  write to bank31:EC3E during cooperative recovery wait; regression reproduced
  missing animation acknowledgement, then passed with copied-state clear. No
  rendering or NMI implementation is implied by this prepared presentation effect.
- 128 controlled non-goal handlers and16 punch recoveries match all represented
  bytes (144 comparisons). Evidence: kickoff-punch-recovery-final. Keeper result2/3
  is constructed input; following control/keeper-area/story remain pending.
- Final Make check/all pass without compiler diagnostics. Private generated
  content and ROM remain outside Git; source-only public daily note updated.

## 2026-10-29 — controlled rebound jitter recovery

- Restored py65 under private analysis/python-deps after temporary execution files
  disappeared. Fresh Make check/all confirmed the committed punch checkpoint;
  regenerated verification logs are kept under analysis rather than /tmp.
- Enabled retained keeper Y through existing rebound jitter recovery2→6. Original
  rebound motion flag remains2 in tested inputs, distinct from punch acknowledgement.
  Real regression failed on the deliberate guard before passing with support.
- 128 controlled non-goal handlers plus64 rebound and16 punch recoveries match
  full original state (208 comparisons). Evidence: kickoff-rebound-recovery-first.
  Constructed result inputs, disclosed host acknowledgements and hardware limits
  still apply. No new snapshot version was needed.
- Validated early boundary exits: unsupported keeper-context phase5 rejects without
  source mutation. Regression failed when the validation was removed and passed
  after restoration. Actual next control/keeper-area/story phases remain pending.
- Final Make check/all pass for rebound recovery. Private content stays outside Git.

## 2026-10-29 — recovered ball to control or keeper-area pause

- Retained keeper Y at1/7, snapshot44i370bytes. Real player-control serialization
  regression failed before support, then passed. Both continuation roundtrips,
  old h marker rejection and guarded story branch covered. All return branches
  validate to avoid exposing unsupported state. Existing control setup is reused.
- 80 original continuations match:64 rebound tails to typed keeper-area pause7,
 16 punch tails to player-control1; all16 then match control setup9. Combined
  verifier compares304 complete states including128 save handlers and80 recovery
  calls. Evidence: kickoff-recovery-tail-first. Non-goal result inputs remain
  constructed; no actual NMI/PPU/audio or full-match claim. Keeper-area and story
  handling remain next integration work.
- Final Make check/all pass without compiler diagnostics. Raw ROM/private data
  remain outside Git. Public daily summary is maintained separately on main.

## 2026-10-29 — prepared keeper-area interaction setup

- Added phase7→65 numeric8B4A..8B73 setup: preserve/generate variant, route1/2
  based on area/possession, clear prefix, counter255 and exact-cell field scan.
  Captures new KeeperAreaContext; optional kickoff keeper Y remains retained.
  CLI/interactive prepare-keeper-area support the operation before its menu.
- Snapshot45j372bytes adds presence/counter at370/371. Readers throughi remain;
  phase65 requires context255 and consistent area/route. Old positive fixtures
  are truncated to their actual prior lengths, and original comparison writers
  now pad the two unused new bytes for older phases.
- Real focused test failed with stub then passed. 64 recovered rebound-area
  setups match all original bytes (keeper-area-original-first);24 constructed
  cases cover both possession sides/areas/variants and selection counts0/4/5
  with exact scan order (keeper-area-routes-first). Every original setup call
  executes. Fixture non-goal results remain constructed inputs; no menu, rendering,
  audio/NMI or complete-match claim. Detailed reference: NATIVE_KEEPER_AREA.md.
- Roundtrip, legacy i area entry, forbidden i phase65, truncated/bad/missing new
  metadata, invalid area and replay checks pass. Full Make check/all pass.
- Interactive keeper-area command matches the flag-based saved output. Final
  lab rebuild and earlier full Make check complete without compiler diagnostics.
  Daily Arabic note is updated separately on main with source-only verification.

## 2026-10-29 — prepared own-area menu and AI entry

- Added route2 menu entry from65: radius10 field scan replaces earlier exact-cell
  prefix, keeper warnings/overlay/human route2 display requests, captured common
  presentation bytes and side0 cursor reset. Side0 reaches18; side11 reaches66
  before original D36E choice. Final animation facing0/64 is an explicit observed
  input because that renderer state is not implemented; no guessed facing rule.
- Snapshot46k372bytes permits area metadata at18/66 and reads j65. Real regression
  failed with stub then passed; both side saves/old marker rejection, legacy j
  setup, bad facing/route and replay covered. Route1 remains unsupported here.
- 24 controlled original menu cases match full represented snapshots and ordered
  presentation requests, including warning priority, stale fields and nearby
  player caps. Evidence: keeper-area-menu-first. Original uses disclosed host
  frame acknowledgements; facing supplied explicitly. No NMI/PPU/audio, AI choice
  or subsequent action execution claim. Next: route2 human selection/confirmation.
- Full Make check/all pass without compiler diagnostics. Source-only public notes
  remain on main; ROM and derived evidence remain outside Git.

## 2026-10-29 — human own-area choices and defensive-entry correction

- Retained area context at human confirmation19; snapshot47l372bytes reads prior
  k menus but rejects old k confirmed-area output. Focused regression failed on
  the old phase-context guard before passing. Roundtrip, legacy input and route2
  ordinary-shot rejection covered.
- 512 original poll comparisons (all256pressed bytes×inactive/active seeds) match
  complete snapshots and sprite requests:280 menu states,232 confirmations,
  original chosen actions1/4/6. Evidence: keeper-area-choices-first, including24
  opening cases. Action execution remains pending; final facing during opening
  is still explicit observed presentation input.
- Tracing D36E beyond entry showed controller polling and human defender choice,
  correcting the earlier AI label. Renamed phase66 to keeper_area_defense_pending;
  its serialized value and original boundary are unchanged. Documentation/tool
  scope now says defensive entry. No actual defensive input or AI choice claimed.
- Full Make check/all pass without compiler diagnostics. Source/private evidence
  remain separate; continuing into original defensive-menu initialization.

## 2026-10-29 — initial defensive choices and empty return

- Added phase66→67 for nonempty prefixes: cursor0, four FF action slots, canonical
  unused zero subtypes and explicitly captured0620 blink. Empty prefix returns68
  without initialization or cursor changes. Stops before D390 presentation or
  at8B8F area action entry. Real stub regression failed before passing.
- Snapshot48m382bytes adds defensive presence/4actions/4subtypes/blink at372..381.
  Old l phase66 inputs load; l cannot encode67/68. Comparison tools pad unused
  metadata, and old j/k/l fixtures retain their original372byte sizes.
- 27 controlled original comparisons match counts0/1/4, blink0/7/255 and flags
 0/128/211. Evidence: keeper-area-defense-init-first. Unread subtypes behind FF
  are normalized; no preservation claim for those irrelevant stale raw bytes.
  Roundtrip, legacy input, malformed/truncated/missing context and replay covered.
  Defensive menu rendering/input and action execution remain next tasks.
- Full Make check/all pass without compiler diagnostics. Private captured ROM data
  remain separate from the source; continuing with defensive prompt preparation.

## 2026-10-29 — defensive prompt requests and private menu content

- Added initialized67→69 prompt boundary, snapshot49n382bytes. Emits count label,
  blank sprite29 for each explicit slot and private selected-player subject label.
  Typed presentation requests gain default0 slot; earlier callers remain valid.
  Real stub regression failed before implementation. Roundtrip/legacy m input,
  forbidden m output, replay and four-icon slot checks pass.
- Added private CMJSHOTA content10:15bytes of defensive titles/direction rows,
  with strict action-ID validation and older1..9 readers retained. Exported private
  canonical20,081byte pack; stripping15bytes/restoring9 header exactly matches the
  earlier v9 payload. No ROM/derived pack is committed. Missing private menu data
  rejects instead of inventing labels.
- Original observer extended through D390 prompt to D3A6. 27 initializations and18
  prompt outputs match full represented state/slot-aware requests, including old
  blink/flags variants. Evidence: keeper-area-defense-prompt-first. Truncated,
  trailing, invalid action data and legacy pack misuse reject. Presentation uses
  explicit host acknowledgements; no renderer/NMI/audio or defensive input claim.
- Full Make check/all pass without compiler diagnostics. Interactive prompt save
  matches flag output. Source and private ROM/content/evidence stay separate.

## 2026-10-29 — defensive directions, back, blink and confirmation

- Added phase69/70 sampled defensive input and phase71 pause before D3D4 action
  preparation. Direction priority, B returning a slot, blink timing and icon slots
  follow original control flow. A records the selected defender without executing
  its action or charging stamina. Snapshot50o383bytes retains the exact confirm
  pressed byte so resumed preparation can process that same sample correctly.
- Private content11B adds seven field icons; canonical20,088byte pack reproduces
  prior A when stripped. A/9 missing-content polling and malformed/truncated/
  trailing packs reject. No ROM/content/generated observations are committed.
- All256 pressed bytes across four constructed UI seeds match complete state and
  ordered slot-aware requests:1,024 cases,640 waits/384 preparation pauses.
  Evidence: area-defense-polls-final. This uses captured area state and explicit
  host acknowledgements; not live rendering, AI or naturally occurring match cases.
- Focused keeper-save regressions, full Make check/all and diff checks pass.
  Pending roundtrip retains pressed input; replay/missing confirmation reject.
  Next: inspect the original confirmed defensive-action loader and connect the
  supported preparation paths without bypassing special-move choices.

## 2026-10-29 — ordinary defensive registration and menu return

- Connected phase71 through original no-special field actions3–6: subtype0/icon,
  next-slot subject prompt, last-slot wait and captured B return. A on a later full
  poll clears the overlay and returns72 before area action execution. An original
  comparison exposed the cleanup request; a failing regression then confirmed
  its addition. No stamina is charged. Actions0–2 preparation and unrepresented
  full-prefix directional scratch writes explicitly reject atomically.
- Snapshot51p stays383bytes. Legacy o pending71 and partial70 remain readable;
  old o full-prefix/new72 states reject. Roundtrip, replay, early completion,
  missing icons and unchanged error inputs are covered.
- 1,433 original boundaries match full represented state/ordered requests:640
  registrations across actions3–6/four slots/ten IDs/four pressed samples,784
  full-prefix polls and nine chained native-save continuations. Evidence:
  defense-register-second. Host parent returns D568→8B8F and presentation/frame
  acknowledgements are disclosed; no live renderer, action execution or AI claim.
- Full Make check/all pass; all1,024 earlier input cases also match snapshot51.
  Fresh read-only review found no Critical/Important issues. Atomic rejection
  coverage was added before review completion. Further suggested legacy-partial
  and mixed-direction sample coverage is recorded in the plan for follow-up.
  Continuing into conditional defensive special-option loading.

## 2026-10-30 — defensive special availability

- Added private content12C rules and a non-mutating availability query. Field
  action2 checks the raw player type and optional teammate among slots1–10;
  actions0/1 bypass special options for the represented nonzero area variant.
  No-option choices register normally; an available special remains pending for
  its unimplemented chooser. No stamina is spent. Snapshot51p is unchanged.
- Original comparison:1,416 availability cases,1,404 ordinary registrations,
  and12 special pauses. Evidence: defense-options-first. The current pack also
  matched1,713 registration and1,024 polling cases. Malformed or missing rule
  data and unsupported player types reject. Full Make check/all passed.
- The requested independent reviewer hit its usage limit; the author inspected
  the change and corrected a version-C guard in the prompt comparison tool.
  Next: special-choice UI and affordability, then the resulting area action.

## 2026-10-30 — special chooser opening and stamina decision

- Added phase73: an available defensive special option opens a prepared menu and
  cursor before its first input. Snapshot52q385bytes stores the option; private
  content13D adds two cursor geometry bytes. Older content/snapshots remain
  readable within their existing scopes. Prior cursor Y is not represented.
- Original CPU matched240 controlled menu openings with full represented snapshots
  and display/cursor metadata; evidence defense-special-open-final. Legacy p phase,
  missing/invalid content and replay reject; flag and interactive saves match.
  Frame/display acknowledgements remain explicit; no live graphics or input claim.
- Added isolated wrapped-sign stamina decision without debit or cost loading.
  10,400 original numeric cases matched, including subtype0 and wraparound.
  Full Make check/all passed. CMake was unavailable on this host. A separate
  read-only review found no critical or important issues.
- Next: sample the chooser's A/B and direction input, load the chosen move cost,
  connect affordability and preserve the subsequent defensive action flow.

## 2026-10-31 — special defensive chooser input

- Added sampled input for the special chooser with phase74 wait and phase75 result.
  Direction precedes A/B, up and A win their respective ties, and the exit sample
  is retained for subsequent original flow. Snapshot53r is387bytes; private E
  content adds one cursor position byte. The previous D payload matches exactly.
- Original CFA9 execution matched2,304 controlled cases across all256 samples,
  opening/waiting cursors and synthetic option values; full represented snapshots,
  cursor position and selection/cancel outcomes match. Evidence: special-poll-first.
  Explicit yield acknowledgement; no renderer, move execution or stamina debit.
- Original research found that move registration also needs subtype-specific icons
  and expanded validation. Selection, move-cost loading and its affordability
  must be connected before claiming a complete defensive action.
- Added a separate inspection helper combining profile-based move cost/effect and
  the stamina decision without debit. The original loader/decision matched800
  controlled supported-profile cases; unsupported alternate encodings remain out
  of scope. It is not yet connected to phase75. Original effect $0445 must not be
  confused with existing action field $0444. A read-only review found no critical
  or important issues in chooser input.
- Full Make check/all pass after updating the legacy snapshot-size assertion.
  Previous chooser opening (240 cases) and availability (1,416 cases) also match
  with content14E. CMake and native Windows execution remain unverified here.

## 2026-10-31 — chosen defensive move and stamina decision

- Connected phase75 choice to original profile move cost/effect and wrapped-sign
  stamina decision; phase76 saves accepted/denied/canceled status and full input.
  Cancellation skips the loader, denial emits script61, and stamina is not debited.
  Snapshot54s393bytes appends the decision; original $0445 remains distinct from
  existing action field $0444.
- Original D3F4→D403/D40C comparison matched136 controlled cases:88 accepted,
  32 denied and16 canceled, including complete represented snapshots and warning
  requests. Evidence: special-resolve-final. Original warning scripts ran with
  host frame acknowledgements; redraw, registration and live presentation remain.
- Read-only review found a missing CLI mutual-exclusion guard; fixed and regression
  added. Next: subtype-specific icon data, register the accepted choice and redraw
  while preserving earlier special choices. See NATIVE_DEFENSE_REDRAW_RESEARCH.md.
- Full Make check/all pass after updating snapshot-size assertions; the previous
  2,304 input and240 opening comparisons also pass with snapshot54s.

## 2026-11-01 — defensive choice registration and redraw

- Added phase76→77: accepted choices register the subtype and advance the cursor;
  denied/canceled choices preserve it. Ordered overlay/count/slot-icon/subject
  requests reproduce the original up to D41F, before the saved-input tail.
- Snapshot55t remains393bytes; private content15F adds an action2 subtype1 icon.
  Special subtypes survive back, blink and unchanged direction; changed action
  clears them. Older snapshots and content keep their prior supported scope.
- Original checks matched30 controlled redraws (20 accepted,5 denied,5 canceled)
  and four subtype input cases. Evidence: special-redraw-check-final. Previous
  136 decisions and2,304 chooser inputs match with the new pack/snapshot.
  Full Make check/all pass; independent review found no critical/important issue.
  Explicit host frame acknowledgements; actual rendering and action execution are
  still unfinished. Next: resume D41F's saved-button tail into the next menu state.

## 2026-11-01 — defensive special choice input continuation

- Connected phase77 through the saved-button tail to the next defensive menu wait.
  Accepted/denied nonfinal choices open the next prompt; canceled/full-prefix
  cases process B/directions from the preserved sample, without reprocessing A.
  Special subtype survives unchanged action and clears when action changes.
- Original execution from D3F4 through D41F to D3A6 matched1,104 controlled
  continuations, with240 unsupported full-prefix directional cases rejected.
  Evidence: special-tail-final. Host frame acknowledgements; no actual rendering,
  stamina debit or defense execution claimed. Focused tests and Make check/all pass.
- Next: use the completed defensive choices in the area action and connect the
  resulting possession, shot and match flow. Overall completion remains unmeasured.

## 2026-11-01 — opponent action-choice research

- Traced the completed menu return8B8F→897D. Original route2 enters the opponent
  choice dispatcher before attack preparation, so the next connection must not
  assume a fixed ordinary shot.
- Added a bounded original CPU observer and recorded768 controlled random pairs
  on one captured area fixture:192 action1 and576 action6 outputs. Evidence:
  area-ai-choice-first/observations.json. Numeric codes have no guessed labels.
  No native AI parity, match execution or overall completion increase is claimed.
- Next: original route/category and weighted choice tables, then native opponent
  decision and attack preparation. See NATIVE_AREA_ATTACK_CHOICE.md.

## 2026-11-01 — native opponent action candidate sampling

- Added a native data-supplied sampler for eight packed action candidates, matching
  original bank28:8B0B nibble order and low-three-bit random indexing. Duplicate
  candidates retain their probability; no ROM tables are embedded in source.
- 8,192 original comparisons passed across32 synthetic replacement rows and every
  random byte. Focused native checks passed. Profile/category row selection, later
  action/subtype adjustments and session integration remain unfinished; this is a
  component checkpoint, not a complete native AI or playable match.

## 2026-11-01 — route2 opponent profile/category selection

- Connected direct opponent types and alternate flags to private AI profile rows,
  original folded cell categories and packed action sampling. Generic team types
  and out-of-scope profile/category inputs reject. Private G adds493bytes, with
  the previous F payload verified identical after stripping the addition.
- Original comparisons matched21,632 candidate stages through87B0 (169 types,
  two raw flag states, eight positions, eight random indices). Five unsupported
  inputs and three malformed packs reject. Full Make check/all pass.
- Final original action/subtype adjustment and saved-session integration remain
  next; no complete AI or playable match claim. See NATIVE_AREA_ATTACK_CHOICE.md.

## 2026-11-01 — supported final opponent action branches

- Added original route2 final adjustments for candidates0/1/3; the roster-based
  candidate2 recipient path rejects explicitly until implemented. No fixed-action
  fallback is substituted. Saved-session integration remains pending.
- Original comparison matched18,832 final action/subtype pairs, including mirrored
  positions, and identified2,800 pending recipient cases. Full Make check/all pass.
  Evidence: ai-final-original.log. Next: recipient selection and its move/subtype
  handling, then saved area-action preparation.
- Continued original recipient research with a bounded observer:12,168 cases
  across169 profiles and three teammate position patterns reached5,911 move
  lookups and6,257 fallback returns. Evidence: ai-recipient-first.json. Native
  implementation must preserve rotated random state, recipient ID/cell and move
  subtype; this research does not claim native recipient parity.

## 2026-11-01 — native recipient selection and combined opponent choice

- Implemented preferred/scan/fallback recipient selection, target cell and random
  mutation. Original comparison exposed that a retry does not store its new
  random target; fixed and covered by a focused regression. Native outcomes now
  match15,210 original cases. Private H adds557bytes; prior G payload is identical.
- Combined candidate and recipient branches into a native area action plan.
  5,976 original complete choices match action/subtype/target/cell/random state.
  Generic teams and unsupported raw option groups reject; no fixed substitution.
- Next: add a saved area-choice phase, preserving optional recipient/cell and
  native random state; then original attack preparation. No playable-match claim.

## 2026-11-01 — saved opponent area choice

- Connected completed defensive choices to saved phase78 through native AI plan.
  Snapshot56u396bytes adds target context; target cell is validated against roster
  position. Preserves action/subtype/random state, stamina, clock and prior choices.
- 480 complete original saved-state comparisons match through897D. Evidence:
  ai-session-final. Roundtrip, older formats, replay, target validation and immutable
  input tests pass; full Make check/all pass. No complete match or attack result
  claimed. Next: original attack preparation from this saved choice, with generic
  team resolution and empty-prefix entry still pending.

## 2026-11-01 — opponent area attack numeric preparation

- Added supported opponent area attack preparation using existing profile/skill
  and power routines. Private I adds12bytes for action6 selectors and two moves;
  the previous H payload is identical after stripping the addition.
- Original comparisons through8980 match640 cost/effect/flags/power/subtype/stamina
  cases. Evidence: area-attack-original.log. No opponent stamina debit; this remains
  a prepared calculation before session commit and action presentation/execution.
- Next: connect phase78 to saved prepared attack and preserve its cost, then start
  the original presentation/defender sequence. Overall percentage remains unmeasured.

## 2026-11-01 — saved opponent area attack preparation

- Connected saved choice phase78 to phase79 before original8980 presentation.
  Commits power, adjusted subtype, effect, flags and cost without stamina debit
  or clock change. Snapshot57v398bytes appends cost; old choice inputs load.
- 480 complete original choice snapshots and480 prepared continuations match.
  Evidence: area-attack-session-original.log. Roundtrip, replay and legacy version
  rejection are covered. Next: action presentation and defender sequence.

## 2026-11-01 — prepared action presentation to defender entry

- Added phase79→80 at8998 after prepared script14 and before the defender loop.
  Original comparison exposed a variant difference: variant1 requests motion2;
  variant2 leaves0 after EC3E reset. Traced selector8259 and motion request path,
  corrected implementation and added a regression.
- 480 original choice/preparation/presentation chains match full represented
  state and script request. Evidence: area-presentation-final. Snapshot58w stays
  398bytes; full Make check/all pass. Explicit host frame acknowledgements; no
  renderer or defensive outcome claim. Continuing into the defender sequence.

## 2026-11-01 — prepared defender announcement prefix

- Added phase80→81 at89D5, explicit scripts and frame-wait requests for the saved
  defender order. Action6 skips announcement; other supported area choices2/4/5
  announce and update the opposing player. Final motion request16 is saved; final
  facing is captured explicitly, matching earlier prepared-presentation boundaries.
- 32 original announcement prefixes match complete represented state and ordered
  requests. Evidence: area-announcement-first. Snapshot59x remains398bytes;
  full Make check/all pass. Next: defender skill/energy preparation and comparison.

## 2026-11-01 — area defender skill and stamina calculation

- Added pure preparation for supported human area defenders/actions2/4. Reuses
  profile skill preparation and original energy arithmetic, preserving type32's
  adjusted cost, exhaustion and subtraction carry. Session commit still pending.
- Original CPU comparison matched2,430 numeric results at8A05; seven unsupported
  inputs reject. Evidence: area-defender-original.log. Full Make check/all pass.
  Continuing into area-specific comparison before saved defensive execution.

## 2026-11-01 — area defender contact comparison

- Added separate private area action groups, variation and thresholds; reused the
  numeric roll without substituting ordinary-shot comparison data. J adds36bytes;
  prior I payload verified identical. Incoming power is preserved at this stage.
- 9,720 complete original defender calculations match through8A28, including all
  four result codes and exhausted/special/random boundaries. Four malformed packs
  reject. Full Make check/all pass. Evidence: area-contact-original.log. Continuing
  directly into saved contact execution and result continuation.

## 2026-11-01 — saved area defender contact

- Added phase81→82, committing first defender stamina, identity, effect/cost,
  strength/subtype and result. Attack power and clock remain unchanged. Snapshot60y
  adds14bytes; old x announcements load; duplicate contact attempts reject.
- 1,296 complete original snapshots match, covering all four results; interactive
  capture, atomic failures, bad metadata, legacy reading and full Make check/all
  pass. Prior32 announcement and480 choice/preparation/presentation chains pass.
  Evidence: area-contact-session-original.log, area-announcement-y,
  area-presentation-y. First choices5/6 and later defenders still pending.
- Continuing directly into the original post-contact status/recovery/clock stage,
  including added-time and period-end boundaries. No complete-match claim.

## 2026-11-03 — saved area contact recovery and clock

- Connected phase82 to original post-contact status/recovery/time handling,
  preserving normal83, added-time84 and period-end85 boundaries. Added-time
  acknowledgement changes only phase; stamina is never charged again. Actual
  result presentation, subsequent defenders and period transitions remain pending.
- Snapshot61z stays412bytes, reads y contact snapshots and rejects newer phases
  in old formats. Boundary roundtrip, bad metadata, replay and resume are covered.
- Original comparison:1,296 complete contact saves and156 complete clock/recovery/
  status continuations match, including all four results, extra time, period end,
  elapsed wrapping and keeper recovery. Evidence: area-contact-clock-original.log.
  Clock display uses the original scheduler with explicit host frame acknowledgements
  and frozen randomness; it is not evidence of a native live display/frame loop.
- Full Make check/all pass. Independent review found no blocking issues. Earlier32
  announcements and480 choice/preparation/presentation chains pass with z snapshots
  (area-announcement-z, area-presentation-z). Overall completion remains unmeasured.

## 2026-11-03 — ordered area defenders and result presentation

- Connected post-contact timing to prepared scripts16/17 then18 (phase86),
  preserving terminal facing/motion clearing. Added saved next-defender87,
  prefix-end88 before final timing, and terminal exit89 before possession dispatch.
- Later selected defenders reuse the existing preparation/comparison/clock path.
  Choices5/6 skip calculations without stamina/time debit. Nonterminal advance
  clears the prior result and requests the original one-frame wait; terminal
  results stop immediately. Final sequence timing and possession dispatch remain
  next. Unsupported defensive choices0/1/3 still reject.
- Snapshot62 uses byte128 after ASCIIz, adds one loop cursor at412 (413bytes),
  preserves earlier offsets/readers and rejects downgraded later-slot contacts.
  This bounded version extension avoids moving existing sections. Old cursor0
  y/z contact/clock saves continue to load.
- 720 complete original sequences match: 696 contacts, 748 skips, both variants,
  one/four selected players, all four results and every slot0..3. Includes both
  saved facing values, special/exhausted inputs, terminal early stops, all-skip
  prefixes and interactive capture/replay. Evidence: area-sequence-work/original-final.log.
- Regression evidence:1,296 complete saved contacts,156 clock states,32 prepared
  announcements and480 opponent choice/preparation/presentation chains pass with
  the new format. Full Make check/all and diff check pass. Independent review
  found no blocking issues; added explicit later-slot downgrade rejection coverage.
- Original scripts run with explicit host frame acknowledgements/frozen RNG in
  the observer. This is prepared native state, not evidence of live rendering or
  a complete playable match. Overall completion remains unmeasured.

- Publication verified: engine feature56b1339 is on public work; Arabic daily note
  ae4e35f is on main. GitHub Pages run54 (41599207276) completed successfully for
  that webpage commit. All seven older dates, approved roadmap/design and manual
  date navigation are preserved. Direct live-page HTTP is restricted here; this
  is deployment verification through GitHub, not a browser-render claim.

## 2026-11-03 — final area prefix clock preview

- Added a pure final prefix clock/recovery calculation. It consumes an explicit
  original-derived elapsed value and does not change the input session or phase.
  Saved continuation, source action-table integration and possession remain next.
- 90 original CPU comparisons match normal, added-time and period-end outcomes
  and represented state, including elapsed wrap and keeper recovery. Evidence:
  area-prefix-original.log. Focused native checks cover input immutability and
  invalid phase. Full Make check/all pass; no complete match claim.

## 2026-11-03 — saved final area prefix clock

- Connected exhausted area defense phase88 to saved normal90, added-time91 and
  period-end92 boundaries. An explicit elapsed byte remains necessary; action
  table selection and possession dispatch are not connected yet. Resume changes
  only phase, and a second clock advance rejects.
- Snapshot63 uses byte129, still413 bytes; legacy saves load and cannot encode
  the new phases. Ninety complete original saved states match across supported
  attacks, normal/added-time/period-end, recovery and elapsed wrapping. CLI and
  replay checks plus full Make check/all pass. Evidence: area-prefix-saved-original.log.
  Original frame acknowledgements remain explicit; no native rendering claim.

## 2026-11-03 — automatically selected final prefix timing

- Added original action-table timing to private development pack K; its J payload
  is unchanged. `advance-profiled-area-prefix-clock` chooses the saved action's
  elapsed value and commits the existing clock step. Missing/truncated content
  and repeated advancement reject; old packs remain readable.
- Ninety saved states match the original across all supported attacks and normal,
  added-time and period-end outcomes. Native checks and full Make check/all pass.
  Evidence: area-auto-original.log. Possession dispatch remains next; overall
  port completion is unmeasured.

## 2026-11-03 — area result dispatch movement preview

- Added a pure preview of original C606 movement before area result dispatch.
  Normal results share route0; terminal results2/3 route to recovery or the
  original terminal handler. Input state is unchanged; no movement or possession
  is committed yet. Earlier prefix snapshots do not retain whether the last
  nonterminal comparison was0 or1, which must be considered before later logic.
- 60 original CPU comparisons match the complete field, 20 per route, including
  zero/nonzero elapsed and random boundary inputs. Explicit host frame
  acknowledgements and frozen input randomness remain research assumptions.
  Evidence: area-dispatch-original.log. Full Make check/all pass. Next: saved
  movement boundary and original result/possession handling. Overall progress
  remains unmeasured.

## 2026-11-03 — final area sequence timing and possession

- Connected the final movement step to saved area-result dispatch. Results 0/1
  continue the attack; result 3 with defense action 2 awards selector-2
  experience once and transfers possession directly; result 2 and result 3 with
  action 4 enter selected loose-ball recovery. The recovery wrapper commits the
  selected recipient, preserves selected IDs on the interior recovered-owner
  phase, and clears selected IDs and prior kickoff metadata on the boundary
  restart route. Added `--dispatch-area-result` and `--recover-area-result` CLI
  operations and interactive commands.
- Snapshot version64 uses marker130 and is414bytes. Earlier formats remain
  readable. Version63 exhausted-sequence snapshots do not contain the final
  result, so they load with an unknown-result sentinel and reject dispatch
  rather than infer the result.
- Original ROM verification: all 720 complete defender sequences match,
  covering 696 contacts, 748 skipped entries, all four result codes, and
  selected slots. Separate comparisons match 100 final dispatch states and 60
  selected recovery continuations (original owner handoff or boundary restart).
  `keeper-save-session-tests.exe`, the native CMake build, and all compiled
  `*-tests.exe` executables pass.
- The first 720-sequence rerun exposed a comparison-fixture bug: it dropped the
  saved prior-result byte across skipped choices. The fixture and comparison
  script now preserve that byte; the final 720-sequence rerun passed. No blocker
  remains in the tested sequence, dispatch, or selected recovery paths.
- Pending: live renderer/action presentation, a running host frame loop, and
  runtime randomness are not covered by these comparisons (they use explicit
  frame acknowledgements and frozen randomness). A complete playable match is
  still unverified. Next: trace and connect the gameplay step after these
  possession/recovery boundaries against the original, then continue the
  remaining restart/keeper/story and shot/goal integration. Keep presentation
  and live timing as pending. Overall completion remains unmeasured.

## 2026-11-04 — sampled area recovered-owner keeper-area trace

- Updated the existing `ROADMAP.md` with scoped playable-match checkpoints,
  dependencies and completion checks. Corrected the stale latest dispatch and
  Windows test descriptions in `PORT_STATUS.md`; preserved the older work.
- New original CPU observations: 12 constructed selected-recovery tails reached
  the keeper-area interaction at bank26 `$8B4A`, preserving selected count 1;
  all 12 had zero host-frame waits in this tail. Cases included result 2/action 2
  and result 3/action 4, four random-byte pairs, and two sampled attack powers
  (some generated states repeat). An assertion check confirmed these counts and
  boundaries. Private script and output: `trace-area-recovered-tail-20261104.py`
  and `area-recovered-tail-20261104.json` in the analysis directory.
- The trace uses constructed inputs and cooperative acknowledgements, with no
  NMI, audio, PPU or real frame timing. It did not compare a new native tail;
  ordinary-field, story-gate, other selected counts and broader coverage remain
  untested. The existing virtual environment could not launch its missing Python
  3.12 runtime; Blender's bundled Python with the existing py65 package ran the
  observation and assertion check successfully.
- Previously reported, not rerun today: 720 defender sequences, 100 final
  dispatch states and 60 recovery continuations matched the original; native
  CMake build and compiled C++ tests passed. Next: trace ordinary-field and
  story-gate area recovered-owner continuations before implementing the tail.
  No complete match or overall percentage is claimed.

## 2026-11-04 — sampled ordinary and story area recovery tails

- Starting from an original result 2/action 2 area recovered-owner state with one
  selected defender, constructed two interior-ball variants for the original
  continuation at `$8C31`. The ordinary variant reached player-control handoff
  at `$C60F`; the game-mode 4 story variant reached `$8E86`. Both retained the
  selected count of 1 and required 72 cooperative host-frame acknowledgements.
- The observer exited successfully and an independent assertion check passed for
  both routes, selected counts and acknowledgement counts. Private research
  files: `trace-area-recovered-routes-20261104.py` and
  `area-recovered-routes-20261104.json` in the analysis directory. These are two
  constructed cases, not a native comparison or evidence of live frame timing,
  NMI, audio or rendering. Different owners, sides, selected counts and boundary
  inputs remain untested.
- Updated the existing roadmap and port status. Next: implement one bounded area
  recovered-owner continuation using these original observations, with saved
  state validation and controlled comparisons before extending coverage. The
  complete playable match remains unfinished; overall completion is unmeasured.

## 2026-11-04 — saved area recovery to keeper interaction

- Connected `continue-recovery` from area recovered-owner phase95 to the existing
  keeper-interaction phase7 when the ball is in the original keeper-area gate.
  The selected prefix and the rest of the saved state remain intact. Area recovery
  outside this gate still rejects atomically; ordinary/story tails remain pending.
  A version64 snapshot can retain selected defenders at this phase; older formats
  reject that combination. No new snapshot bytes or version were introduced.
- New original comparison: 12 constructed area keeper continuations matched the
  original represented field and preserved the complete native snapshot apart
  from its phase. The original selected count and IDs were unchanged. Inputs
  covered result 2/action 2 and result 3/action 4, four random-byte pairs and
  two attack-power variants; some constructed states repeat. Evidence is in the
  private analysis folder: `compare-area-keeper-continuation-20261104.py` and
  `area-keeper-continuation-20261104-retry/verification.json`.
- The regression first failed at the unavailable continuation, then passed after
  implementation. Full native CMake build passed; all 45 compiled C++ test
  executables passed. The first ROM comparison failed to launch the session CLI
  because `libunwind.dll` was unavailable in the sandbox; it passed with the
  installed toolchain runtime on PATH outside the sandbox. A CTest attempt could
  not find `ctest.exe`; direct execution of the 45 test executables was used.
- This is a prepared original CPU boundary with frozen randomness, not live
  presentation or a complete match. Next: implement the ordinary-field area
  recovered-owner tail with explicit prepared presentation acknowledgements,
  original comparison and atomic rejection of the unimplemented story route.
  Overall completion remains unmeasured.

## 2026-11-04 — prepared ordinary area recovery continuation

- Added `continue_session_area_ordinary_recovery` and CLI / interactive
  `continue-area-ordinary-recovery FACING` for area recovered-owner phase95.
  It accepts captured facing 0 or 64 after an explicit prepared presentation
  acknowledgement, preserves selected defenders and commits player-control
  phase1. Keeper-area and story-gate inputs reject without changing input;
  those routes retain separate continuations. Snapshot version64 is unchanged.
- Eleven constructed original CPU comparisons matched the represented field,
  retained selected prefix and captured final facing across both possession
  sides, ordinary positions and sampled keeper/story thresholds. Native saved
  output differed from input only in phase and captured facing. Observer waits
  were acknowledged cooperatively; this does not implement live presentation,
  NMI, audio or real timing. Private evidence: `inspect-area-ordinary-tail-20261104.py`,
  `compare-area-ordinary-continuation-20261104.py`, and
  `area-ordinary-continuation-20261104/verification.json`.
- Test-first regression failed to link while the new operation was absent; the
  focused test passed after implementation. The full native CMake build passed;
  all 45 compiled C++ test executables passed. Story rejection and snapshot
  roundtrip are covered. The area story route and broader original coverage
  remain pending. Next: trace and connect the area story-gate continuation with
  selected defenders retained, then compare the represented original state.
  A complete playable match remains unfinished; overall completion is unmeasured.

## 2026-11-04 — prepared area recovery story-gate entry

- Added `continue_session_area_story_recovery` and CLI / interactive
  `continue-area-story-recovery FACING`. From area recovered-owner phase95 it
  checks the original story gate after the keeper-area gate, accepts captured
  facing 0 or 64 and commits story-transition phase8 with selected defenders
  retained. The operation rejects non-story, keeper-area, invalid facing and
  repeated inputs without changing them. Captured kickoff formation is retained.
- Version64 snapshots can retain selected defenders in phase8; earlier versions
  reject that combination. No snapshot bytes or version were added. Native tests
  cover snapshot roundtrip, old-version rejection and kickoff retention.
- Ten constructed original CPU comparisons matched the represented field,
  selected prefix and captured facing for two owners and five positions. The
  native saved snapshot changed only in phase and captured facing. The original
  observer uses cooperative presentation acknowledgements; no live timing,
  rendering, audio or story scene execution is claimed. Evidence remains private:
  `compare-area-story-continuation-20261104.py` and
  `area-story-continuation-20261104/verification.json`.
- The test-first regression failed to link before implementation, then the
  focused test passed. Full native CMake build and all 45 compiled C++ test
  executables passed. Broader original inputs and the story scene remain pending.
  Next: trace the original next boundary for area attack continuation (phase93).
  The complete playable match remains unfinished; overall completion is unmeasured.

## 2026-11-04 — initial area attack result dispatch trace

- Observed original bank26 `$8A6F` through the first result-table destination
  after `$8A72`, starting from a constructed saved area prefix with elapsed0.
  Manually selected attack bytes 1, 4 and 6 and result bytes 0 and 1 produced
  six cases; every case reached `$8A80`, in 23 instructions with no host frame
  waits. An assertion check confirmed all combinations and destinations.
- The action bytes were constructed at the dispatch boundary rather than each
  being generated through a complete action sequence. The observer preserves CPU
  execution across movement and table dispatch; it uses frozen randomness and
  cooperative acknowledgements, with no NMI, audio, PPU or live frame timing.
  This is original-only evidence, not a new native comparison or implementation.
- Private evidence: `inspect-area-attack-tail-20261104.py`,
  `trace-area-attack-dispatch-20261104.py` and
  `area-attack-dispatch-20261104.json` in the analysis directory. Next: trace
  original `$8A80` including its `$8BBA` call and subsequent action dispatch,
  before connecting a native continuation. Overall completion is unmeasured.

## 2026-11-04 — area attack action dispatch trace

- In six constructed original traces, action bytes 1, 4 and 6 with result 0 or
  1 pass through `$8A80`, return from `$8BBA` to `$8A83`, and reach the action
  jump table at `$8A8B`. At the indirect jump, action 1 selected `$C612`, action
  4 selected `$C627`, and action 6 selected `$C62D` for both results. A separate
  assertion check passed for all six combinations and recorded checkpoints.
- The observer counted 162, 59 and 71 cooperative frame acknowledgements for
  actions 1, 4 and 6 respectively in these constructed cases. The actions were
  set at a shared dispatch checkpoint; no complete action chain, native match,
  live rendering, audio, NMI or actual frame timing is verified. A preliminary
  diagnostic recorded later bank26 reentries; those are not the jump targets.
- Private evidence: `trace-area-action-targets-20261104.py`, its underlying
  `trace-area-attack-action-20261104.py`, and
  `area-attack-action-targets-20261104.json` in the analysis folder. No native
  code changed. Next: trace the action 1 `$C612` handler to its next meaningful
  gameplay boundary before deciding how to connect the continuation. Overall
  completion remains unmeasured.

## 2026-11-04 — area action 1 entry trace

- Traced four constructed action 1 entries from `$8A6F`, varying result 0/1 and
  bit 7 of `$0628`, through `$C612` and bank30 `$DE52` to `$DE5E`. At this
  boundary `$061A` was 255 when bit 7 was clear, or 38 when set. A separate
  assertion check confirmed all four cases and the observed banks/values.
- Each constructed run used 162 cooperative host-frame acknowledgements before
  this boundary. Inputs were modified at the shared dispatch checkpoint rather
  than generated by complete action chains. NMI, audio, PPU and live frame
  timing are not emulated; no native comparison or new gameplay code is claimed.
- Private evidence: `inspect-area-action1-entry-20261104.py`,
  `trace-area-action1-entry-20261104.py` and `area-action1-entry-20261104.json`
  in the analysis directory. Next: trace the shared `$E059` call from `$DE5E`
  and identify the next durable gameplay boundary before implementing this path.
  Overall completion remains unmeasured.

## 2026-11-04 — area action 1 shared recipient-cell call

- Traced eight constructed original cases through action 1's `$DE5E` call to
  `$E059` and its return at `$DE61`, with results 0/1 and recipients 12, 13, 21
  or 255. Selected recipients at positions (80,100), (128,124), and (200,150)
  produced cell bytes 50, 125 and 236; recipient 255 preserved the prior cell 77.
  An independent assertion check passed for all eight cases. The call added no
  host waits after entry in these observations.
- Inputs were constructed at a common dispatch checkpoint. The observer uses
  frozen randomness and cooperative presentation acknowledgements; NMI, audio,
  PPU, live timing, full action flow and a native continuation are not covered.
  No native code changed or C++ tests ran for this original-only research task.
- Private evidence: `trace-area-action1-shared-20261104.py`,
  `trace-area-action1-positions-20261104.py` and
  `area-action1-positions-20261104.json`. Next: trace `$DF8B`, called at `$DE61`,
  to identify the next original gameplay boundary. Overall completion remains
  unmeasured.

## 2026-11-04 — area action 1 distance-related call

- Traced eight constructed action 1 cases through `$DF8B` at `$DE61` to its
  return at `$DE64`. For target-cell values 50, 125, 236 and 77, with ball at
  (204,124), the original wrote 5, 4, 3 and 5 respectively to `$062B`, for both
  result bytes 0 and 1. All eight passed an assertion check, retained `$0638`,
  and added no host waits during the call in these observations.
- Static inspection shows a calculation based on target-cell coordinates, ball
  position and a lookup table; these sampled values do not establish exhaustive
  arithmetic or boundary coverage. Inputs are constructed, and the observer uses
  cooperative acknowledgements and frozen randomness. No native comparison,
  live timing, audio, PPU or complete action execution is claimed.
- Private evidence: `inspect-area-action1-df8b-20261104.py`,
  `trace-area-action1-distance-20261104.py` and
  `area-action1-distance-20261104.json`. No native code changed or C++ tests ran.
  Next: trace original `$DE64` through the `$E73E` call to its next durable
  boundary. Overall completion remains unmeasured.

## 2026-11-04 — area action 1 initial flight boundary

- Ten constructed original traces continued from `$DE64` through `$E73E`:
  eight reached flight setup `$E773`, and two already-at-target cases reached
  early return `$E7CF`. All entered with one selected defender and exited with
  zero; elapsed was zero both before and after, and `$061B` was 1. No additional
  host waits occurred inside this setup in these samples. An assertion check
  passed for all ten cases. The early-return cases do not establish velocity.
- These observations use constructed recipients/results, cooperative presentation
  acknowledgements and frozen randomness; NMI, PPU, audio, live timing and full
  action execution are outside scope. No native comparison or C++ test was run.
  Existing native saved flight is a shot path with flag 0; the area action's flag
  1 must be accounted for before reuse is claimed.
- Private evidence: `inspect-area-flight-setup-20261104.py`,
  `trace-area-action1-flight-20261104.py` and
  `area-action1-flight-20261104.json`. Next: compare area flight setup requirements
  with the native saved-flight path, including destination, fractions, flag 1 and
  early return, before implementing any connection. Overall completion remains
  unmeasured.

## 2026-11-04 — area versus saved shot flight assessment

- Inspected the actual saved-flight implementation, session validation and the
  ten existing original area-flight observations. The numeric direction/velocity
  helpers and `$E773`/`$E7CF` boundaries are candidates for reuse, but phase46,
  shot context, flag0 and counter255 assumptions prevent direct reuse for
  phase93 area action 1, whose observed flag is 1 and whose counter can be 38.
- Area recipient/cell context is distinct: the original refreshes the target
  cell from the recipient, and recipient255 is outside native action 1's saved
  12..21 recipient range. Low X/Y fractions are explicit setup inputs; early
  return preserves stale original velocity while shot snapshots normalize unused
  velocity to zero. Area downstream use of those bytes remains unverified.
- Rechecked the existing observation report: 10 cases, flag1, eight `$E773`
  and two `$E7CF` boundaries, with selection and elapsed zero after setup. This
  was a report assertion and source inspection, not a new ROM/native comparison,
  build or C++ test. Added the compatibility details to
  `NATIVE_AREA_ATTACK_CHOICE.md`; no gameplay code changed.
- Next: compare the reusable direction/velocity helpers with original area
  action 1 setup using explicit fractions, then map the saved flag/counter and
  early-return requirements before connecting phase93. Broader match behavior,
  live presentation and overall completion remain unmeasured.

## 2026-11-04 — saved area action 1 flight setup

- Compared reusable native direction/velocity helpers with the original `$E73E`
  setup using flag1: 12 constructed cases matched, covering four target cells
  and three fractional-coordinate pairs. Evidence is private in
  `area-flight-numeric-20261104.json` and its private comparison helper.
- Connected phase93 action1 to area-only phase96 (`$E773`) or phase97 (`$E7CF`).
  Preserves the area recipient/cost, destination, 255/38 counter, flag1 and
  explicitly supplied post-presentation facing/motion/distance/fractions. Clears
  selected count and elapsed. Early return preserves explicitly supplied prior
  velocity. Existing shot movement rejects these phases pending area-specific
  original callback research. Added batch and interactive area-flight commands.
- Version65 marker131, with the existing 414-byte layout, is written only for
  phases96/97; other phases still use version64. Tests cover normal/early
  roundtrip, old-version rejection, stale velocity, special counter, invalid
  input, replay rejection and input immutability.
- Twenty-four constructed original/native saved setups match represented field,
  trajectory, recipient, destination, counter and captured presentation values.
  Coverage is results0/1, two counter variants, three fraction pairs and
  normal/early paths, with one selected defender, recipient13 and elapsed0.
  Interactive replay matches the batch snapshot. Evidence remains private in
  `area-flight-session-20261104-final2/verification.json` and its comparison
  helper. The first interactive replay attempt failed because its test fixture
  quoted a save path incorrectly; the corrected rerun passed.
- Full native CMake build and all 45 compiled C++ test executables passed.
  Cooperative acknowledgements and frozen randomness are not live presentation;
  original unused selected-ID bytes after count0 were not compared. Original
  area movement/callback behavior, moving-recipient cases, NMI, rendering, audio
  and real timing remain pending. Next: trace and compare the first area flight
  step and its flag1 callbacks before connecting further saved phases. Overall
  completion remains unmeasured.


## 2026-11-04 ? area flight, action outcomes and control boundaries

- Connected action1 flag1 flight steps and the cell callback to saved phases.
  Twelve pure numeric and 12 saved first-step comparisons matched the original;
  60 constructed cell-selection comparisons matched selected prefix and field
  state, including random thresholds, stunned players and replay rejection.
- Connected already-at-target and zero-defender normal arrival to recipient
  transfer and keeper entry/preparation. Twelve early-arrival, 12 normal-arrival,
  12 recipient-to-keeper, and 12 keeper-setup original comparisons passed at
  their represented boundaries. Defender-queued arrival remains unsupported;
  keeper menu/polling and presentation are not connected.
- Retained choices across phase93 for actions4/6 using marker131/version65.
  Eight constructed action4 boundaries matched filtering/preserved prefixes and
  field state. Action4 encounters and the resulting control path are pending.
- Action6 destination and race paths are connected: 16 cases at each of two
  vertical halves matched destination/field and experience, six races matched
  field/candidates/time, four winner and two timeout-recovery control handoffs
  matched, and six prepared ordinary-control setups matched represented state.
  Keeper-area and story recovery, control input and restart routes remain open.
- Found that prior action1/4/6 continuation comparisons omitted experience:
  the original awards carrier selector1 when the selected count is nonzero.
  Added this award atomically before each action branch, preserving replay and
  missing-content rejection. Re-ran 24 action1 setups (including interactive
  replay), eight action4 boundaries and 32 action6 destinations with experience
  bytes included: all matched. The six control-setup comparisons also include
  represented experience; the previous attempt failed on the missing award.
- Windows CMake build passed, focused keeper-save-session checks passed, and
  all 45 compiled C++ test executables passed (zero failed). A sandboxed CTest
  invocation could not access the toolchain; running each executable with the
  compiler runtime available outside the sandbox completed successfully. Four
  initial sandboxed comparison runs likewise could not launch native children;
  the outside-sandbox reruns above passed. No claim is made about live frames,
  random scheduling, rendering, audio or a complete playable match.
- Next: trace and compare defended arrival handling, then implement its saved
  boundary; continue the action4 encounters and action6 keeper/story transitions
  only after original observations. Preserve existing incomplete work. Overall
  completion remains unmeasured.
- A subsequent focused test also verifies that zero selected defenders skips the
  award without content. Its first fixture failed validation because it retained
  the old selected cursors; after correcting both cursors, the rebuilt focused
  executable passed. The 45-executable suite above ran before this test-only
  fixture addition; no production code changed afterward. `git diff --check`
  passed before the final test-only addition.


## 2026-11-04 ? defended area pass entry

- Traced normal defended arrival from original `$8835` to `$8840`: a nonzero
  selected count resets `$0616` to zero before the first defender presentation.
  Later code temporarily pushes `$044E`, clears it, selects a defender and
  enters contact; that later behavior was inspected but is not implemented.
- `continue-area-already-arrived` now saves phase114 for a normal arrival with
  queued defenders, preserving possession, flight and selected IDs. Empty arrivals
  still transfer to the recipient. Already-at-target snapshots with queued
  defenders reject, as do replay and invalid phase inputs. New phase114 requires
  a selected defender and uses existing marker131 snapshots.
- Twelve constructed original comparisons covered counts 1, 2 and 4, both
  prior sequence results and two counter variants. Original owner, selected
  count/ordered IDs and cursor matched; all other native snapshot bytes were
  checked for preservation, not compared against the original. The private
  report is `area-defended-arrival-entry-20261104/verification.json`.
- Test-first focused check failed as expected before the implementation because
  defended arrivals were rejected; it passed after implementation, including
  snapshot roundtrip, replay rejection and unchanged input. The Windows CMake
  build passed and all 45 compiled C++ test executables passed. Initial full
  build had an enum-display warning; the switch was updated and rebuilt cleanly.
- Still pending: presentation, contact and all outcome branches for queued
  defenders, followed by possession/keeper flow. The original temporary stack
  value must survive any future saved boundary. Frame, RNG, audio and rendering
  remain unverified; complete playable match and overall percentage remain
  unmeasured. Next: trace the first defender preparation/contact and its saved
  state requirements, then compare a narrowly scoped implementation.


## 2026-11-05 ? first queued area-pass defender announcement

- Traced original `$8840` through the announcement return at `$8845`; in six
  exploratory cases (selected counts 1/2/4, ball variants 1/2), each used one
  cooperatively acknowledged frame, preserved the variant and had the original
  stack restored. The next instructions push `$044E` and clear it.
- Added phase115 and batch/interactive `ack-area-pass-defender` to save that
  post-announcement, pre-push boundary. The source snapshot is untouched; the
  ball variant, possession, ordered selection, trajectory and cursor remain
  available for the later contact. Replay is rejected and marker131 snapshots
  roundtrip. This acknowledges presentation rather than executing it.
- Twelve constructed original/native comparisons covered selected counts 1, 2,
  4, two prior sequence results and two ball variants. Owner, selected prefix,
  cursor and variant matched. Represented field and experience remained
  unchanged independently in each implementation; other native snapshot bytes
  were checked for preservation, not compared directly against the original.
  Evidence: private `area-defended-announcement-20261104/verification.json`.
- The test-first focused session test failed as intended on the unimplemented
  operation, then passed after implementation, including roundtrip and replay.
  Before that expected failure, an initial test build failed due to duplicate
  local test variable names; renaming the fixture variables fixed the build.
  The full Windows CMake build passed; all 45 compiled C++ test executables
  passed (zero failed). The original comparison passed 12/12 after resuming.
- Still pending: selecting/preparing the first defender, contact comparison,
  restoring the temporary flag and processing results. No live frame timing,
  audio or rendering was verified; a complete playable match remains unfinished
  and overall completion is unmeasured. Next: trace defender preparation and
  preserve the temporary flag across the next saved boundary.


## 2026-11-05 ? first queued pass defender preparation and contact

- Traced the original after `$8845`: it pushes `$044E` on the 6502 stack,
  clears it, selects the first queued defender, sets defense action2/subtype0,
  and calls presentation script7 before reaching `$886D`. The pass contact
  then uses `$888D`/`$8EE9` and `$8132`, not the earlier keeper-area contact
  entry. The caller restores the stacked variant after the result; that
  restoration and the outcome handler are not implemented in this increment.
- Added saved phase116 with selected defender, cleared active ball variant,
  explicit captured original presentation cost, and a context preserving the
  stacked variant. Phase117 saves comparison power, subtype, randomized
  strength, loss, score and result, without running the result handler.
  Defender scratch power (`$32/$33`) and context (`$0621`) are explicit
  captured inputs. Private comparison table values are exported from the
  verified original by `tools/export_area_pass_comparison.py` to `CMJPASS1`.
  Existing saved states retain their previous formats; the new phases use
  marker132/version66 with a three-byte appended context. Roundtrip, old
  version rejection, truncation, replay and atomic invalid-input checks passed.
- Twelve constructed preparation comparisons covered selected counts 1/2/4,
  two prior results and both ball variants; selected defender, owner, ordered
  prefix, saved stack value and captured cost matched. Unchanged native
  snapshot bytes were checked for preservation.
- Contact comparisons: 96 constructed cases varied both ball variants, four
  attack powers, three captured defender powers and four random-byte pairs;
  two threshold sweeps of 65 and 191 cases varied attack power with one
  variant/defender/random setting. All 352 matched original defender strength,
  subtype, loss, score, result and preserved variant; all four result codes
  appeared across the runs. These are controlled instruction observations with
  supplied scratch inputs, not an end-to-end live match or full roster survey.
  Private reports are `area-defended-preparation-20261105/verification.json`,
  `area-pass-contact-comparison-20261105/verification.json`,
  `area-pass-contact-thresholds-20261105/verification.json`, and
  `area-pass-contact-thresholds-20261105-v2/verification.json`. Exported
  comparison content matched the private table used in the tests byte for byte.
- Both test-first additions failed on their unimplemented stubs, then passed.
  Full Windows CMake build and all 45 compiled C++ test executables passed;
  `git diff --check` passed before this documentation update.
- Next: trace and implement result handling and restoration of the temporarily
  saved variant. Repeated defenders, automatic production of captured scratch
  inputs, live frame/presentation/audio and full-match play remain unverified.
  Overall completion remains unmeasured.

## 2026-11-05 - defended-pass outcomes and prepared return to control

- Added saved restoration of the temporarily stacked ball variant, defender
  outcome acknowledgements, nonterminal clock/reward handling, subsequent
  defender selection and recipient transfer. Terminal outcomes now connect
  sampled blocked-ball and defender-capture paths to prepared control and an
  ordinary field round. Captured control can enter the ordinary action menu and
  prepare a basic shot. Presentation values remain supplied or acknowledged;
  this does not run the original presentation engine.
- Constructed original/native comparisons: 96 variant restorations (results
  0/1/3), 8 handler entries (all four results), 4 normal nonterminal clocks,
  4 added-time entries, 4 period ends and 4 nonterminal added-time resumes;
  20 next-defender cases and 8 recipient transfers; 2 blocked repositionings,
  4 terminal clocks and 4 terminal added-time entries/resumes; 2 each recovery
  entry, recovered owner, recovered control and blocked control setup; 2 each
  capture, capture continuation and capture control setup. These are narrow
  constructed samples, not exhaustive or live-game coverage.
- For each of capture and blocked recovery, 12 first control polls, 12 timer
  expiry polls, 12 ordinary clock cases and 12 ordinary field rounds matched the
  represented original state. Capture also matched 2 each goal-area, added-time
  and period-end clock entries without kickoff metadata, 6 field-round menu
  interruptions (two ball variants and three positions), 2 direct menu openings
  and 2 ordinary shot preparations. The original retains flight flag $061B at
  menu entry; native clears historical flight context, so its behavior through
  subsequent flight must be checked before claiming an end-to-end match.
- Rechecked the 352 prior defender-contact samples after synchronizing original
  scratch inputs: 96 plus threshold sweeps of 65 and 191 matched, covering all
  result codes. Fixed the native captured calculation context ($0621): earlier
  comparisons omitted this field and it had incorrectly remained 2. A focused
  regression failed before the correction and passed afterward. Some comparison
  fixtures initially failed because scratch inputs or keeper profile types were
  inconsistent; corrected fixtures produced the results above. A field-menu
  report initially raised a KeyError after assertions; the corrected run
  completed. Private evidence remains in ../Captain-Majid-analysis.
- Fresh Windows CMake build: exit 0 (no work to do). All 45 compiled C++ test
  executables: 45 passed, 0 failed. `git diff --check` passed before this entry.
  Historical runs should not be confused with these fresh checks.
- Not yet verified: blocked-recovery menu and shot follow-up, boundary recovery,
  blocked-control goal/added-time/period branches, terminal period-end against
  the original, kickoff-metadata goal/clock paths, and live frame/random/audio
  scheduling. Active-control added time is rejected: the synthetic original
  observer exited to $0001 and needs further tracing. Field-contact, special and
  story interruptions are rejected atomically. Action 4 encounters, action 6
  keeper/story paths and a complete playable match remain unfinished. Next:
  compare blocked menu entry and trace new-action flight flags, then continue
  only with behavior supported by original observations. Overall completion is
  unmeasured.

### Follow-up: menu and new shot after defended pass

- Two constructed blocked-recovery cases (ball variants 1 and 2) matched the
  represented original fields through action-menu opening, ordinary shot choice
  and basic shot preparation. The first sandboxed comparison could not launch
  its native subprocess; the outside-sandbox rerun passed. Evidence:
  private `area-pass-blocked-menu-shot-20261105-v2/verification.json`.
- Two capture cases matched represented field/control bytes through shot clock,
  result dispatch and shot destination. An initial attempt used an exporter
  that does not accept these phases; its corrected comparison passed. Original
  $061B remained 1 at each of these boundaries.
- Extending that trace to new shot flight setup found a genuine mismatch in
  the first tested variant: represented field bytes matched, but original
  trajectory flag $061B was 1 and native trajectory flag was 0. Native menu
  entry discards the historical flight context and its shot setup initializes
  the flag to zero. The second variant was not reached after the assertion.
  The effect on later callbacks remains untraced; do not claim the new shot
  flight is faithful. Private failed evidence:
  `area-pass-capture-shot-followup-20261105-v3`. Next trace the flag's uses and
  design a backward-compatible saved-state representation before connecting
  further behavior. No native fix was made for this mismatch.
- A follow-up intentionally excluded the known differing flag from comparison
  and continued both variants through setup and the first numeric flight cell.
  Both matched represented field and the other trajectory bytes; original flag
  remained 1 and native 0. This does not verify the subsequent callback. Private
  evidence: `area-pass-capture-shot-followup-20261105-v4/verification.json`.

## 2026-11-05 - retain defended-pass flight flag

- Found the prior pass context was discarded at action-menu opening while the
  original $061B flag remained 1. Retain that flag in phase state and marker133
  snapshots, then use it for a subsequent prepared shot. Existing snapshots
  still read with zero as their historical default. Added validation of invalid,
  truncated and downgraded retained-flag snapshots.
- Two constructed capture variants match original represented field/control and
  trajectory, including flag 1, through shot preparation, ten flight cells,
  callbacks and empty goalkeeper entry. Two blocked-recovery variants match
  menu/basic-shot fields while retaining flag 1. The samples use cooperative
  presentation and frozen inputs, not live frames. Private reports:
  `area-pass-shot-caller-compare-20261105-v2/verification.json` and
  `area-pass-blocked-menu-flag-fixed-20261105/verification.json`.
- Original empty arrival callback with flag 0 versus 1 matched represented field
  and processor status in two constructed variants; real shot return reached
  $DD1C. Retained-flag loose arrival rejects atomically until compared. Defender
  arrival, broader menus, additional keeper paths and live match remain unverified.
- Continued both capture variants through keeper setup, a normal keeper decision
  and goal/scorer reward, then acknowledged post-goal cleanup to kickoff pending;
  represented fields, score and retained flag matched. A separate zero-shot-power
  fixture reached result3 and keeper possession in both variants, matching the
  represented fields and flag. These keeper decisions explicitly changed the
  keeper to ordinary type1 in both original and native fixtures; the first attempt
  with generic type0 failed because the private pack lacked its team profile.
  Presentation/menu steps remain prepared acknowledgements. Private reports:
  `area-pass-shot-restart-compare-20261105/verification.json` and
  `area-pass-shot-save-compare-20261105/verification.json`.
- Test first: the menu retention check failed before implementation; a separate
  keeper-entry regression failed while the temporary arrival guard was in place,
  then passed after the original trace justified the narrow path. A separate
  guard regression failed before the guard was added. Initial comparison script
  creation had a literal newline escape syntax error, fixed in the v2 report.
  An unprivileged build could not access the installed toolchain; the approved
  outside-sandbox build succeeded. The full Windows build passed; all 45 compiled
  test executables passed after flag preservation and initial guard. After the
  narrow keeper-entry change, the focused ball-flight test and a fresh full run
  of all 45 compiled test executables passed (0 failures).
- Next: compare retained-flag loose/defended arrivals, then trace remaining
  defender, keeper, restart and clock routes toward a playable match. Overall
  project completion is unmeasured.

## 2026-11-05 - retained-flag empty loose recovery

- Original tracing showed two sampled keeper catches enter the shared pass code
  at $DE5E/$DE61/$DE64/$DE69, then transfer to player16 at $DE73 and reach
  $E0DF. The observer acknowledged 145 host waits; this is research, not a
  native keeper-distribution implementation. An attempted trace stopping at
  $DE52 failed because this path enters at $DE5E. One original entry fixture
  had out-of-field ball coordinates and needs further investigation before reuse.
- Compared two constructed flag1, empty loose-shot flights from phase46 through
  all eleven sampled numeric cells and the original $DD14 return. The fixture
  explicitly sets side0 scratch context, ball position/destination and opposing
  players; it is not an uninterrupted match. Both native results match original
  represented field/control and trajectory. Original recovery and native
  `recover-shot-loose` then matched represented field/control and retained flag
  in both cases. Private reports: `area-pass-shot-loose-compare-20261105-v4`
  and `area-pass-shot-loose-recovery-20261105`.
- Removed the temporary retained-flag loose-arrival rejection, while preserving
  the existing rejection for queued defenders. Fixed phase validation so saved
  kickoff formation survives already-arrived and loose-recovery boundaries.
  Original comparison first failed the temporary flag guard and then exposed the
  formation validation failure; focused regression tests reproduced both before
  the fixes and pass afterward. An earlier synthetic trace used mismatched
  original scratch context and correctly returned the keeper path; the corrected
  fixture is documented above.
- Fresh Windows build and focused `ball-flight-session-tests.exe` passed. Full
  45-executable suite and `git diff --check` are pending after this increment.
  Defended arrivals, keeper distribution, live scheduling/presentation and a
  complete playable match remain unfinished. Overall progress is unmeasured.

## 2026-11-05 - retained-flag defended shot entry

- A constructed side0 shot with retained flag1 can reach original $8835 with
  up to five selected defenders, then resets cursor $0616 and enters $8840.
  Added phase150 to save that ordered prefix, phase151 for an explicitly
  acknowledged defender announcement, and phase152 for the first prepared
  defender. The shot path can have stacked ball variant zero; marker133 now
  preserves that valid zero-valued context and captured preparation cost.
- Six constructed original/native comparisons (one, three or five selected
  defenders, each with two earlier ball variants) matched represented fields,
  ordered selection, cursor and saved state at entry, announcement and first
  preparation. The first selected defender was 13 and captured cost was 50 in
  these fixtures. Announcement used one cooperatively acknowledged wait. The
  tests do not establish live presentation or broader roster coverage. Reports:
  `area-pass-shot-defended-compare-20261105-n{1,3,5}`,
  `area-pass-shot-announcement-compare-20261105-n{1,3,5}` and
  `shot-pass-defender-prep-compare-20261105-n{1,3,5}` under private analysis.
- A synthetic contact probe with shot action0, flag1 and nonzero frozen random
  values did not return within 200000 instructions: the original repeatedly
  subtracted a zero table value at $8D4C-$8D54. With random bytes 0/0 the two
  sampled variants did return result0. This is a constructed, cooperatively
  acknowledged state, not evidence that normal live gameplay hangs; live
  presentation/NMI may produce different state. The first probe lacked an
  informative timeout; a later one recorded the zero table value. Do not reuse
  the action1 comparison table or implement this behavior without tracing valid
  original input production and the action0 lookup. Contact and outcomes remain
  pending, not silently simulated.
- Regression tests first failed on unimplemented entry, announcement and
  preparation; they passed after each addition. The fresh Windows CMake build
  succeeded and all 45 compiled test executables passed, zero failures. New
  progress remains local and uncommitted; GitHub still contains the prior backup.
  Overall completion is unmeasured; a complete playable match remains unfinished.
- Next investigate the action0 zero-range lookup and valid random/presentation
  state, then compare first contact before processing its result or later
  defenders. Keeper distribution and other match routes still need tracing.

## 2026-11-05 - guarded shot-pass contacts and clock

- Exporter now accepts `--attack-action 0` to export the original action0 table;
  default action1 output was verified byte-for-byte against the earlier private
  table. The action0 entry sampled has zero range, offset and eligibility.
  For context0 and random-byte sums 0 or 1, original strength/loss were 0,
  score255 and result0 in 120 controlled original/native comparisons: two
  earlier ball variants, five shot powers, four defender powers and three random
  pairs. Native explicitly rejects other ranges/offsets, eligible specials,
  contexts and random inputs. A rejected input leaves the snapshot unchanged.
- Added saved comparison, stacked-zero restoration, acknowledged result display,
  ordinary clock/status/experience handling, next-defender selection and
  completed-prefix return. Six constructed original comparisons matched each
  initial handler/clock/next boundary for counts 1/3/5 and both earlier ball
  variants. Two further full five-defender sequences matched represented fields,
  experience, clock, cursor and retained flag through loose-arrival return;
  original stale IDs were normalized after the count cleared. Presentation uses
  explicit cooperative acknowledgement, not live timing.
- Four constructed comparisons each matched added-time entry, period-end entry
  and added-time resume (counts 1 and 5, two variants). Active-control added-time
  resume remains explicitly rejected. The full-match period transition is not
  implemented. Initial handler comparison scripts accidentally reused output
  filenames twice; non-destructive v3 reruns passed. A PowerShell hash-comparison
  syntax attempt failed before a corrected hash check passed.
- The Windows CMake build passed and all 45 compiled C++ test executables passed
  after the latest engine changes; `git diff --check` passed before these notes
  with line-ending warnings. Work remains local and uncommitted. Earlier notes
  about loose-arrival rejection and missing contact are historical; the scoped
  paths above now exist. Other random conditions, other defender outcomes,
  action4/6 routes, keeper distribution, live frame/RNG/audio/presentation and
  complete playable match remain unfinished. Overall completion is unmeasured.

## 2026-11-05 - action4 field entry and no-encounter control

- Added marker134 saved action4 phases for the original $8044 field-encounter
  setup and for the no-encounter $E0DF control entry, prepared control, input
  wait, control clock, field round and goal/added-time/period boundaries.
  Preserved filtered defender choices and original action context. No-encounter
  added time can resume to an ordinary field round without repeating the clock.
  AI-cell, field-contact, story and other untraced interruptions reject atomically.
- Six constructed original/native comparisons matched the field-entry boundary
  after $803C clears the comparison route; counts 1/2/4 and prior results0/1
  were covered. Two no-encounter cases matched original control preparation.
  Twelve cases each matched first input polls, timer-expiry polls, ordinary
  clock and zero-input field rounds. Twelve cases each also matched goal-area,
  added-time and period-end entries, plus twelve added-time resumes. Cases use
  two prior results and six input masks; they are constructed, with cooperative
  frame acknowledgements and no live presentation or RNG scheduling.
- Clock comparisons supplied a valid type1 keeper profile in both original and
  native fixtures. Goal/added/period comparisons explicitly omitted native
  kickoff-formation metadata because generic control rejects those interruptions
  with it; the first attempt exposed that restriction and was not counted as a
  pass. The unit fixture was similarly corrected. Initial field-setup tracing
  exceeded its instruction bound at $CB21 after 3407 cooperative waits; no
  encounter continuation was inferred. Further trace must determine the needed
  presentation/input behavior. Private reports: `area-action4-field-entry-compare-
  20261105`, `area-action4-no-encounter-control-compare-20261105`,
  `area-action4-control-poll-compare-20261105`, `area-action4-control-expiry-
  compare-20261105`, `area-action4-control-clock-compare-20261105`,
  `area-action4-field-round-compare-20261105`, `area-action4-{goal,added,period}-
  compare-20261105-v2`, and `area-action4-added-resume-compare-20261105`.
- Test-first checks failed on the unimplemented entries and clock routes, then
  passed. Windows CMake build succeeded; all 45 compiled C++ tests passed with
  zero failures. `git diff --check` passed with line-ending warnings before this
  note. The code remains local and uncommitted. Complete match progress remains
  unmeasured; encounter processing, kickoff-metadata interruptions, goal/period
  continuation and live frame/audio/presentation remain unfinished.

## 2026-11-05 - action4 goalkeeper preparation

- Added marker134 phases169-171 and CLI operations for the sampled action4
  no-encounter goal-area route: select keeper0 and route3, acknowledge captured
  presentation facing/motion, and save observed action6/subtype0 with explicitly
  captured 16-bit cost and power. The keeper-preparation snapshot roundtrips;
  repeated operations reject without changing their input.
- Twelve constructed original/native samples matched each entry, presentation
  and preparation boundary (two prior results and six input masks). The selected
  synthetic keeper profile was type1; the observed cost was 50 and power 8.
  Presentation used 117 cooperative host waits per case; preparation used zero.
  These observations are not live rendering, scheduling or broad roster coverage.
  Private outputs: `area-action4-keeper-prep-compare-20261105`.
- An original-only trace reached the next stamina boundary in the same twelve
  constructed cases: represented field state was unchanged and scratch $3A
  changed from 0 to 128. This covers an exhausted keeper only; stamina handling
  and keeper comparison/outcome are not yet implemented for the action4 route.
  Private output: `area-action4-keeper-stamina-trace-20261105`.
- Verification run after these changes: Windows CMake build passed; focused
  `keeper-save-session-tests.exe` passed; all 45 compiled `*-tests.exe` passed
  (0 failed). The new preparation test failed against the initial stub before
  implementation. Work remains local and uncommitted; private traces stay outside
  Git. Next compare sufficient, exact and insufficient keeper energy against the
  original before implementing stamina, then trace the keeper outcome. Encounter
  execution, other interruptions, live presentation and a complete match remain
  unfinished. Overall completion is unmeasured.

## 2026-11-05 - action4 keeper stamina thresholds, original-only

- Traced 36 constructed original cases at the existing action4 keeper preparation boundary: two prior results, six input masks, and stamina 49, 50 or 51 against captured cost 50 and power 8. Respectively the original produced stamina 0, 0 and 1; scratch $003A was 128 only for the insufficient case. These are controlled type1 keeper cases with cooperative presentation, not a live match. Private scripts, snapshots and result summary: `area-action4-keeper-thresholds-20261105` in the analysis directory; scripts retain their temporary output paths and must be redirected to fresh directories before rerun.
- This is original-only evidence. No native stamina continuation, native comparison, outcome trace, focused test, or build was completed in this increment. Next implement and compare the saved stamina boundary, including atomic rejection and snapshot compatibility, then trace the keeper outcome. Overall completion remains unmeasured.

## 2026-11-05 - action4 keeper stamina saved boundary

- Added saved phase172 and CLI `--spend-area-action4-keeper-stamina`. For the observed type1 keeper, cost50/power8 and stamina 0/49/50/51 only, it applies the existing original-byte energy calculation, saves the weakened flag, and leaves its input unchanged. Other inputs and repeated calls reject atomically. Marker134 snapshots roundtrip; existing snapshot readers remain in place. Broader keeper profiles, costs and stamina values remain unverified and explicitly rejected.
- The focused test failed against a placeholder before implementation, then passed; it checks 49/50/51, snapshot roundtrips, repeated calls and unsupported stamina rejection. Thirty-six constructed original/native snapshot comparisons passed, matching the original observed stamina and weakened flag, while checking all other snapshot bytes unchanged. These are controlled cases, not live-match parity.
- Original-only probes continued those 36 constructed states to bank26 $8771, immediately after the $8148 call: A=255, Y=0 and scratch $003A=255 in each. Scratch $0032 was 0 for stamina49 and 8 for stamina50/51. This is a CPU boundary; its final gameplay outcome and broader inputs remain unknown and were not implemented. Private scripts, snapshots and summaries are in `area-action4-keeper-native-20261105`; scripts contain temporary output paths and need fresh output directories before rerun.
- Windows `cmake --build build -j 2` passed. All 45 compiled `*-tests.exe` passed with zero failures. `ctest` was attempted but unavailable on PATH; direct executables were used instead. `git diff --check` passed with line-ending warnings before this log entry. No Linux Make checks or complete match were run. Next trace the original comparison inputs, random production and branches after $8771, then compare and implement only supported outcome boundaries. Overall completion remains unmeasured.

## 2026-11-05 - action4 goalkeeper comparison, bounded outcomes

- Traced 144 constructed original cases across stamina 49/50/51, two prior results, six input masks and four explicitly frozen random pairs. Original keeper comparison reads captured attacking power22, scratch action0/subtype128 and profile adjustment0; the saved action4 context does not itself represent all those scratch inputs. Its observed range/offset were 218/37 and thresholds 168/90/82/0. Original random production and live presentation were not run.
- Added saved phase173 and `--compare-area-action4-keeper POWER ACTION SUBTYPE ADJUSTMENT`. The caller supplies the otherwise unrepresented captured inputs; only power22/action0/subtype128/adjustment0 and random pairs 127/128 or 255/255 are accepted. Existing deterministic comparison calculates strength, loss, score and subtype; the saved boundary records result0/1. Other inputs reject without mutation. Original prepared keeper power remains saved, even when weakened original scratch power is shifted.
- Seventy-two constructed original/native full snapshot comparisons passed: 48 result0 and 24 result1. Original field and results were compared for each; strength/loss/subtype values were checked against separate focused scratch traces for the sampled categories. Snapshot roundtrip and atomic rejection checks passed. Test first failed against a placeholder, then passed after implementation. Marker134 retains the new phase and earlier readers remain supported.
- The remaining 72 original-only cases with random 0/0 and 1/1 produced result4 and changed the represented opposing keeper penalty from79 to0 after $8E33/$C55A. That behavior is not implemented; do not infer it from result code alone. A dispatch probe with cooperative waits hit its 500000-instruction bound at $CD4C after 869 waits; final gameplay continuation remains unknown. Private traces, scripts and native comparisons: `area-action4-keeper-comparison-20261105` in the analysis directory; scripts retain temporary paths and need fresh output directories before rerunning.
- Fresh Windows `cmake --build build -j 2` passed; all 45 compiled `*-tests.exe` passed with zero failures. Linux Make, live randomness/presentation and a complete match were not tested. Overall completion remains unmeasured. Next trace the result4 penalty/animation effects and result0/1 dispatch using valid presentation timing, then implement only observed boundaries.

## 2026-11-05 - sampled action4 goalkeeper override

- Original-only controlled traces varied the attacking side keeper penalty across 0, 1, 25, 79, 80, 127 and 255. With selected keeper0, owner12, captured attack power22/action0/subtype128/adjustment0 and random0/0, all seven reached result4 and cleared the attacking side keeper penalty to0; other represented field bytes remained unchanged. These are constructed states, not live match observations.
- Extended saved phase173 to support the observed result4 path for random0/0 or1/1, owner12 and the seven sampled penalty values. Other new cases reject atomically. The comparison preserves prepared keeper power, calculates strength/loss/score, records result4, and clears that sampled penalty. Focused test failed before implementation and passed after it; snapshot roundtrips included.
- Seventy-two original/native full snapshot comparisons matched result4 for stamina49/50/51, two prior results and six input masks at the two random pairs with penalty79. Seven additional original/native comparisons covered the penalty values above at stamina50/random0; separate original probes established those outputs. Windows `cmake --build build -j 2` passed, and all 45 compiled `*-tests.exe` passed. No Linux or live match checks were run.
- A further presentation-dispatch diagnostic hit 500000 instructions after 869 cooperative waits at $CD4C, with result byte2; it did not establish the dispatch outcome or cause of non-return. Private scripts and comparisons are in `area-action4-keeper-override-20261105` in the analysis directory. Overall completion remains unmeasured; next investigate presentation scheduling and result dispatch, then continue match routes towards a full playable match.

## 2026-11-05 - action4 goalkeeper result dispatch entry

- Corrected the earlier non-return diagnosis: result4 dispatch enters $87D4, which was absent from the earlier probe's stop set; that probe continued into later gameplay and is not evidence of a presentation hang. Constructed original traces reached result0 $8789 after 182 cooperative waits (48 cases), result1 $879F after 246 waits (24 cases), and result4 $87D4 after 172 waits (72 cases). All 144 preserved represented field and sampled scratch values at the entry. Explicit cooperative waits are not live presentation.
- Added saved phase174 and CLI `--ack-area-action4-keeper-result` for an explicit acknowledgment of the result presentation before its handler. It preserves the saved outcome and snapshot, rejects repeated calls atomically, and does not simulate presentation. The regression failed against a placeholder, then passed; 144 original/native full snapshot comparisons matched, including results0/1/4. Original traces and private reports are in `area-action4-keeper-dispatch-20261105` under the analysis directory.
- Preliminary original-only single-case handler probes reached result0 $88F3 and result1 $8BDF, both changing the represented keeper0 stun from0 to6, and result4 $9366 unchanged. Those later paths are not implemented or broadly validated; continue tracing them before native changes. Windows CMake full build and all 45 compiled tests passed. Linux Make, live presentation/randomness and complete match remain unverified; overall progress remains unmeasured.

## 2026-11-05 - action4 goalkeeper result0/1 handlers

- Original controlled handler traces covered 72 cases at stamina49/50/51 and random127/128 or255/255, with two earlier results and six input masks. Result0 reached $88F3 in 48 cases; result1 reached $8BDF in 24. In all cases original awarded the carrier selector1 experience (sampled total3 to6) and set keeper0 stun0 to6; other represented field and sampled scratch bytes were unchanged. Handler execution used zero further cooperative waits after the saved result entry.
- Added phases175/176 and `--continue-area-action4-keeper-result ACTION`, requiring captured original action0, owner12, observed random inputs, saved result0/1 and experience data. The handler awards experience once, sets keeper0 stun6 and saves the corresponding continuation; result4 and untraced inputs reject atomically. Original action0 is a separate captured scratch input from the retained action4 session. Regression failed against a placeholder before implementation and passed afterward, including snapshots and rejection.
- Seventy-two original/native full snapshot comparisons matched original represented field and experience bytes. Windows full CMake build and all 45 compiled tests passed. Scripts and private comparisons are in `area-action4-keeper-handler-20261105` under the analysis directory. These are constructed original CPU cases, not live presentation, random scheduling or a complete match. Next trace result0 $88F3, result1 loose recovery $8BDF, and result4 $9366 to their next supported match boundaries. Overall completion remains unmeasured.

## 2026-11-05 - action4 keeper result1 ball boundary check

- Twenty-four controlled original result1 cases (stamina50/51, two prior results and six input masks) continued from $8BDF to $8CA4. With original ball56,124 and frozen random255/255, ball coordinates became32,100, jitter scratch $062C became131, and experience and other represented field bytes remained unchanged. No additional cooperative waits occurred in this segment; $8CA4 is before the boundary decision.
- Added saved phase177 and CLI `--advance-area-action4-keeper-jitter` using existing byte-accurate jitter arithmetic. The implementation accepts only the observed owner12, ball56/124, random255/255 and initial jitter-control0; untraced inputs reject. Validation now permits the observed out-of-field ball specifically at phase177, with older checks unchanged. Focused regression failed against the placeholder, then exposed the existing interior-ball validator; after the scoped validator change it passed and snapshots roundtripped.
- Twenty-four original/native full snapshot comparisons matched field, experience and jitter control. Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-keeper-result1-jitter-20261105`. This is not yet the original boundary decision, live frame/presentation/random execution, or a full match. Next trace the $8CA4 boundary decision and its restart setup, as well as result0 and result4 continuations. Overall completion remains unmeasured.

## 2026-11-05 - sampled action4 keeper result1 restart entry

- Twenty-four controlled original cases continued from the prior out-of-field $8CA4 boundary check to bank26 $92EE without further host waits. In the sampled state, keeper0 stun6 became0 and keeper11 penalty79 became0; their other represented keeper fields, the rest of the field, experience, and jitter scratch were unchanged. The route is before restart presentation and setup.
- Added saved phase178 and `--route-area-action4-keeper-restart`, restricted to the sampled side, owner, ball, random, jitter and complete represented keeper states. It preserves the existing state and atomically rejects other inputs; no general keeper reset is inferred. The focused regression failed against a placeholder then passed, including snapshot roundtrip and replay rejection. Twenty-four original/native full snapshots matched the original field, experience and jitter scratch at $92EE.
- Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-keeper-result1-restart-20261105` outside Git. A disassembly probe shows $92EE enters presentation and restart setup, but that continuation is unimplemented; trace it before connecting existing restart systems. Result0 and result4 tails, live timing/random/audio and a complete playable match remain unfinished; overall completion is unmeasured.

## 2026-11-05 - sampled keeper restart choice capture

- Twenty-four controlled original result1 restart cases continued from $92EE through two cooperative presentation sequences to $9318, with 193 host waits. The original selected ball owner14, action1, subtype0 and choice6; represented field and experience otherwise remained unchanged. The earlier action4 remains the native session origin; these newly captured original scratch values are stored separately and must not be mistaken for live AI execution.
- Added phase179 and `--prepare-area-action4-keeper-restart OWNER ACTION SUBTYPE CHOICE`, limited to the observed 14/1/0/6 values and saved restart entry. It records the captured choice and owner after explicit acknowledgment, rejects unobserved inputs, and leaves its input unchanged. Session marker135/version69 appends the three-byte restart context; the updated reader continues to accept older snapshot formats and a downgraded new snapshot rejects. This does not run presentation or calculate the restart AI.
- The focused test failed against a placeholder, then passed after implementation; 24 original/native full snapshots matched represented field, experience and captured action/subtype/choice at $9318. Windows full CMake build and all 45 compiled tests passed. Private observations and comparisons are in `area-action4-keeper-restart-choice-20261105`. Next trace the prepared action through $9E0D/$9C0F and its ensuing restart continuation, then address result0/result4 and the remaining match loop. Overall completion is unmeasured; a live playable match remains unfinished.

## 2026-11-05 - captured action4 restart formation

- Twenty-four controlled original restart-choice cases continued through $9E0D/$9C0F to $9335, using eight cooperative host waits. The ball moved from32,100 to52,172/cell11; player coordinate high bytes were repositioned in four observed formation variants, while represented experience and sampled action scratch were unchanged. Original placement calculations, frames and presentation are not implemented here.
- Added phase180 and `--ack-area-action4-restart-formation FIELD` to accept an explicitly captured field snapshot. It requires the sampled starting and ball coordinates, preserves low coordinate bytes, player targets and statuses, and rejects unrelated field changes. It stores the supplied positions and retains the version69 restart choice. The caller is responsible for supplying a truthful captured formation; the code does not validate that all supplied positions match an original formation or compute them.
- The focused regression failed against a placeholder and passed after implementation. Twenty-four original/native full snapshots matched the captured fields at $9335. Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-restart-formation-20261105`. Next derive and implement original formation placement rather than relying on captured fields, then trace restart action execution and other keeper routes. Live frame/random/audio, a full match and overall completion remain unverified/unmeasured.

## 2026-11-05 - computed sampled restart formation

- Traced original $9E0D/$9BEE for restart choice6: it indexes a 22-cell formation table by side and player, substitutes the owner cell, converts cells to coordinates, and reflects coordinates for side11. The sampled keeper entries are skip markers, so this path does not infer unrepresented keeper coordinates. Original $9C0F then requests menu/presentation; the represented field did not change between $931E and $9335 in the sampled probe.
- Added `--compute-area-action4-restart-formation TABLE`, which takes an explicit 23-byte private table (22 cells plus owner cell), computes field-player positions while preserving fractional bytes, and derives the ball position from the owner. The path is restricted to the sampled side11/owner14/choice6 and sampled ball destination; invalid cells and non-skipped keepers reject. It reaches the same phase180 as the captured alternative. The private table was not added to Git and is not a live content loader or full restart/AI implementation.
- The focused test failed against a placeholder and passed after implementation. Twenty-four original/native complete snapshots matched four sampled formation variants using the private original-derived table. Windows full CMake build and all 45 compiled tests passed. Private table, observer and comparisons are in `area-action4-restart-placement-20261105` outside Git. Next trace the original restart action from $9335; broader formations, keeper positioning, live timing/randomness, other routes and a complete match remain unfinished. Overall completion is unmeasured.

## 2026-11-05 - sampled restart action power

- Twenty-four controlled original restart cases continued from $9335 to $9338 with incoming carry1 and zero further waits. Original action1/subtype0 preparation produced cost20 and power21, without changing represented field or experience. Inputs used side11, owner14 and explicitly frozen random255/255; presentation, the AI choice and prior formation were staged.
- Added phase181 and `--prepare-area-action4-restart-power`. It calls the existing original-byte attack preparation using private content and captured carry1, accepts only the sampled action/side/random and cost20/power21 result, and retains the prior action4 session context separately. Marker136/version70 appends the two 16-bit values; older snapshot formats remain readable and a downgraded new snapshot rejects. Untraced inputs reject atomically.
- Focused regression failed against the placeholder, then passed after implementation, including snapshot roundtrip and rejection. Twenty-four original/native full snapshots matched original field, experience, cost and power. Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-restart-power-20261105`. Next trace the action presentation, flight/destination and subsequent possession route from $9338, then other keeper results and live match scheduling. Complete match progress remains unmeasured.

- Follow-up original-only single-case probe continued from $9338 to $9342 with three cooperative waits. Represented elapsed changed1 to19, clock low byte190 to172 and $05F9 changed0 to1; other represented field and experience bytes did not change. This is one constructed sample, not a native comparison or live presentation. The probe is saved in the private restart-power analysis folder; compare additional clock/interrupt cases before implementing this continuation.

## 2026-11-05 - sampled restart action clock and interruptions

- Sixty controlled original cases followed the saved action1 restart from $9338 through the clock: 12 each at remaining/extra 446/0, 18/0, 17/0, 0/50 and 0/1, with two prior results and six input masks. Original first incremented $05F9 by1, added18 to field elapsed, applied clock/status recovery, and reached $9342 (24 ready), $D214 (24 added time), or $DA98 (12 period end). Some cases used three cooperative waits; original presentation, NMI and random production were not executed live.
- Added saved phases182/183/184 and `--advance-area-action4-restart-clock`, reusing byte-accurate keeper recovery and match clock arithmetic. The operation is restricted to the five traced clock cases and sampled statuses; unknown inputs reject atomically. Version70 snapshots retain the restart context. Focused regression failed against a placeholder, then passed with snapshot/replay checks. Sixty original/native full snapshots matched field, clock and experience at the three boundaries.
- Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-restart-clock-20261105`. A separate single original-only ready-case probe found action1 dispatch reaches $9359 after $9342; later action execution and added-time resume are not yet implemented. Next compare and connect that dispatch and resume, then continue the other keeper and match routes. Overall completion remains unmeasured and there is no complete playable match.

## 2026-11-05 - sampled restart resume and action dispatch

- Twenty-four controlled original added-time cases (remaining17/18 before the clock) resumed from $D214 to $9342 after 45 cooperative waits, without changing represented field, clock, experience or sampled action scratch. Twenty-four ready cases reached the action1 dispatch entry $9359 from $9342 with zero further waits and no represented changes. These are constructed, frozen-random observations, not live presentation or gameplay.
- Added `--resume-area-action4-restart-clock` to acknowledge the sampled added-time presentation without repeating the clock, and phase185 / `--dispatch-area-action4-restart` for the captured action1 dispatch entry. Only observed clock states are accepted; period-end and other cases reject atomically. The focused regression failed against a placeholder and passed after implementation, with snapshot and replay checks. Original/native full snapshots matched for 24 resume and 24 dispatch cases.
- Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-restart-dispatch-20261105`. Next trace action1 from $9359 into the original pass/possession route, and the other goalkeeper outcomes; a complete live match and overall percentage remain unverified/unmeasured.

## 2026-11-05 - sampled restart pass entry

- Twenty-four controlled original action1 dispatch cases reached $C615, immediately before $DE5E, after 40 cooperative waits. The original set $061A from255 to77; represented field, clock, experience and other sampled context remained unchanged. These cases use staged presentation and frozen random values, not a live match.
- Added phase186 and `--ack-area-action4-restart-pass` to record the captured pass-entry counter77, limited to the two directly observed ready clock states. Snapshot marker137/version71 saves the new counter; the current reader still accepts older formats and a downgraded new snapshot rejects. Other contexts, including the added-time-resumed pass entry, remain untraced and reject. The focused regression failed against a placeholder and then passed with snapshot/replay checks.
- Twenty-four original/native complete snapshot comparisons matched field, clock, experience and the observed pass counter. Windows full CMake build and all 45 compiled tests passed. Private traces and comparisons are in `area-action4-restart-pass-entry-20261105`. A single original-only probe continued through the shared $DE5E flight to $DE73 with 15 cooperative waits, clearing selected defenders and moving the ball; this later flight/recipient is not yet implemented for this restart. Next trace the flight setup and per-cell movement before connecting possession. Complete match progress remains unmeasured.


### 2026-11-05 - Sampled action4 restart pass flight

Connected phase186's observed action1 restart to the existing staged area flight, including the original counter77, recipient15, destination113 and flag1. The transition is restricted to the captured formation/trajectory case and reuses the existing numeric movement; no live presentation was implemented. A focused test first failed with the transition stub, then passed with the implementation, including saved-state round trip and invalid-entry checks.

Controlled private original traces versus native: 24 setup cases (two clock states, six inputs and two prior results) matched represented field, clock, experience and numeric trajectory at $E773. Twelve ordinary-clock cases matched those categories again at $DE73 after 15 original cooperative waits. Intermediate cells, original animation/audio/random timing, other restart formations and ownership after $DE73 have not been compared here; native metadata was not compared to unrepresented original internals. Windows CMake build passed; all 45 compiled test executables passed. Private scripts and reports: ../Captain-Majid-analysis/area-action4-restart-flight-setup-20261105.

Next: compare intermediate movement and recipient/control transfer against original, then continue remaining keeper outcomes and match situations into a verified full classic match. Overall progress percentage remains unmeasured.


### 2026-11-05 - Sampled restart recipient control

Original execution from $DE73 reaches $E0DF after 95 cooperative waits, with recipient15 becoming owner; 12 controlled original/native cases matched represented field, clock, experience and trajectory at that boundary. The original then reaches player-control-ready. Added a guarded transition from the saved recipient handoff into the existing general control setup, consuming no-longer-needed flight context. A focused regression failed before the implementation and passed afterward; the Windows build and all 45 compiled test executables passed. Twelve original/native cases matched represented field, clock and experience at control-ready. These comparisons do not cover live animation, sound, intermediate flight cells, or every possible restart and clock case.

Next compare this path's intermediate flight cells and control input/clock progression, then continue remaining keeper outcomes and match routes. Full playable-match verification remains outstanding; overall percentage is unmeasured.


### 2026-11-05 - Sampled restart control tick

After the restart pass enters control, compared 36 cases spanning two prior outcomes, six earlier inputs and three current inputs. The represented field and clock matched original at the control-poll wait, after setting the decision timer to zero in both systems and polling again, and after the ensuing control clock tick to field-round pending. These are boundary comparisons, not full snapshot or real-time presentation comparisons. No additional gameplay implementation was needed. Next compare the original field update and intermediate flight cells; other keeper and match paths and an end-to-end playable match remain open. Evidence is private under ../Captain-Majid-analysis/area-action4-restart-flight-setup-20261105.


### 2026-11-05 - Sampled restart field round

After the sampled restart pass and control tick, ran a no-input field round in the original and native engine. In 36 cases the represented field and clock matched upon return to control-ready. This validates one staged path around the control loop, not a complete real-time match; intermediate ball-flight cells, different world inputs, other keeper outcomes and other match routes remain to be compared. Evidence remains private in ../Captain-Majid-analysis/area-action4-restart-flight-setup-20261105.


### 2026-11-05 - Restart flight cells and field inputs

For 12 original/native sampled restart flights, compared represented field, trajectory, clock and experience at each movement and callback boundary: 15 movement steps and 14 callbacks per case, 348 matches. Original waits were explicitly acknowledged with no scheduler/NMI execution. For the ensuing field round, 144 additional original/native cases used four repeated held/pressed input patterns (1/0, 2/0, 64/64, 128/128), three prior poll inputs, six earlier inputs and two prior outcomes. Represented field and clock matched upon control-ready in all cases. Other snapshot metadata, real-time audiovisual presentation, other restart formations and match paths were not covered. No code changed; the build and 45 tests were verified at the preceding checkpoint. Next investigate other goalkeeper outcomes and match situations. Private evidence: ../Captain-Majid-analysis/area-action4-restart-flight-setup-20261105.


### 2026-11-05 - Sampled action4 keeper result0 goal

From result0 $88F3, controlled original cases reached bank26 $85F0 after 191 cooperative presentation waits. With side11, owner12, ball56/124, frozen random127/128, clock446 and score0-0, the right score became1 and the carrier gained goal experience (sampled total6 to21); represented field and clock were unchanged. Thirty-six cases covered keeper stamina49/50/51, two preceding results and six earlier input masks. Earlier original trace probes with a smaller instruction bound timed out during presentation and are not evidence of a gameplay hang.

Added guarded phase187 and `--prepare-area-action4-keeper-result0-goal`, using the existing goal experience calculation and score context. Only the sampled state is accepted; duplicate transitions and other scores reject atomically. Focused regression failed before the implementation, then passed including snapshot roundtrip. Thirty-six full native saved snapshots matched original field, clock, experience and score at $85F0; other context bytes remained as in the valid native input. Windows `cmake --build build -j 2` passed; all 45 compiled test executables passed. This does not simulate real-time presentation or prove other goal scenarios. Next trace the result0 post-goal route toward kickoff, result4 and other match situations. Private scripts and logs: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-05 - Sampled result0 post-goal kickoff boundary

Continued the sampled side11 result0 original goal through its celebration and post-menu cleanup. Twelve original/native cases matched represented field, score, clock and experience at $85DD after 333 cooperative waits: side11 changed to side0, keeper0 stun6 cleared, and kickoff marker became4. The native transition acknowledges presentation, clears consumed native action/area metadata, and enters the existing saved kickoff boundary using a narrow validator branch; it does not run audiovisual frames. The focused regression failed before implementation, then passed including roundtrip and replay rejection. Windows full build and all 45 compiled tests passed. A two-case original-only probe continued seven waits to $DC07 formation entry, setting owner8 and clearing keeper11 penalty79; it remains unimplemented. Next compare and integrate kickoff preparation without guessing its elapsed/defender state. Evidence is private under ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-05 - Sampled side0 kickoff formation after result0 goal

Traced the side11 goal from post-goal cleanup into formation entry $DC07 (seven cooperative waits) and through its return. The original sets owner8 and clears keeper11 penalty, then places the formation; elapsed1 and selected keeper0 remain. Extended the existing formation transition and validation for this specifically guarded side0 route, without discarding those observed values. A focused regression first failed at the former formation validator, then passed and roundtripped; unsupported side11 selection rejects. Twelve original/native cases matched the represented field, goalkeeper formation positions, clock, score and experience after formation. Windows full build and all 45 test executables passed. Original audiovisual scheduling and receiver/control continuation are not established. Next trace that kickoff continuation, result4 and other match routes. Evidence including private formation table: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - Sampled post-goal kickoff control

Continued the sampled side11 goal through side0 receiver selection, ball placement, acknowledged transfer and regular control setup. Original traces showed receiver7 for captured random127, 189 cooperative waits between placement and transfer, and a return to control-ready with owner7. Extended the existing saved kickoff validators for this bounded route, retaining elapsed1 and selected keeper0 until normal control setup. The receiver transition explicitly rejects unobserved random and candidate inputs; focused tests first failed at the previous validation and passed after the change, with snapshot roundtrip and rejection checks.

Twelve original/native cases matched represented field, clock and experience at each of four boundaries, totaling 48 comparisons; score, goalkeeper formation positions and receiver were also checked at the first three. Other snapshot metadata and real-time audiovisual presentation were not compared. Windows full CMake build and all 45 compiled test executables passed. Next compare control input/clock/field progress from this route, then trace result4 and other match situations toward a verified playable match. Overall completion percentage is unmeasured. Private traces and comparison scripts: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - Post-goal control, action menu and retained action correction

The sampled post-goal kickoff control was compared against original for 36 input polls (including 12 menu requests), 36 polls with the decision timer explicitly set to zero on both sides, and 24 applicable subsequent clock ticks; boundary, represented field, clock and experience matched. The original's menu branch was then traced through menu opening, ordinary-shot confirmation and preparation in 12 cases; represented field, clock, experience and goalkeeper positions matched at all 36 boundaries. The menu presentation is still host-acknowledged rather than live.

A first diagnostic compared original $043F to native shot power and appeared to mismatch. The established original mapping uses $061C/$061D; both original and native prepared power were11. A separate real discrepancy was confirmed: the prior post-goal handoff reset represented action metadata, whereas the original retained action0, route3, cursor128, flash128 and power22 until menu processing. A focused regression failed before replacing the reset with this narrowly observed state and passed afterward. Rebuilt the entire sampled route for 12 cases (120 native transitions): all nine represented action bytes matched original at seven post-goal, kickoff, control and menu boundaries per case; represented field/clock/experience matched through shot preparation. Original scratch not represented by the engine and live audiovisual scheduling remain outside coverage. Windows full CMake build and all 45 compiled test executables passed. Next investigate the subsequent original shot route and result4; no playable-match completion percentage is measured. Private scripts/logs: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - Sampled kickoff loose shot and defended arrival

After the sampled side11 goal and side0 kickoff, traced the ordinary shot through elapsed clock, result dispatch, acknowledged attack tail and approach. Twelve original/native cases matched represented field, clock, experience, all action bytes, score and goalkeeper positions at four boundaries (48 comparisons), including the loose flight setup. Original/native numeric flight setup, every cell step and callback matched field, clock, experience and trajectory at 312 checkpoints across 12 cases, with cooperative original waits explicitly acknowledged. Each arrived at cell209 with flight flag0 and ordered defenders19/14.

The original reaches bank26 $8835 with that queue, then first-defender announcement $8845 after one cooperative wait. The native engine formerly rejected this case. Extended the existing saved defender-pending and announcement phases150/151 for only this observed flag0 loose-shot state; the pass-specific preparation explicitly rejects the new case until its distinct action0 path is implemented. Version67/marker133 encoding now writes its required zero flag and absent pass-context bytes for these phases. A focused regression initially failed, then exposed and verified a corrected truncated-snapshot encoding; it now passes, including roundtrip, replay/unknown-input rejection. Twenty-four original/native boundary comparisons matched represented field, clock, experience, ordered defenders and flight flag. Original-only continuation identified defender19/action0/result2; no native comparison or result handler is claimed yet. Windows full CMake build and all 45 compiled test executables passed. Next trace and implement this specific defender's preparation/comparison and terminal result before further match work. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - First sampled loose-shot defender preparation

Added saved phase188 and `--prepare-sampled-loose-shot-defender` for the already traced flag0 loose arrival. It saves defender19 and variant0 before the original action0 comparison without supplying a guessed stamina cost. Twelve original/native cases matched represented field, clock, experience, defender ID and ball variant at $886D. Focused regression failed before implementation, then passed including snapshot roundtrip and replay rejection; marker133 includes the new state while previous supported snapshot versions remain readable. Windows full CMake build and all 45 compiled test executables passed.

In 12 original-only internal traces, $8EE9 received defender scratch power19, incoming11, random0/128, action0, range192 and offset64; the original returned strength9, loss2, score78 and result2 at $8870. The existing native comparison arithmetic independently returned those values for the same captured inputs and table. This does not yet save the comparison or implement its result2 handler. Next compare and save that bounded result, then trace the original terminal continuation. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - Sampled loose-shot defender comparison

Added guarded phase189 and `--compare-sampled-loose-shot-defender`, requiring captured power19 and the observed action0 private flight content. Existing byte-accurate comparison computes strength9, loss2, score78 and result2 from incoming11 and frozen random0/128; other power/table inputs reject atomically. Focused regression first failed with a stub, then passed including version67 snapshot roundtrip and replay rejection. Twelve original/native cases matched represented field, clock, experience, selected defender and saved comparison arithmetic at $8870. Windows full CMake build and all 45 compiled test executables passed.

Original-only continuation reached result2 handler $88BB after 194 cooperative waits, then $81BC with ball188/124 changed to140/140 and one experience point awarded. The original cell value remained stale209 there. These later steps are not yet native behavior; explicit presentation waits do not simulate real-time presentation. Next trace and implement result2 and its clock/recovery continuation, then other keeper and match routes. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105. Overall completion percentage remains unmeasured.


### 2026-11-06 - Sampled loose-shot result2 and recovered owner

Traced 12 original result2 cases through $88BB, $81BC, $81C3 and $8BDF. The first handler awards defender experience and sets ball position to140/140 while leaving cell209 stale; the terminal prelude awards the defender again, applies action0 contact status and elapsed2, and advances clock445 to443; the queued defenders then clear. The original used 194 cooperative presentation waits for the result and three for the clock segment. Added `--recover-sampled-loose-shot-result2` as one guarded saved transition that explicitly acknowledges those presentation effects (facing64 and motion0), performs the two original awards and clock prelude, clears the consumed flight/defender metadata and enters ordinary ball recovery. Unsupported clock/content/position/replays reject atomically. Focused regression first failed with a stub; original/native session comparison then revealed facing/motion mismatches, which were added to the regression and corrected. Twelve original/native cases now match represented session bytes 11..323 at $8BDF.

Ran the existing recovery step on the resulting states: 12 original/native cases matched represented session bytes 11..323 at recovered owner handoff, with player8 as owner. This remains a controlled, staged route; no live frame/audio/presentation parity is claimed. Windows full CMake build and all 45 compiled test executables passed. Next trace continuation to player control and remaining match routes. Private traces and comparisons: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105. Overall completion is unmeasured.


### 2026-11-06 - Sampled recovered player returns to control

Continued the sampled result2 loose-shot recovery from player8 ownership to the original player-control handoff and control-ready. The first 12-case comparison found that the original recovery presentation changed facing64 to0, which the native prepared tail had retained as64. A focused regression failed before adding the specifically guarded observed facing reset, then passed. Twelve original/native cases now match represented session bytes 11..323 at each of the player-control handoff and control-ready boundaries, 24 comparisons total. The original presentation is acknowledged, not executed live. Windows full CMake build and all 45 compiled test executables passed. Next investigate result4 and other match situations toward a complete match; overall progress is unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result0-goal-20261105.


### 2026-11-06 - Sampled keeper result4 restart branch

Traced result4 from saved handler $87D4 through $9366 and captured decision $936E to $94CF in 12 controlled original cases. For the sampled side11 owner12 ball56/124, frozen random0/0 and clock446, the boundary check reported carry1 and side0; represented field, clock, experience and score were unchanged. Added guarded phase190 and `--route-sampled-keeper-result4 CARRY SIDE`, requiring the explicit captured 1/0 values and the traced state. Focused regression failed before implementation, then passed including marker134 roundtrip and invalid/replay rejection; 12 original/native full saved snapshots matched at $94CF. Windows full CMake build and all 45 compiled test executables passed.

Original-only continuation reached $94D7 after 242 cooperative waits and formation entry $94E6 after another 175, with owner20, ball68/132/cell30 and relocated players. At $94EC the original had selected action9 and power21 after 41 further waits; at $94F8 the extra-time counter was5 after 221 more waits, while field elapsed remained1. These later formation, action and clock changes remain unimplemented. Presentation acknowledgments are not live audiovisual execution. Next trace and save the bounded restart setup and action, then remaining match paths. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106. Overall progress is unmeasured.


### 2026-11-06 - Captured goalkeeper result4 restart formation

Implemented phase191 and `--ack-sampled-keeper-result4-formation FILE` for the observed restart formation at $94E6: owner20 and ball68/132/cell30. The explicitly captured field may change player position high bytes but must preserve position fractions, target/status and all unrelated field state. This acknowledges the original's 242- and 175-wait presentation/selection segments; it does not calculate the selection or execute live frames. A focused regression first failed with a stub, then passed after implementation including snapshot roundtrip, replay rejection and unrelated-field protection. Twelve original/native full saved snapshots matched the controlled original formation cases. Windows full CMake build and all 45 compiled tests passed. Next trace the action9/power21 selection and clock, then remaining match situations. Original evidence and private captured fields remain in ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106. Overall progress is unmeasured; a complete playable match remains unverified.


### 2026-11-06 - Captured goalkeeper result4 restart action

At original $94EC, 12 controlled cases captured action9, subtype128, context5, cost40 and power21 after the result4 formation. Added guarded phase192 and `--prepare-sampled-keeper-result4-action ACTION SUBTYPE CONTEXT COST POWER`; these are explicit observed inputs and no AI or action-power calculation is claimed. The previous action4 origin is retained separately. Focused regression failed with a stub, then passed with snapshot roundtrip, older-version rejection and invalid/replay checks. Twelve original cases matched the captured values and full native snapshot preservation. Windows full CMake build and all 45 compiled tests passed.

Correction to earlier note: original $05F9 maps to the extra-time counter, not field elapsed. At $94F8 that counter changed from0 to5 while field elapsed remained1. The next task is to trace and implement the timing/interruption behavior and ensuing action9 route; the original's cooperative waits do not simulate live presentation. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106. Overall progress remains unmeasured and a complete playable match remains unverified.


### 2026-11-06 - Sampled result4 extra-time byte update

Traced original $94EC..$94F8 after the captured result4 action. In 96 controlled cases spanning eight clock inputs, the original added5 to $05F9 (extra time) with 8-bit wraparound; remaining match time, field elapsed, represented field and experience did not change. The original used 221 cooperative presentation waits per case and reached $94F8 even for zero remaining time in these samples. Added guarded phase193 and `--advance-sampled-keeper-result4-extra-time`; it records this byte update and explicitly acknowledges the presentation, without advancing the match clock or claiming live presentation. Focused regression first failed with a stub then passed, including snapshot roundtrip, replay/unknown-input rejection and wrap cases. Ninety-six original/native full saved snapshots matched. Windows full CMake build and all 45 compiled test executables passed. Next trace the original action9 dispatch and any later match-clock/interruption boundary. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106. Overall progress is unmeasured; a complete playable match remains unverified.


### 2026-11-06 - Sampled result4 action returns to recovery

From original $94F8, 12 controlled cases dispatched the captured action9 and reached $8BDF without additional cooperative waits. The original awarded the opposing goalkeeper selector3 experience, cleared the selected queue, moved the ball from68/132 to80/128 while retaining stale cell30, and changed keeper0 timer/duration/penalty from0/0/0 to7/4/79. Added guarded `--recover-sampled-keeper-result4-action` for the observed clock446/extra5, random0/0, goalkeeper and position inputs. It retains the original action9 context and subtype while clearing consumed restart metadata. A focused regression failed against the stub; original comparison then exposed display-parameter2 versus0, which was added to the test and corrected. Twelve original/native cases matched represented session bytes 11..323 at recovery entry.

The existing recovery transition then matched 12 original cases at recovered-owner handoff, with side0 player3 in possession. Original audiovisual effects remain acknowledged/captured rather than live; other result and timing inputs remain unverified. Windows full CMake build and all 45 compiled tests passed. Next trace the recovered-owner continuation and other match situations. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106. Overall completion is unmeasured; a complete playable match remains unfinished.


### 2026-11-06 - Sampled result4 recovered player returns to control

Continued the sampled result4 loose recovery from side0 player3 ownership to the original player-control handoff and control-ready. Twelve controlled original/native cases matched represented session bytes 11..323 at each boundary (24 comparisons). Existing native recovery/control transitions required no changes. This is a staged CPU comparison with presentation acknowledged, not live match execution. Windows full CMake build and all 45 test executables passed at the preceding implementation checkpoint. Next compare subsequent input/clock behavior and continue remaining keeper/match paths; overall progress is unmeasured and a playable match is not yet verified. Evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106.


### 2026-11-06 - Result4 control through next ordinary shot

The sampled result4 recovery route matched original/native represented session bytes 11..323 for 96 input/timer/clock comparisons, 72 field-update comparisons, and 36 action-menu/ordinary-shot preparation comparisons. Original-only traces of 12 subsequent shots reached the approach with distance135 and destination233. The native direct continuation rejects the original retained defense-action byte9 because the contact prelude accepts 0..6. An exploratory substitution of action0 ran all 48 shot boundaries but is not validated as original behavior: every comparison differed at byte304 (motion request: original2, native0), and action0 was not the original input. Do not treat this as a passing parity result or broaden accepted values without tracing the original table lookup and presentation effects. Next investigate the original action9 clock/table reads and the acknowledged shot animation, add focused tests and only then implement evidence-backed behavior. The latest full Windows build and all 45 tests passed at the prior implementation checkpoint; these subsequent checks did not alter engine code. Live presentation and a complete playable match are unverified; overall progress is unmeasured. Private evidence: `area-action4-keeper-result4-restart-20261106`.



### 2026-11-06 - Result4 recovered shot approach

For the sampled result4 recovered-player ordinary shot, traced original bank26 action9 clock lookup: it reads $9107=0, while the existing action0 keeper entry is also0. Added a narrowly guarded action9 continuation only for the observed keeper0/clock445/extra5/owner3/ball80,128 case, and acknowledge the observed motion request2 at its preceding shot dispatch. This uses the existing action0 prelude only when its keeper elapsed is zero; unrelated action9 inputs still reject. Two focused regressions failed first and now pass. Twelve original/native cases matched represented session bytes 11..323 at each of four boundaries (48 comparisons) from final shot clock through approach, using the actual original action9. Full Windows CMake build and all 45 test executables passed. Original presentation is acknowledged, not live; coverage is sampled, other match routes and a complete playable match remain unfinished. Next trace shot flight and its defender/keeper outcome and compare with the engine. Overall progress remains unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106.


### 2026-11-06 - Result4 shot flight and generic goalkeeper

The sampled result4 recovered-player shot flight matched the original across 12 cases and 384 saved setup/movement/callback checkpoints, arriving at cell233 with no field defenders. The existing native route and keeper-entry transitions then matched represented session bytes 11..323 in 12 cases each. With explicitly supplied goalkeeper choices, original-only constructed traces observed choice0 result3 and a keeper save/side11 ownership, and choice1 result0 and score1-0, 12 cases each; these choices are inputs, not observed live AI selection. Native keeper comparison rejected all 24 because the goalkeeper is generic type0 and the private pack has no runtime generic team profile. The exporter intentionally leaves that table empty; do not substitute an unverified profile. Next trace the original generic team selection/context and implement or supply evidence-backed private profile mapping, then compare keeper results and their continuations. Windows full CMake build and all 45 test executables passed at the preceding shot implementation checkpoint; no engine code changed in this follow-up. Live presentation and a complete playable match remain unverified; overall progress is unmeasured. Evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106.


### 2026-11-06 - Generic keeper trace limitation

Further tracing found that the constructed keeper comparison reads the generic-team pointer from scratch bytes $38/$39, which in this captured setup resolves into zero-page RAM at $0002 rather than a verified selected-team table. Thus the earlier constructed choice0/save and choice1/goal outcomes are not reliable evidence of real-match outcomes, although they are reproducible from that constructed scratch state. Do not implement or claim those results as original match behavior. The smallest missing input is an original capture establishing the selected team and valid generic-team pointer/table at the real keeper loader entry; trace that setup before comparing generic keeper behavior. The earlier matched flight and keeper entry boundaries do not include this comparison and remain valid within their disclosed scope.


## 2026-11-06 - Explicit generic goalkeeper team data (newest checkpoint)

Original bank28:$8B39-$8B49 derives a team-data pointer using the explicit team ID and pointer table $BAB2. IDs3..6 were traced to mapped records; no real team identity is established for the constructed saved scenario. `tools/export_shot_content.py --team-id N` now exports an explicitly selected private record for IDs3..6; omission preserves the original output byte-for-byte and untraced IDs reject. Team choice remains the caller's responsibility, and private packs remain outside Git. A synthetic unit regression failed before implementation and passes; private CLI verifies selected output, existing-file protection, and untraced-ID rejection. The four new outputs exactly match the manually constructed private packs used below.

For each of those four explicitly supplied teams and keeper choices0/1, 12 original/native constructed keeper comparisons and 12 subsequent goal-registration comparisons matched represented session bytes 11..323 and keeper/goal fields (96 cases at each boundary); the observed score was 1-0 in these constructed inputs. These are not evidence of the actual match's selected team, AI choice, or live presentation. The earlier scratch-pointer outcome remains invalid for real-match inference. All 45 Windows test executables passed after this Python-only change; the last full Windows build passed at the preceding implementation checkpoint. An initial separate CLI attempt in sandbox temporary storage failed with an access error and cleanup error; repeating in the authorized workspace passed. No engine or Japanese-text files were changed in this increment. Next capture or trace the real scenario's team selection and valid team pointer, then continue its verified goal/restart or save route and broader match loop. Overall progress is unmeasured; a complete playable match remains unverified. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/keeper-generic-controlled-20261106-v1.


## 2026-11-06 - Supplied-team goal returns to kickoff control (newest checkpoint)

Continued the existing constructed result4-to-shot route for explicit original team IDs3..6, 12 saved variants and keeper choices0/1. All 96 original/native comparisons matched at each of six subsequent boundaries: post-goal cleanup, kickoff formation, receiver selection, ball placement, prepared possession handoff, and player-control setup (576 boundary comparisons). Cleanup, formation, receiver and ball comparisons cover represented session bytes 11..323 plus relevant score/formation/receiver bytes; handoff compares the complete saved snapshot; control compares bytes11..323. The sampled original goal has score1-0 and side11 kickoff; no live presentation is simulated and the original post-display handoff explicitly acknowledges display effects. Existing native transitions required no changes. The original formation trace initially exceeded a helper's 2,000-instruction bound; increasing the trace bound to 20,000 completed all 96 cases.

The scenario was constructed from prepared test states and its actual opponent team is not established. Supplying a traced team and keeper choice gives controlled evidence, not a verified real-match selection or AI decision. Next obtain a valid real original match/team capture or trace original match initialization, and then compare the corresponding team selection and subsequent live match route. Other match situations and a complete playable match remain unfinished; overall progress is unmeasured. The selected-team exporter unit test and all 45 existing Windows test executables passed in this increment; no engine code or Japanese text was changed and no fresh full build was needed. Existing Git status remains unavailable with "this operation must be run in a work tree"; all existing changes were preserved. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/keeper-generic-kickoff-20261106.


## 2026-11-06 - Original stage-to-opponent initialization (newest checkpoint)

Original bank2:$88AF-$88B6 loads stage byte $26 and writes opponent team ID $2B as stage+3. Original bank28:$8B39-$8B4B then uses that ID to initialize the generic-team pointer via $BAB2. Executed both original instruction sequences for stages0..3: IDs3..6 and pointers $BAF6, $BB00, $BB0E, $BB1C respectively; each pointed record matched the existing explicitly selected private exporter. All 12 constructed keeper-entry states have stage0 but inconsistent uninitialized team byte0. Restoring the observed stage0 initialization selects team3 for that constructed scenario, making the preceding team3 controlled comparison the initialization-consistent variant. This does not prove an actual live match was played or that these constructed states and team values correspond to a captured match; team3 is not a claim about an unrelated real save. Keeper choice remains supplied, not live AI.

The earlier 96-case supplied-team continuation reaches player-control setup across six compared boundaries; stage0/team3 is a 24-case subset (12 variants times two choices), with explicitly acknowledged presentation. No engine code changed in this investigation; selected-team exporter unit test and 45 Windows test executables passed at the immediately preceding checkpoint. Next trace real match initialization or capture a live original match to verify team/stage and unsimulated effects, then continue player input and remaining match situations. Overall progress remains unmeasured; a complete playable match is unverified. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/keeper-generic-kickoff-20261106.


## 2026-11-06 - Stage0 controlled kickoff input and field round (newest checkpoint)

Reconstructed 24 stage0/team3 kickoff-control states from the prior supplied-team goal continuation (12 saved variants, two explicitly supplied keeper choices), preserving their constructed nature and acknowledged presentation. Compared original/native represented session bytes11..323 for four held/pressed pairs 0/0, 1/0, 64/64 and 128/128: 96 input polls matched at control-poll wait. With the control timer deliberately set to zero in both inputs, 96 more polls matched at control-clock pending and all 96 subsequent clock ticks matched at field-round pending. From those ticks, three field input pairs 0/0, 1/0 and 64/64 produced 288 matching original/native field-round comparisons, returning to control-ready. Total 576 comparisons; existing native transitions required no changes. The selected-team exporter unit test and all 45 existing Windows test executables passed; no new build or engine-code edit was required.

Stage0/team3 follows the earlier traced original stage-to-team initialization, but this is a constructed scenario, not a live match capture or evidence of the actual goalkeeper AI choice. No rendering, audio, NMI or live timing is simulated. A valid live original match/team capture or trace from full startup remains necessary to establish real match initialization and unsimulated behavior; remaining match situations and complete playable match are unfinished, and overall progress is unmeasured. Next trace the next opponent-possession action or repeat control/field cycles through an interaction, comparing original and native, while pursuing live initialization evidence. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-kickoff-input-20261106.


## 2026-11-06 - Opponent possession reaches AI pass (newest checkpoint)

From 24 constructed stage0/team3 post-kickoff variants with no held/pressed input and acknowledged presentation, original/native represented session bytes11..323 matched through 480 control-poll waits, 24 clock-pending polls, 24 field ticks, 24 field rounds, and 24 opponent-carrier cell-decision boundaries (576 comparisons). The carrier moved from cell152 to140 and the existing native state reaches phase12; no native continuation exists for this field AI decision. For all 24, a constructed original CPU/task continuation from $E19B reached $E1B2 with action1/subtype0/power22, $E1CB with action1/subtype128/power21, and bank26:$8027 after nine cooperative host waits. These AI choice/attack-preparation boundaries are original-only observations, not native parity.

One representative continuation reached bank26:$8107 after 31 cooperative waits and fixed-bank $C612 after 66; the original cooperative execution changed many player positions during presentation. This does not emulate NMI, PPU, audio or hardware timing and does not prove the same later behavior for all 24 variants. Next trace the original field AI selection, action1 preparation and pass/position effects; implement only supported, tested saved boundaries, with live/frame-dependent effects explicitly captured or unimplemented. Stage0/team3 and keeper choice remain constructed inputs, not a live match capture; actual match initialization is not yet independently verified. Cinema's separate RetroArch session was not accessed. No engine code was changed; selected-team exporter unit test and all 45 existing Windows test executables passed. A complete playable match remains unverified and overall progress is unmeasured. Git status still errors "this operation must be run in a work tree"; do not reset or overwrite existing work. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-opponent-possession-20261106.


## 2026-11-06 - Sampled opponent AI pass handoff implemented (newest checkpoint)

Added strictly guarded phases194..199 for the constructed stage0/team3 opponent-cell case: capture the explicitly supplied original action1/subtype0; capture prepared subtype128/power21; acknowledge the original presentation flag changes; advance the original carrier-cell reset; acknowledge a private captured pass-presentation field; and acknowledge the observed pass handoff. These operations do not calculate general AI choice, attack power, live animation, or frame scheduling. The field capture can replace motion values and the observed elapsed/movement-region values only; serialized unrelated field data must match. The guarded handoff records the observed ball variant and control fields, preserving other state.

For 24 controlled original variants, all six consecutive native stages matched represented session bytes11..323 (144 comparisons), including the captured field and acknowledged presentation. The original used 31 and 66 total cooperative host waits to pass-handler and $C612 handoff, respectively; these are not NMI/PPU/audio or real timing emulation. The first handoff comparison failed because recovery byte $44E was incorrectly mapped to special-owner flags; the regression exposed the mismatch, and correcting it to ball variant yielded 24/24 matches. Focused tests cover state preservation, snapshot roundtrip, invalid/replay cases and captured-field protection. Full Windows CMake build and all 45 test executables passed.

Original-only continuation from $C612 reached $DE5E/$DE61/$DE64 in all 24 cases: captured receiver19, destination103, counter255, distance3. These are not yet native pass target or flight parity. Next preserve the pass recipient, counter, destination and distance in a versioned saved context, trace/compare pass flight and subsequent possession, then connect remaining match paths. The AI and presentation inputs were captured from constructed states; actual match initialization, live AI/rendering and a complete match remain unverified. Cinema's RetroArch session was not accessed. Overall progress is unmeasured. Git status still errors "this operation must be run in a work tree"; preserve all prior changes. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-opponent-ai-pass-20261106.


## 2026-11-06 - Sampled opponent pass target captured (newest checkpoint)

Added phase200 and version72 marker138 to retain the explicitly supplied pass recipient19, destination103, counter255 and distance3 after the sampled action1 handoff. The guarded capture checks that the recipient's current field position maps to the supplied destination; phase validation enforces the same invariant after reload. Version72 is restricted to this new context and prior snapshot versions retain their existing layouts. The saved state, context values and represented session bytes11..323 matched 24 original $DE64 cases. Focused tests cover capture, input preservation, snapshot roundtrip, wrong/replayed input, malformed/truncated snapshots, old-version rejection, and a receiver-position mismatch. Full Windows CMake build and all 45 test executables passed.

Original-only traces for the same 24 constructed cases reached flight setup $E773, then crossed cells128, 116, 115 and103 with no selected defenders; flag1 was retained. After explicit frame acknowledgements the original returned to control handoff with side11 player19 owning the ball at119,140. The observed 176 cooperative waits are not NMI, rendering, audio or hardware timing. Native flight setup, per-cell flight, reception, and subsequent control have not yet been compared or implemented for this new field-pass route. A prior draft flight observer used the wrong bank number for $E773, was interrupted, and the corrected observer completed all 24 cases. Actual live-match initialization, live AI and presentation, other routes and a complete playable match remain unverified; overall progress is unmeasured. Next connect the saved target to numeric flight using the existing shared ball/trajectory code, compare each original boundary and retain the captured flag, then trace reception and resumed opponent control. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-opponent-pass-target-20261106.


## 2026-11-06 - Sampled opponent pass reaches control (newest checkpoint)

Connected phase200's explicitly captured stage0/team3 pass target to existing area/numeric ball flight using observed starting fractions127/0, shared direction/velocity arithmetic and original flight flag1. The sampled recipient19/destination103/counter255/distance3 remains constrained; this is not general AI target selection. Original/native comparisons matched 24 setup states, 96 cell steps, 72 defender callbacks, 24 recipient handoffs and 24 ball placements. All passed through cells128,116,115,103 without defenders and returned possession to side11 player19 at119,140.

Added a strictly guarded continuation from this received-pass position to player-control entry. It acknowledges the observed facing change, preserves flag1 in saved state, and clears consumed flight/target contexts. Original/native represented session bytes11..323 matched 24 control entries and 24 control-ready setups; a further 96 held/pressed input polls matched and preserved the flag (384 comparisons total across setup, flight, reception, control and input). Original 176 cooperative waits are acknowledged, not live NMI/rendering/audio or hardware timing. Focused tests cover replay/invalid capture, input preservation and snapshot roundtrip, including version72 marker138 pass-target rejection of malformed/truncated/older versions and position mismatch. Full Windows CMake build and all 45 test executables passed.

These are constructed stage0/team3 states with explicitly captured AI/presentation input, not a validated live-match initialization, general opponent AI or full-game parity. The original match capture remains unavailable while its RetroArch session is managed separately. Next follow subsequent opponent control through additional decisions, trace broader branches and original startup, and integrate genuine presentation/frame scheduling before testing a complete match through final whistle. A complete playable match remains unverified; overall progress is unmeasured. Git status still errors "this operation must be run in a work tree". Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-opponent-pass-flight-20261106.


## 2026-11-06 - Sampled pass reaches three-defender contact (newest checkpoint)

Continued the 24 constructed stage0/team3 pass-reception cases with no player input. Original/native represented session bytes11..323 and retained flight flag1 matched through 720 control waits, 72 clock-pending polls, 72 field ticks, 48 field rounds and 24 field-contact boundaries (936 comparisons). Each contact had side11 owner19, ball cell103 and selected defenders3,7,8. Initially the native transition rejected the retained kickoff keeper-position context. A focused regression failed, then validation and snapshot version checks were corrected to allow that existing context at field contact while preserving the pass flag.

Added guarded phase201 / `--ack-sampled-opponent-contact` for the observed contact-presentation handoff: original/native session bytes11..323, saved kickoff keeper positions and retained flag matched all24 cases at bank26:$8003. The original used 96 cooperative host waits; they are acknowledged, not live NMI/PPU/audio or hardware timing. Focused tests cover preservation, snapshot roundtrip, replay and changed defender rejection. Full Windows CMake build and all 45 test executables passed.

An original-only attempt to continue from $8003 did not reach the anticipated $805A or $8127 within 100,000 instructions; its representative run ended in bank30:$CAD1 after 280 cooperative host waits. This is an observer limit, not proof of which input or event is required. The 24-case attempt was stopped after identifying the diagnostic issue, and no native confrontation outcome is claimed. Next trace the confrontation scheduler, presentation and required defender input from phase201 before implementing action/defense resolution. The scenario, team and prior AI choices remain constructed/captured, not a verified live match; actual initialization, remaining routes, live presentation and a complete match through the final whistle are still unverified. Overall progress is unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-after-pass-contact-20261106.


## 2026-11-06 - Opponent contact menu observer follow-up (newest checkpoint)

A representative phase201 original-only diagnostic from bank26:$8003 entered $803C and its $C600/$D565 menu before reaching attacker preparation $804C. A constructed host trace with a fixed confirm input on every cooperative wait did not reach $805A/$8127 within 1,000,000 instructions (3,369 acknowledged waits); this does not establish the real user-input sequence, menu outcome or a game bug. The prior neutral-input trace reached its 100,000-instruction bound after 280 waits. Next trace the actual field-defense menu/scheduler and its required held/edge input sequence from the three-defender contact, or capture it from a coordinated live original run. Do not infer confrontation outcomes from either bounded attempt. Phase201 and all 24 previous comparisons remain verified as documented above; no further engine code changed and full Windows build/all45 tests passed at the preceding checkpoint. A complete playable match and live initialization are unverified; overall progress is unmeasured. Private evidence: ../Captain-Majid-analysis/area-action4-keeper-result4-restart-20261106/stage0-after-pass-contact-20261106.

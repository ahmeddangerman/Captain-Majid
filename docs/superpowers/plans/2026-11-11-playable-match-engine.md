# Playable Match Engine Implementation Plan

> **For agentic workers:** Use superpowers:executing-plans to implement task-by-task. No delegation without authorization. This supersedes automatic continuation of the opponent21 trace.

**Goal:** Deliver a reusable local match runtime and a playable exhibition match.

**Architecture:** Preserve numerical kernels and historical replay tools. Introduce a separate controller whose states represent gameplay responsibilities, not particular captures. It consumes current state, validated content and typed inputs; it produces state changes, presentation requests and explicit unsupported-capability errors.

**Tech Stack:** C++17, existing CMake/Make, optional SDL2 client, Python evidence tools.

**Spec:** ../../ENGINE_PROGRESS_AUDIT_2026-11-11.md

## Global constraints

Compare new gameplay semantics with original observations. Preserve atomic errors, byte order/arithmetic and old snapshot readers. No private data in Git, engine changes only on work. Never substitute captured outcomes or invented rules to make an acceptance run pass. No overall completion percentage.

## Review focus

- Different legal owners and positions must not require new scenario phases: tasks 1–2.
- Invalid/unaffordable actions preserve state and explain rejection: tasks 1–2.
- Presentation delays must not consume random values unpredictably: task 3.
- Goals, out-of-play and period boundaries must not trap play: task 4.
- Save/resume and repeated input must not duplicate outcomes: task 5.

## Milestones

M1: reusable headless possession cycle. M2: interactive graphical possession cycle. M3: complete local match. M4: repeatable user delivery. These are dependencies, not equal-sized units. Do not promise a complete match in eight hours before measuring M1 blockers.

### Task 1 — Runtime boundary and capability map

Files: create engine/include/captain/match_runtime.hpp, engine/src/match_runtime.cpp, engine/src/match_lab.cpp, engine/tests/match_runtime_tests.cpp and docs/MATCH_CAPABILITIES.md. Modify CMakeLists.txt and Makefile.

Interfaces: define MatchConfig, MatchContent, MatchRuntimeState, MatchInput, MatchEvent, UnsupportedCapability and MatchStepResult in match_runtime.hpp. Expose create_match(const MatchConfig&, const MatchContent&) and step_match(const MatchRuntimeState&, const MatchInput&, const MatchContent&), returning MatchStepResult. Config includes fixture, seed and clock. Result contains state, presentation events and optional unsupported capability. Inputs distinguish player action, tick and presentation completion; there is no captured-result input.

- [ ] Map initialization, control, action choice, contact, flight, possession, restart and period transitions to exact existing functions; classify reusable, scenario-bound, missing or unverified. Record the original evidence needed for missing semantics.
- [ ] Write failing tests for invalid content/configuration, atomic unsupported results and duplicate input. Prove failure, implement the minimal driver, then prove success.
- [ ] Provide a synthetic public unit-test fixture and explicit private local content selection. Do not claim constructed initialization matches original startup.
- [ ] Build captain-match-lab. One command must start a runtime and either advance or report the first named missing capability without invoking phase-session CLI commands.
- [ ] Commit code, tests and capability map together.

Acceptance: executable runtime boundary and a concrete dependency map. No new player-specific phase IDs. This task identifies blockers before another lengthy trace investigation.

### Task 2 — General possession cycle

Files: create engine/src/match_control.cpp, engine/src/match_actions.cpp and engine/tests/match_possession_tests.cpp. Use existing control, pass_target/distance, ball_step/velocity, attack, defender and recovery kernels through the runtime interface.

- [ ] Write failing integration tests for movement → action choice → pass → contact or free flight → receipt/recovery → resumed control, on both sides and at least three legal owner/recipient combinations.
- [ ] Adapt only the required paths, replacing exact scenario guards with semantic preconditions. Preserve old replay tools. Do not rewrite all PhaseSessionState code.
- [ ] Test cancel, unaffordable action, invalid receiver and no valid target atomically. Compare defended/undefended passes and meaningful alternate outcomes against original observations.
- [ ] Connect supported AI helpers. Identify missing category/subtype/recipient rules explicitly and trace those rules with bounded experiments, rather than following a recorded match indefinitely.
- [ ] Repeat possession cycles from varied legal fixtures using normal input and computed decisions, without trace-fed random memory or manual transitions.

Acceptance M1: one API supports repeated possession cycles. An unsupported branch reachable in the chosen fixture remains a blocker; hiding it does not count as autonomous play.

### Task 3 — Timing and interactive client

Files: create engine/src/match_timing.cpp, engine/src/match_view.cpp and engine/tests/match_timing_tests.cpp. Update build targets. Reuse random/travel kernels and SDL initialization patterns, not the research viewer's manual orchestration.

- [ ] Enumerate gameplay timing/random dependencies from task 1. Write seed/input determinism tests and implement original-supported ordering. Missing random-memory semantics must not be replaced silently with an arbitrary PRNG.
- [ ] Compare timing-sensitive original outcomes and label exact/partial/unsupported coverage. Separate cosmetic presentation acknowledgements from gameplay effects.
- [ ] Render a placeholder pitch, players, ball, score, clock and action options from runtime state. Route movement/select/confirm/cancel/pause through MatchInput.
- [ ] Build and visually inspect on Windows; exercise focus loss, repeated confirm and invalid actions. A source check alone does not pass this task.

Acceptance M2: a user launches a window and plays the M1 cycle without a developer operating a CLI. If timing blocks M1, implement its gameplay dependency before graphics. Cosmetic wait reconstruction is not a prerequisite.

### Task 4 — Complete match lifecycle

Files: create engine/src/match_resolution.cpp, engine/src/match_lifecycle.cpp and engine/tests/match_lifecycle_tests.cpp. Adapt existing shot/keeper, goal, kickoff, restart and clock kernels.

- [ ] Write failing tests for shot/keeper outcomes, goals/kickoff, loose ball, boundary restarts, halftime and final whistle. Obtain controlled original evidence for each new rule and alternate result.
- [ ] Connect routing from current state. List unsupported reachable routes as blockers; do not inject predetermined outcomes.
- [ ] Run shortened-clock test fixtures, then the declared match configuration from fresh initialization through final whistle.
- [ ] Observe a graphical human-versus-computer playthrough and a second match without restarting the process. Keep private reproducible seed/input records.

Acceptance M3: complete interactive local match, with goal/restart and final-whistle evidence. Headless tests alone are insufficient.

### Task 5 — Repeatable delivery

Files: create engine/src/match_save.cpp, engine/tests/match_save_tests.cpp, tools/run_match_demo.ps1 and docs/PLAYABLE_DEMO.md; update targets and handoff.

- [ ] Implement versioned runtime saves independently of legacy replay snapshots. Test corruption/truncation, content mismatch, roundtrip and uninterrupted-versus-resumed outcomes.
- [ ] Provide a Windows launcher with explicit private content selection and actionable prerequisite errors. Never copy private inputs into the checkout.
- [ ] Verify launch from a fresh process, user play, reload and second match. Document commands and limitations.
- [ ] Run focused tests during development; full existing and new suites at the coherent milestone. Review and push only work.

Acceptance M4: user launches and plays without developer intervention. Distinguish private local demo, public distributable assets and full-port fidelity.

## First eight hours — work budget, not a completion promise

0–1: capability map and shortest evidence-supported possession path. 1–3: runtime target and failing possession acceptance test. 3–6: connect adapters and resolve the first demonstrated blocker. 6–8: run acceptance, report exact missing rules, and add a visible client only if the path is ready.

An investigation exceeding 45 minutes must produce the exact missing rule, evidence and smallest next experiment. Switch to an independent prerequisite when possible; never invent rules to meet the timebox. Required output is a runnable target, capability map, executable integration test and demonstrated behavior or actionable blocker. Only accumulating new trace checkpoints is a failed work block.

## Anti-repetition policy

Every task must unlock a reusable capability or remove an acceptance blocker. No new production phase for a specific score/player/random value. Exact cases belong in evidence fixtures. Expand observations across meaningful outcomes, not just farther along one recording. Batch routine documentation and commits by coherent capability. Freeze opponent21 stamina-order work unless the capability map proves it blocks this path. The next resume starts task 1.

## Verification and reporting

Use cmake --build build -j 2 and the installed CTest against build, or the existing registered-test runner if CTest is unavailable on PATH. Register all new tests in CTest and appropriate Make checks. Record actual results. Latest prior implementation reports 62 passing checks; this plan adds no engine capability. Report milestone acceptance, user-visible behavior, original comparison coverage, blockers and suggested model. Suggested implementer: GPT-6.1 Terra High.

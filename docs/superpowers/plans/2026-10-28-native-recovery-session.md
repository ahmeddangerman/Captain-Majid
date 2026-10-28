# Native Recovery Session Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement inline continuously, with one fresh whole-plan review.

**Goal:** Advance a terminal loose ball into a persistent recovered owner and supported prepared player-control continuation.
**Architecture:** Split jitter/recipient selection around the existing field advancement; optional recovery context and typed pause phases preserve unresolved routes.
**Tech Stack:** C++17, private Python/py65 observations, production Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-28-native-recovery-session-design.md`

## Global constraints

No original assets/tables/RAM committed. No tests added/run. Keep old content/session readers. Frozen presentation inputs remain explicitly developmental. Do not publish or push this checkpoint.

## Review focus

- Jitter before movement, post-movement recipient positions/random and no duplicate jitter.
- Owner/opponent/status exclusions, preserved coordinate fractions and cleanup of player20 low stamina.
- Correct stage pauses for boundary restart, keeper area and story gate; no unsupported path labeled player control.
- Bounded optional recovery codec, absent context for old snapshots and strict phase/context relationships.
- Atomic command errors, zero defender-count gate, unchanged clock/rewards and isolated cosmetic events.

## Task1: Research and recovery primitives

Files: `tools/trace_ball_recovery_phase.py`, `engine/include/captain/ball_recovery.hpp`, `engine/src/ball_recovery.cpp`.
- [x] Capture original entry/movement/owner-handoff boundaries with a bounded research tool.
- [x] Add `jitter_loose_ball(x,y,e2,e3)` and `recover_stationary_ball(input)`; keep `recover_ball` composing both for existing callers.

## Task2: Persistent session and commands

Files: `phase_session.hpp/.cpp`, new `recovery_session.hpp/.cpp`, `phase_session_io.cpp`, `tools/export_phase_session.py`, `phase_session_lab.cpp`, Make/CMake.
- [x] Add optional six-byte recovery context and typed phases5restart,6handoff,7keeper,8story.
- [x] Implement `advance_session_recovery(input,primary,movement)` and `continue_session_recovery(post_presentation)`; guard phase/context/count and preserve copied state.
- [x] Add bounded296-byte session3 encoding (session2 plus presence+six context bytes), retaining old readers.
- [x] Add `recover` / `continue-recovery` development commands and private export options; build production targets and inspect native/original snapshots.

## Task3: Checkpoint

- [x] Update current status/log and recovery docs, request fresh whole-plan review, address material findings and commit source locally.

## Ledger

Ruling: User previously opted out of routine questions and now requests the next step. Continue inline using existing C++17 architecture. Developer test prohibition overrides skill test steps; evidence remains observations/builds/normal usage, not a passing suite.

Pre-flight: Existing recover_ball consumes one random state for both jitter and recipient selection; E074 sits between these stages in the real caller and shifts E2. Split the stages and pass updated state rather than reuse pre-movement random bytes.

Research correction: Prior BALL_RECOVERY.md called9070 a random-generator call. Static original bytes show special-owner cleanup instead. Correct documentation, preserve supplied staged randomness and do not add an invented random step.

Ruling: Require selected-defender count0 in the new joined recovery operation — the current terminal result2 already clears it and persistent state has no defender list. Cost of assuming a list would be fabricated direct-capture behavior. Existing pure helper keeps its explicit list support.

Task1 complete: Original recovery tool observes8BDF→8C31 or outside-field boundary; prepared continuation observes8C31→C60F/keeper/story gates. Eight primary and seven continuation observations recorded privately. No original calls skipped. Pure helper split preserves the legacy composition.

Task2 complete: Native snapshot hashes agree on the296represented bytes at all15inspected boundaries. Joined rewarded terminal contact→recovery→prepared control gives the same primary/continued hashes as the captured original path. Codec3 retains1/2, explicit absent contexts and padding checks. New pause phases reject contact/movement; wrong-phase recovery leaves native session unchanged. Make production build completed; final Make/CMake builds follow the final guard changes. No automated tests added/run.

Ruling: The original wait/presentation tail is an explicit prepared continuation with frozen inputs, as in the prior clock plan. Cost if mistaken for a live runtime is incorrect timing/state refresh; document the limitation and retain typed boundaries for unsupported routes.

Task3 review: Fresh independent static review found no material implementation findings. Original bytes confirmed geometry/story gates, special-owner cleanup and ball variant. Reviewer identified stale9070 wording in historical docs/tool comment and session2 README wording; these fall within the planned research correction/version documentation and were corrected without changing verifier behavior. No deferred findings. Legacy session1/2 usage preserves absent context and refuses recovery without mutation. Final build evidence/commit pending.

Task3 complete: Final Make/CMake production commands both returned0 with no warning/error lines. Fresh production-binary recovery/continuation output retains the documented original boundary hashes after the final format guards. No automated tests added/run. Save source locally; no public push or private data publication. Next: native player-control setup/actions and the explicit restart/keeper/story continuations; full-match scheduling remains unfinished.

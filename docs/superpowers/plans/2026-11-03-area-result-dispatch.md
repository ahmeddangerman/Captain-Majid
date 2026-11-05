# Area Result Dispatch Implementation Plan

> **For agentic workers:** Execute this plan inline under the user's explicit instruction. Use test-first implementation.

**Goal:** Connect the final opponent-area sequence to the original movement, reward, loose-ball, and direct-possession boundaries.

**Architecture:** Save the last area result with the final prefix timing so results 0/1 remain distinguishable across snapshots. After the verified C606 movement, route result 3 with defense action 2 to direct possession; route result 2 and result 3 with action 4 to the existing staged loose-ball recovery calculation, carrying the selected area actions. Result 3 grants one original selector-2 experience award before either handoff. Result 0/1 stop at a saved attack-continuation boundary.

**Tech Stack:** C++17 phase session, bounded binary snapshots, Make/CMake checks, Python/py65 ROM comparisons.

**Spec:** `docs/NATIVE_AREA_ATTACK_CHOICE.md`, `docs/PLAYER_MOVEMENT_FOUNDATION.md`, `docs/BALL_RECOVERY.md`, and `docs/NATIVE_RECOVERY_SESSION.md`.

## Global Constraints

- Preserve original movement-before-dispatch order, reward selector and action-sensitive routes.
- Preserve old snapshot readers and reject legacy phases that lack the result required for a safe dispatch.
- Keep ROMs, derived packs, RAM, observations, and session files in `../Captain-Majid-analysis`.
- Explicit host frame acknowledgements and frozen input randomness are research assumptions, not live presentation evidence.

## Review Focus

- All-skip prefixes retain the original zero result and enter attack continuation.
- Last nonterminal result 0 versus 1 survives a later skip and snapshot roundtrip.
- Result 2 and result 3/action 4 preserve old possession until recovery chooses the new owner.
- Result 3/action 2 awards once, transfers to the actual defender, and requests sound only when side changes.
- Missing experience or movement content rejects atomically; added-time resume and dispatch cannot repeat timing or rewards.
- Old snapshots remain readable, while old exhausted-prefix snapshots without a saved last result cannot dispatch.

---

### Task 1: Save the final area result and dispatch boundary

**Files:** `engine/include/captain/phase_session.hpp`, `engine/src/phase_session.cpp`, `engine/src/phase_session_io.cpp`, `engine/include/captain/area_ai_session.hpp`, `engine/src/area_ai_session.cpp`, `engine/tests/keeper_save_session_tests.cpp`.

- [ ] Add a failing test for preserving result 0/1 through later skipped defenders and through a new snapshot version.
- [ ] Add a failing test for movement commit exactly once and the saved attack-continuation/recovery boundaries.
- [ ] Implement the new result metadata, format gate, and atomic movement commit using the already verified `advance_field_phase` calculation.
- [ ] Run the focused C++ session tests and confirm old y/z/62/63 snapshots retain their existing read behavior.

### Task 2: Connect original possession and recovery routes

**Files:** `engine/include/captain/area_ai_session.hpp`, `engine/src/area_ai_session.cpp`, `engine/src/recovery_session.cpp`, `engine/src/phase_session_lab.cpp`, `engine/tests/keeper_save_session_tests.cpp`.

- [ ] Add failing tests for result 3/action 2 direct capture and result 2/result 3/action 4 recovery, including one selector-2 reward only for result 3.
- [ ] Implement direct transfer and selected-area recovery with saved phases, staged movement, experience, player coordinates, and sound request.
- [ ] Add CLI save/replay coverage and verify atomic errors and no duplicate clock, reward, or possession writes.

### Task 3: Compare original behavior, verify, and document

**Files:** `tools/check_area_dispatch_original.py`, a focused area-handoff comparison script, `docs/NATIVE_AREA_ATTACK_CHOICE.md`, `docs/WORK_LOG.md`, `docs/PORT_STATUS.md`.

- [ ] Compare all supported results/actions through original recovery or direct-control boundaries using the Japanese ROM and constructed RAM inputs.
- [ ] Run focused checks, full `make -j2 check`, `make -j2 all`, and `git diff --check`.
- [ ] Record match counts, scope, observer assumptions, regressions, and remaining attack-continuation work in the existing progress notes.

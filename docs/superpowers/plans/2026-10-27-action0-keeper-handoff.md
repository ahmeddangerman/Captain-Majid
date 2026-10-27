# Action-0 Keeper Handoff Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Route eligible action-0 field results through the existing keeper and goal/save calculations.

**Architecture:** Add a separate continuation API after field-contact calculations. It receives the final power/subtype and the ROM's route gates, returns early for diverted routes, and reuses the existing keeper/outcome code for a keeper attempt.

**Tech Stack:** C++17, existing `make check` and CMake/CTest.

**Spec:** `docs/superpowers/specs/2026-10-27-action0-keeper-handoff-design.md`

## Global Constraints

- Only action code 0 with field result code 0/1 can use this keeper continuation.
- A nonzero post-loop `$0617`, field-proximity carry, or opposing goalkeeper +10 status prevents the keeper call.
- Power, subtype, keeper inputs, outcome state, and random bytes remain explicit; do not add ROM content data.
- Keep `resolve_explicit_shot` as the existing direct calculation path.

## Review Focus

- Result code 2/3 must never fall through to keeper resolution.
- A nonzero `$0617` count/flag must not produce a keeper result.
- Field-proximity carry must return the loose-ball route before checking keeper status.
- Nonzero opposing goalkeeper +10 status must not produce a keeper result.
- The keeper comparison must use post-contact power/subtype and its result must update goal/save outcome state.

---

### Task 1: Add the gated keeper continuation

**Files:**
- Modify: `engine/include/captain/shot_sequence.hpp`
- Modify: `engine/src/shot_sequence.cpp`
- Test: `engine/tests/shot_sequence_tests.cpp`
- Modify: `docs/SHOT_FLOW_TRACE.md`
- Modify: `docs/ENGINE_PROGRESS.md`

**Interfaces:**
- Consumes the existing `GoalkeeperInput`, `ShotOutcomeInput`, and explicit post-contact power/subtype.
- Produces a continuation route plus optional `GoalkeeperResult` and `ShotOutcomeResult`.

- [x] **Step 1: Write failing tests** for eligible action-0 results producing goal/save/keeper outcomes and each diversion condition returning no keeper/outcome.
- [x] **Step 2: Run `make build/shot-sequence-tests && ./build/shot-sequence-tests`** and confirm the missing API fails to compile.
- [x] **Step 3: Implement the minimal route gate and reuse the existing keeper/outcome calculations.**
- [x] **Step 4: Run the focused test, then `make check`.**
- [x] **Step 5: Run the CMake build/CTest suite and the relevant original-ROM keeper/outcome comparisons.**
- [x] **Step 6: Update the trace and progress notes, inspect the diff, and commit the milestone locally.**

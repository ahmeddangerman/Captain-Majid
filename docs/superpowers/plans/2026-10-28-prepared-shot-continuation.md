# Prepared Shot Continuation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline continuously; one fresh whole-change static review.

**Goal:** Continue a prepared action0 shot through existing contacts, flight, keeper and outcome calculations without charging attacker stamina twice.

**Architecture:** Extract the post-attacker half of `resolve_complete_prepared_shot` into a public pure function. The existing complete-shot API performs attacker preparation once and delegates; a persistent-session caller can supply its already-committed prepared roster and action data to the new function.

**Tech Stack:** C++17, existing Make/CMake production targets.

**Spec:** `docs/superpowers/specs/2026-10-28-prepared-shot-continuation-design.md`

## Global Constraints

- Use caller-supplied defender, keeper, route and random inputs; infer no AI or scheduling.
- Do not alter persistent session formats or claim a completed match in this increment.
- Keep ROMs and extracted content outside Git.
- Do not add or run automated tests under the active developer instruction.

## Review Focus

- The prepared-shot path must not debit the attacker a second time.
- Attacking and keeper subtypes must remain separate through the handoff.
- Terminal field and later-flight branches must continue to skip keeper/outcome work where they do today.
- Existing complete-shot callers must retain their current calculation order and returned results.
- Any new validation must reject an action other than action0 or malformed prepared input before mutating caller state.

### Task 1: Extract post-attacker shot continuation

**Files:**
- Modify: `engine/include/captain/complete_prepared_shot.hpp`
- Modify: `engine/src/complete_prepared_shot.cpp`
- Modify: `docs/COMPLETE_PREPARED_SHOT.md`
- Modify: `docs/WORK_LOG.md`

**Interfaces:**
- Consumes: `PreparedFieldShotInput` with action0, prepared incoming power/subtype, a roster reflecting attacker stamina, explicit continuation gates and caller-selected choices.
- Produces: a public continuation result containing the existing `PreparedFieldShotResult`, optional approach, optional flight, and optional prepared keeper; the existing wrapper still returns `CompletePreparedShotResult`.

**Steps:**
- Define the smallest public continuation input/result types that represent the already-prepared boundary.
- Move the current field/approach/flight/route/keeper/outcome sequence into the new function without changing order.
- Change `resolve_complete_prepared_shot` to prepare the attacker once and delegate using the mutated roster and prepared action values.
- Document that the new API does not commit a phase session or invent decisions/scheduling.
- Run `git diff --check` and the production Make build only; inspect the final diff and record the evidence limits in the work log.

## Decision record

- Ruling: Keep defender and keeper decisions, random values, and scheduling explicit — the project has not yet traced their live decision loop — so this API can be reused without inventing CPU behavior. Cost if wrong: callers still need to supply those values until their original behavior is integrated.
- Ruling: Stop before persistent session commits — score, ball-active, kickoff, terminal recovery, and restart state are not all represented in the current phase snapshot. Cost if wrong: the new function is a calculation boundary rather than a playable action; it avoids partial or lossy session writes.

## Execution evidence

- [x] Defined the post-attacker continuation input/result.
- [x] Moved the existing field/approach/flight/keeper/outcome sequence to the new API.
- [x] Kept `resolve_complete_prepared_shot` as a one-preparation convenience wrapper.
- [x] Documented the continuation boundary and remaining gaps.
- [x] `git diff --check` completed cleanly; `make -j2 all` returned0. Automated tests were not added or run per active instruction.

**Expected result:** Both APIs share one continuation implementation; only the convenience API performs attacker preparation. No automated tests are added or run.

# Ordinary Area Defense Registration Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Register ordinary field-defender choices and finish a no-special defensive selection sequence.

**Architecture:** Extend the existing atomic action-menu operations. Resume phase71 through the original non-special loader branch and its UI tail; retain the full-prefix wait until a subsequent A returns. Unsupported special options and full-prefix direction writes reject explicitly.

**Tech Stack:** C++17, existing Make tests, private Python/py65 ROM observations.

**Spec:** `docs/NATIVE_KEEPER_AREA.md`, plus observed original D3D4–D49F and bank28:8D58.

## Global Constraints

- Overall completion is unmeasured; no complete playable match.
- Source on work; website main separate; private ROM/content/evidence outside Git.
- Never infer a special-move choice or consume stamina in this menu step.
- Atomic errors, old snapshot readers and exact pressed sample must survive.

## Review Focus

- Final cursor equals count: accept confirmation without indexing an absent slot.
- Full-prefix directional samples: reject unrepresented scratch-slot effects atomically.
- Actions0–2: reject instead of skipping possible special menus.
- Back after final registration: return to the last actual selected defender.
- Legacy snapshots: old o cannot encode completed/new full-prefix states.

### Task 1: Register ordinary choices and finish selection

**Files:** action_menu.hpp/cpp, phase_session.hpp/cpp, phase_session_io.cpp,
phase_session_lab.cpp, keeper_save_session_tests.cpp and snapshot version assertions.

**Interfaces:** Consume phase71 and AreaDefenseMenuContent; produce
`ActionMenuResult prepare_keeper_area_defense(const PhaseSessionState&, const AreaDefenseMenuContent&)`.
Extend `poll_keeper_area_defense` for a full-prefix wait and new complete phase72.

- [ ] Add failing focused cases for ordinary registration, next slot, last-slot
  wait, final A, B back, replay and special/full-direction rejection.
- [ ] Run `make -j3 build/keeper-save-session-tests` and the executable; observe failure.
- [ ] Implement actions3–6 ordinary shortcut, subtype0 and sprite, exact pressed
  tail; snapshot51p383bytes; CLI/interactive `prepare-keeper-area-defense`.
- [ ] Run focused tests and full `make -j3 check`/`make -j3 all`; expect success.
- [ ] Commit after original comparison below confirms behavior.

### Task 2: Original comparisons and checkpoint publication

**Files:** trace_area_defense_poll.py, new check_area_defense_registration_original.py,
NATIVE_KEEPER_AREA.md, WORK_LOG.md, tools/README.md.

**Interfaces:** Start original D3D4 with saved selected action/player and pressed;
compare represented snapshots and ordered requests through D3A6. Final A returns
to bank26:8B8F; unsupported special/full-direction routes are negative cases.

- [ ] Extend bounded private observer for preparation and completed-prefix return.
- [ ] Compare actions3–6, slots0–3, field IDs and plain/mixed A/B samples.
- [ ] Exercise a full native multi-defender sequence and save/resume against ROM.
- [ ] Record exact coverage/limits, inspect diff and get one fresh code review.
- [ ] Push verified source to work and publish only the daily Arabic progress note.

## Execution ledger

Pre-flight: Task2 uses the snapshot and operations from Task1; names and383-byte
layout agree. Existing work branch is the authorized engine workspace. User has
explicitly requested continuous execution and already authorized pushes/public notes.

Task1: complete. Stub regression failed before implementation; original comparison
then exposed the missing final overlay clear, whose regression failed before its
fix. Focused tests and full Make check/all pass (defense-register-final logs).
Task2: original proof1,433 boundaries passes; earlier1,024 input cases remain green.
Fresh reviewer found no Critical/Important issues. Suggested minor follow-ups:
legacy o partial70 positive roundtrip and A+direction/A+B+direction preparation
samples. Atomic rejection coverage was already added during the review.
Original observer requires explicit host parent returns D568→8B8F for a resumed
sample; these restore the original wrapper path without skipping instructions.

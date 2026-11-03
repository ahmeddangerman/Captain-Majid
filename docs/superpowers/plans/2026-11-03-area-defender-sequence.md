# Area Defender Sequence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. The user authorized autonomous inline execution and opted out of further approval questions.

**Goal:** Continue the saved opponent area contact through prepared result presentation and the ordered defender sequence.

**Architecture:** Extend the current C++ phase session. Research original post-clock scripts and loop control first, then preserve explicit presentation acknowledgements and a saved cursor. Reuse the existing contact preparation/clock stages for later supported defenders; stop before unimplemented possession/restart dispatch.

**Tech Stack:** C++17, Make, Python/py65 private original-ROM comparison.

**Spec:** `docs/NATIVE_AREA_ATTACK_CHOICE.md`, current `AGENTS.md`, and original bank26 area sequence.

## Global Constraints

- Preserve classic original behavior, atomic failures, old snapshot readers and explicit host presentation boundaries.
- No ROM, patch, derived pack, RAM or observation files in Git. Source goes to public `work`; website `main` receives daily progress notes only.
- No live rendering, full-match or online claim from prepared calculations.

## Review Focus

- Terminal results stop remaining defenders, while nonterminal results advance once.
- Skipped choices5/6 neither charge stamina nor inherit an earlier contact result.
- Save/resume cannot repeat stamina, clock or result presentation.
- Exhausted prefixes and four-defender limits never read a fifth choice.
- Older snapshots cannot encode new phases or sequence cursor metadata.

### Task 1: Observe and implement prepared result continuation

**Files:** `tools/trace_area_defender_sequence.py`, `engine/include/captain/area_ai_session.hpp`, `engine/src/area_ai_session.cpp`, phase session header/validator/IO/lab, `engine/tests/keeper_save_session_tests.cpp`.

- [x] Trace original scripts16/17/18 and next-defender/sequence-end boundaries with explicit frame acknowledgements; record represented mutations privately.
- [x] Add failing native checks for ordered script requests, terminal exit, one/four-defender cursor advancement, roundtrip, replay and atomic rejection.
- [x] Implement the smallest explicit prepared continuation supported by those observations, with a new version gate and preserved old readers.
- [x] Compare full represented snapshots and ordered requests to original cases, including every contact result and both area variants.

### Task 2: Connect later defenders and skipped choices

**Files:** same sequence/session files, `tools/check_area_defender_sequence_original.py`.

- [x] Add checks for second/fourth defender processing, skip-only prefixes and mixed skip/contact order.
- [x] Reuse the existing comparison/clock path for the saved next cursor. Preserve distinct next/end phases and reset original result at the observed boundary.
- [x] Compare complete supported defender chains, including terminal early exit, skipped entries, exhaustion and save/replay.

### Task 3: Verify, document and publish

- [x] Run `make -j3 check`, `make -j3 all`, focused original comparisons and `git diff --check`.
- [x] Request an independent read-only code review; resolve material findings.
- [x] Update `docs/WORK_LOG.md`, detailed reference and status with scope/evidence. Push verified source to `origin work:work`.
- [x] Update the current Arabic daily progress note, preserve design/history/manual navigation, syntax-check and publish on `main`; verify GitHub Pages deployment separately.

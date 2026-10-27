# Banked Attacking Trace Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Execute original attacking preparation/energy/power as one banked routine and compare native output.
**Architecture:** Private PRG memory adapter and strict native content/case adapter, with checkpointed original state.
**Tech Stack:** Python/py65, C++17 test utility.
**Spec:** `docs/superpowers/specs/2026-10-27-banked-attack-trace-design.md`

## Global Constraints

No ROM content in Git; reject unsupported hardware accesses; no skipped calls or full-match claim.

## Review Focus

Bank restoration, RAM mirrors, variable content bounds, caller carry provenance, scratch clobber checkpoints and exact output counts.

### Task1

- [x] Trace real bank switch in an external spike, then add bounded cartridge memory and native research adapter.
- [x] Observe full-corpus comparison mismatch and inspect original multiplier scratch lifetime; capture multiplier at8FDD before multiplication.
- [x] Compare3,882 cases, all14 outputs and expected bank-switch writes; inspect full result.
- [x] Fresh independent review, document evidence/next phase, commit locally.

Independent review reproduced3,882 matches and found unsupported expansion-space accesses4020..5FFF were silently accepted. A synthetic guard regression failed before extending the rejection range; all3 memory tests then passed. These tests also cover internal RAM mirrors, PRG mode/switches and unsupported mapper writes, and are registered in Make/CMake. Final banked comparison rerun retained all3,882 matches.

Final verification: banked trace3,882/3,882; synthetic cartridge3/3; full Make suite; CMake/CTest26/26. Review finding corrected; product engine untouched.

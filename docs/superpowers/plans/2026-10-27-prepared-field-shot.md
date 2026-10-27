# Prepared Field Shot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Connect prepared contacts to field possession and eligible goalkeeper outcomes.
**Architecture:** Pure composition deriving field result, last visited defender, power and ownership from preceding stages. Return detailed phase outputs and updated roster.
**Tech Stack:** C++17, Make/CMake, existing private-ROM component oracles.
**Spec:** `docs/superpowers/specs/2026-10-27-prepared-field-shot-design.md`

## Global Constraints

Caller-supplied content/random data; C++17; immutable inputs; no full gameplay trace claim.

## Review Focus

Defender versus attacker subtype, last visited versus last selected defender/action, stale ownership fixtures, absent defenders, and unreachable malformed phases must have explicit coverage.

### Task1: Joined field-to-keeper handoff

- [x] Declare prepared_field_shot.hpp and write native tests for field capture/animation, recovery, no recovery, power loss into goal/save, catch/punch, subtype isolation, both scoring sides, all continuation diversions, empty list, invalid reached phases and input preservation.
- [x] Observe missing API linkage, implement prepared_field_shot.cpp and register Make/CMake tests.
- [x] Pass native suite and CMake checks; rerun existing outcome/recovery ROM comparisons for component evidence.
- [x] Fresh independent review, update progress/evidence docs, commit locally.

## Execution evidence

Missing wrapper linkage observed before implementation. Initial integration tests passed. Ruling: repair the existing keeper/outcome composition discovered during integration: ROM keeper variation updates$043E while the outcome reads attacking$043C. Added regression failed with `keeper subtype remains separate from attacking subtype`; preserve keeper move subtype and pass attack subtype directly to outcome. This corrects inherited behavior in both direct shots and action0 continuation.

Full Make suite and CMake/CTest 22/22 passed. Component ROM evidence: 384 outcomes, 512 recoveries, 5,248 keeper comparisons, 7,456 action-power cases and 69,632 skill cases. Joined wrapper remains native integration evidence.

Independent review found no blocking correctness issues and reproduced both targeted binaries. Its stale subtype-copy sentence in SHOT_FLOW_TRACE was corrected as documentation consistency work. No source changes followed the green suites.

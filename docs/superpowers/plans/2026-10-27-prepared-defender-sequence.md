# Prepared Defender Sequence Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Connect profile preparation, stamina and ordered contact results.
**Architecture:** Pure22-slot roster transformation; computed preparation and energy override contact fields, with short-circuit evaluation on terminal results.
**Tech Stack:** C++17, Python/py65, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-prepared-defender-sequence-design.md`

## Global Constraints

No bundled ROM content; caller-supplied random/comparison data; C++17; no modification of inputs.

## Review Focus

Record type versus slot, current stamina aliasing profile fields, repeated defenders, unvisited invalid choices, and skipped-side weakening reset must have explicit tests.

### Task1: Prepared ordered field contacts

- [x] Declare API and write failing native tests for depletion/weakening, side skip, record-type surcharge, ordered power/subtypes, early stop, repeated defenders, profile aliases, invalid visited choice and input preservation.
- [x] Observe missing API linkage; implement in prepared_defender_sequence.hpp/.cpp and register Make/CMake tests.
- [x] Run targeted and regression checks. Reuse existing private-ROM preparation/energy comparisons to confirm component contracts; document this increment as native composition evidence rather than a full original-loop comparison.
- [x] Fresh independent review, update progress and commit locally.

## Execution evidence

Missing API linkage observed before implementation. Targeted sequence tests passed. Ruling: profile flag/variant fields derive from current stamina bytes, because original profile and energy routines read the same record locations; independently configured fields would produce inconsistent future actions. The native composition intentionally leaves random/comparison selection and skipped-path CPU carry explicit.

Full Make suite and CMake/CTest 21/21 passed; two additional integration assertions passed targeted Make/CMake rebuilds. Component verifiers: profile/preparation 91,392; energy 9,728; keeper story 6,812; keeper state 5,632; random 3,584. Fresh independent review reported no actionable findings.

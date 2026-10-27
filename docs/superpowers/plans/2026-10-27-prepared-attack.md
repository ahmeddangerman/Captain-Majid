# Prepared Attacking Action Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Connect attacker profile/move lookup to energy and action power.
**Architecture:** Pure attack preparation wrapper over supplied profiles/content and existing energy/power math.
**Tech Stack:** C++17, Make/CMake, Py65 private-ROM oracle.
**Spec:** `docs/superpowers/specs/2026-10-27-prepared-attack-design.md`

## Global Constraints

No bundled ROM data; C++17; caller-supplied randomness; preserve input values.

## Review Focus

Action0 selector alternate rule, subtype low7 boundary, side11 no debit, type32 primary surcharge, action2 no-defender zero-cost, move flag preservation, and weakened power input.

### Task1: Prepared attacker action

- [x] Add native tests for selector rule, base prep, both side energy paths, surcharge, cost override and action power.
- [x] Observe missing API, implement and register Make/CMake.
- [x] Run full Make/CMake suites and existing action-power/energy oracles. Record the attack wrapper as native composition evidence; no complete loader differential harness in this increment.
- [x] Fresh independent review, update progress and commit locally.

## Execution evidence

An initial API-missing check exposed an invalid aggregate initializer; after fixing the fixture, targeted checks passed. Independent ROM review then found two caller-order cases: side11 skips the action2 cost override as well as deduction, and side0 action power uses carry after energy. Regression checks cover both; the power case fixes output at228 for random bytes100/100. The wrapper composes existing profile, energy and action-power functions. The whole `$819D` routine was traced, but no complete differential harness was added; existing component verifiers remain the ROM evidence.

Full Make suite and CMake/CTest23/23 passed after the caller-order fixes. Existing original-ROM comparisons remain green: skill69,632, power7,456, keeper5,248, energy9,728, story6,812, keeper state5,632, random3,584. The complete attacker loader wrapper remains native composition evidence. Independent review confirmed both fixes against `$8F81` and `$8F97–$8F9A`.

Independent review reproduced no remaining correctness findings after both caller-order fixes. Final full Make suite passed; CMake/CTest23/23 passed.

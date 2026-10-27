# Field Interruption Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Model field result4 override, its reset writes and immediate restart route.
**Architecture:** Pure state transform; optional prepared-loop context and distinct result4 handoff, keeping raw comparison output.
**Tech Stack:** C++17, py65, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-field-interruption-design.md`

## Global Constraints

No original content; immutable inputs; immediate handler boundary only; existing fixture defaults unchanged.

## Review Focus

Keeper vs field bypass, inclusive random/coordinate boundaries, preservation of nonkeeper +7, existing result4 no repeated reset, stopped keeper stage and raw/effective result distinction.

### Task1

- [x] Trace8E33,CF4F,9366 and distinguish immediate reset/routing from later restart setup.
- [x] Observe missing override API, implement helper/route/optional loop integration and targeted checks.
- [x] Compare27,904 private original fixtures through actual reset call and initial handler route with no skipped calls.
- [x] Run full suites, fresh review, document limits and commit locally.

## Evidence

Private oracle compares effective code, reset call occurrence, all22 statuses, both keeper penalties, initial restart route and situation; it also asserts nonkeeper record+7 unchanged. Native joined-shot check stops before malformed/unreached keeper data after an interruption and retains earlier stamina debit in returned state.

Full Make suite and CMake/CTest28/28 passed; added raw/effective-code and input-preservation assertions passed targeted Make/CMake builds. Independent review reproduced27,904 private-ROM cases with no actionable findings. No full original loop/late restart setup claim.

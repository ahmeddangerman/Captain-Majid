# Field Comparison Selection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Remove an unknown caller-specific comparison selection rule and verify original classification.
**Architecture:** Pure supplied-content selector/configurator; bounded original CPU oracle executes834C..837F before later handlers.
**Tech Stack:** C++17, Make/CMake, py65.
**Spec:** `docs/superpowers/specs/2026-10-27-field-comparison-selection-design.md`

## Global Constraints

No original tables; immutable inputs; no full-loop/gameplay trace claim.

## Review Focus

Keeper fixed group, eligibility sign bit, byte index wrapping vs doubled offset wrapping, selected vector bounds, zero-width data, caller X/Y reset and later result4 exclusion.

### Task1

- [x] Read original selection/classification instructions; identify later override8E33.
- [x] Observe missing selector API linkage, implement and pass synthetic native tests.
- [x] Compare550 original selection/arithmetic fixtures through837F using private content and no skipped calls.
- [x] Run full suites, fresh review, document remaining loop boundary, commit locally.

## Execution notes

The first oracle used only16 eligibility bytes and rejected defense-action4 fixtures. Original byte indexing may read beyond labelled table bounds; the private oracle now supplies a256-byte eligibility view and128 variation pairs to cover byte-domain accesses. This proves original indexed behavior over constructed fixtures, not the gameplay validity of those combinations. No such content is bundled in product code.

Final evidence: full Make suite; CMake/CTest27/27; private selection/caller arithmetic550/550; existing defender comparison19,200/19,200. Extra zero-width-content assertion passed targeted Make/CMake rebuilds. Independent review found no actionable findings and reproduced the550-case oracle. New content API remains separate from full loop pending result4 and power-loss trace.

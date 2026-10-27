# Field Defender Preparation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans for inline implementation.

**Goal:** Load standard field-defender strength, move effect, and stamina cost from replaceable content and verify the result against the original ROM loader.

**Architecture:** Pure C++17 preparation API plus a contact-calculation wrapper. A strict stateless research command accepts decoded profile/move/curve data. A Python harness privately reads the reference ROM and compares the complete loader outputs.

**Tech Stack:** C++17, Make/CMake, Python/Py65.

**Spec:** `docs/superpowers/specs/2026-10-27-field-defender-preparation-design.md`

## Global Constraints

- Original ROM tables stay outside Git; the core accepts replaceable content.
- Preserve byte wrapping, subtype offset masking, and 10-bit cost masking.
- Support defense actions 0–4, situations 0–2, skill selectors 1–30; reject unsupported content safely.

## Review Focus

- Missing move records must fail without an out-of-bounds read.
- High subtype bits must select the same record as the ROM's wrapped offset.
- Level and move arithmetic must reuse original byte-wrap semantics.
- Strength must come from the selected player's skill and curve entry.
- Preparation must not silently charge stamina or alter ordered fixture state.

### Task 1: Preparation and contact composition

**Files:** create `engine/include/captain/field_defender_preparation.hpp`, `engine/src/field_defender_preparation.cpp`, `engine/tests/field_defender_preparation_tests.cpp`; modify Make/CMake definitions.

**Interfaces:** `prepare_field_defender(input, content) -> FieldDefenderPreparationResult`; `calculate_prepared_field_defender(input, content, contact) -> PreparedFieldDefenderResult`.

- [x] Add failing native tests for selected skill, curve strength, masked cost, high subtype bits, wrapping, validation, and composition.
- [x] Implement the API and observe the targeted checks pass.

### Task 2: Original-ROM evidence and documentation

**Files:** modify research protocol/main; create `tools/verify_field_defender_preparation.py`, `engine/tests/check_preparation_cli.py`, `docs/FIELD_DEFENDER_PREPARATION.md`; update progress/trace/README and build checks.

**Interfaces:** `--field-defender-preparation-batch` and stateless `defenderprep` query; format documented beside the helper.

- [x] Add strict parsing and CLI regression coverage including fixture-state preservation.
- [x] Execute original `$8224` with banks 28/29 and compare selected skill/index, strength, effect, cost, and player pointer over boundary and seeded actual-profile/move cases.
- [x] Run Make checks, CMake/CTest, and the ROM comparison; record bounded coverage and limits.
- [x] Commit, obtain a fresh read-only review, and resolve material findings.

## Execution evidence

- Initial native test failed to link the absent preparation API; implementation then passed the native assertions. The research CLI test initially failed on the unsupported mode and passed after integration.
- `make check` passed, including the new native and CLI checks and existing replica tests.
- CMake build and CTest passed 19/19 checks.
- The complete original `$8224` loader matched 16,064 fixtures over actual normal profiles and move tables. Scope and remaining content/profile decoding limits are recorded in the specification and public engine notes.
- A fresh read-only review found no defects and independently reproduced the native, CLI, and ROM comparison checks. Its set-aside behaviors all match the documented caller/content boundary; no additional scope changes or deferred findings were required.

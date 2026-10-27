# Field Player Profiles Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Feed actual player profile selection into defender move preparation.
**Architecture:** A pure profile resolver and a preparation wrapper use caller-supplied content. Private-ROM differential evidence exercises the original loader.
**Tech Stack:** C++17, Python, py65, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-field-player-profiles-design.md`

## Global Constraints

C++17; no bundled original tables; preserve existing APIs; field IDs 1..10 and 12..21 only.

## Review Focus

Generic own-team IDs, missing team bytes, absent type mapping, missing profile, and alternate types below35 must reject safely. Ordinary types must ignore variant flags. Level carry and subtype masking remain the existing preparation's responsibility.

### Task 1: Resolve and prepare field player profiles

- [x] Write native tests for ordinary, generic position, alternate, variant override, composed strength/cost and invalid references.
- [x] Observe missing API failure, implement `resolve_field_player_profile` and `prepare_profiled_field_defender` in new `field_player_profile.hpp/.cpp`, then pass targeted tests.
- [x] Register native tests in Make/CMake; add a strict private comparison driver in `engine/tests/profile_preparation_driver.cpp`.
- [x] Compare the composed operation to `$8224` using `tools/verify_field_player_profiles.py`, decoding content only in memory.
- [x] Run native regression suite and CMake checks; record evidence and remaining scope in ENGINE_PROGRESS.
- [x] Review diff and commit the increment locally.

## Execution evidence

Missing API linkage reproduced before implementation. Native profile tests passed. The private composed ROM harness matched 91,392 cases. Regression and final review evidence follow below.

Ruling: enumerate all nonzero type bytes in constructed fixtures rather than assert an untraced alternate roster size; these prove table-access behavior, not roster validity. Skill views retain 31 bytes to preserve original cross-row reads for selectors 24..30.

Full Make suite passed; CMake/CTest 20/20 passed. Existing preparation verifier 16,064 cases still passes. Fresh independent review found no actionable findings and reproduced native profile checks and 91,392 composed ROM cases. Documentation updated with remaining import/team-selection/stamina integration scope.

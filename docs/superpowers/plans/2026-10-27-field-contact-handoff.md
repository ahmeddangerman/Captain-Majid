# Field-Contact Handoff Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Compose ordered field contacts with their existing recovery and defender-possession handoffs.

**Architecture:** Add a small `field_handoff` module that calls the existing contact-sequence helper, then sends its final result code and caller-supplied match context to the existing outcome helper. The result contains both stages; player selection and persistent state remain outside this module.

**Tech Stack:** C++17, current Make and CMake/CTest targets.

**Spec:** `docs/superpowers/specs/2026-10-27-field-contact-handoff-design.md`

## Global Constraints

- Run already-selected defender contacts in the supplied order.
- Do not embed ROM data or infer selection, statistics, random bytes, or recovery roster.
- Preserve result 0/1 as `continue_action`; route 2/3 through the existing outcome helper.
- Preserve the fixture `Session` as a contact-loss test fixture.

## Review Focus

- Code 2 must receive the caller's optional recovery state and return the recovered owner/side.
- Code 3 keeper control and field-player control must retain their existing ownership rules.
- Code 3 animation must preserve possession until its later phase resolves.
- Codes 0/1 must remain continuation routes with no handoff state change.
- Empty contact input must dispatch its initial result consistently.

---

### Task 1: Compose contact sequence and result handoff

**Files:**
- Create: `engine/include/captain/field_handoff.hpp`
- Create: `engine/src/field_handoff.cpp`
- Test: `engine/tests/field_handoff_tests.cpp`
- Modify: `Makefile`
- Modify: `CMakeLists.txt`
- Modify: `docs/SHOT_FLOW_TRACE.md`
- Modify: `docs/ENGINE_PROGRESS.md`
- Modify: `docs/ROADMAP.md`

**Interface:**
- `FieldContactHandoffInput` fields are `std::vector<DefenderContactInput> contacts`, `std::uint16_t incoming_power`, `std::uint8_t initial_result_code`, `std::uint8_t possession_side`, `std::uint8_t ball_owner`, `std::uint8_t defender_id`, `std::uint8_t defense_action`, and `std::optional<BallRecoveryInput> recovery`.
- `FieldContactHandoffResult` returns `DefenderSequenceResult field` and `DefenderOutcomeResult outcome`.
- `resolve_field_contact_handoff(const FieldContactHandoffInput&)` returns `FieldContactHandoffResult`; it computes the contact sequence, constructs `DefenderOutcomeInput` from the sequence result plus the caller's handoff context, and calls `resolve_defender_outcome`.

- [x] **Step 1: Write failing tests** for code-2 recovery, code-3 player control, code-3 animation, code-1 continuation, and empty contacts with an initial result.
- [x] **Step 2: Run `make build/field-handoff-tests && ./build/field-handoff-tests`** and confirm the missing API fails to compile.
- [x] **Step 3: Implement the small orchestration module** and add it to Make/CMake builds.
- [x] **Step 4: Run the focused test and `make check`.**
- [x] **Step 5: Run the CMake build/CTest suite and applicable original-ROM defender/recovery comparisons.**
- [x] **Step 6: Update progress/trace notes, inspect the diff, request code review, and commit locally.**

## Review record

Independent review found no critical or important issues. Deferred minor: a composed multi-contact handoff test could add another integration check; ordered multi-contact behavior is already tested in `defender_sequence_tests.cpp` and the wrapper's terminal handoffs are covered here.

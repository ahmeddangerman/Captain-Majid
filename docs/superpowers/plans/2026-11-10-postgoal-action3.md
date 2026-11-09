# Post-goal Action 3 Implementation Plan

**Goal:** Continue the captured opponent decision through its observed action selection and the original-backed preparation calculation.

**Architecture:** Add two narrowly validated saved phases. Reuse the existing serialized target context for the observed recipient and cell, then use the shared attack preparation kernel with the captured input and private content. No autonomous decision or presentation is implied.

**Tech stack:** C++17, existing snapshot reader, native phase-session CLI and private Python original-trace verification.

**Spec:** The latest action3 research entry in `docs/CLI_HANDOFF.md` and `docs/WORK_LOG.md` records the original boundaries and limits.

## Global constraints

Preserve old snapshots and atomic failure. Keep original-derived data and observations outside the checkout. Compare each new phase with captured original RAM before progressing.

## Review focus

- Wrong recipient, target, action, subtype or source state must reject without mutation.
- Missing or inconsistent target context must reject on validation and reload.
- Truncated and malformed snapshots must reject; existing versions remain readable.
- Preparation must use the shared kernel and reject content producing unobserved results.
- Captured choices and event samples must not be described as autonomous AI or presentation.

## Task 1: Save and verify the observed choice and preparation

- [x] Add focused failing tests for selection, preparation, state preservation, rejection and snapshot roundtrip.
- [x] Add guarded phases and target context validation; reuse the existing serialized target layout.
- [x] Expose the transitions to the phase-session CLI and compare both original boundaries from private evidence.
- [x] Run the full build and registered tests, and update the dated work log and handoff with comparison scope.

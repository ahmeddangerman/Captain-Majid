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

## Task 2: Compare the captured action presentation handoff

- [x] Add failing tests for entering the observed presentation wait, ordered ball-mode samples, explicit handoff and invalid/replayed operations.
- [x] Add guarded saved phases for the observed wait and handoff; retain target and preparation context.
- [x] Replay the captured random/travel events and presentation samples, comparing before and after the original bank26:$8027 boundary.
- [x] Run the full build and registered tests, and record the verified boundary and remaining limitations.

## Task 3: Compare the captured action clock and dispatch

- [x] Add failing tests for the original one-tick clock, ordered waits and carrier-cell reset, including invalid or repeated operations.
- [x] Reuse the shared keeper recovery and match-clock arithmetic in guarded saved phases.
- [x] Replay the three captured random/travel pairs and compare the original pre-wait and $C606 boundaries.
- [x] Run the full build and checks and update the dated handoff.

## Task 4: Compare ordered field movement

- [x] Add failing tests for the guarded 22-player movement sequence, malformed ordering and snapshot roundtrip.
- [x] Reuse the existing field-movement kernel for the captured action3 dispatch.
- [x] Compare the original E0DE boundary with captured random/travel and ordered player events.
- [x] Run the full build and checks and document the result.

## Task 5: Compare action3 routing to the flight handler

- [x] Test captured facing, display and motion observations and the guarded flight-handler handoff.
- [x] Retain the existing target and preparation context while applying explicitly observed presentation changes.
- [x] Replay the original event sequence through $C621 and compare the represented state.
- [x] Verify and document the checkpoint before implementing flight.

## Task 6: Compare the first numeric flight setup

- [x] Add failing tests for target recalculation, supplied fractions and stale distance, validation and saved trajectory.
- [x] Reuse the shared direction and velocity kernels for this guarded original route.
- [x] Compare the saved native state and trajectory against the captured $E773 entry.
- [x] Run the full build and registered checks and document coverage and limitations.

## Task 7: Compare the first flight and defender callback

- [x] Test the recorded random/travel waits, cell crossing, empty defender callback and arrival; reject unsupported decisions.
- [x] Reuse shared numeric ball stepping and defender selection while retaining the saved action context.
- [x] Compare each original flight boundary and snapshot roundtrip before handling the recipient exchange.
- [x] Verify and document the completed scope.

## Task 8: Compare the inter-flight exchange

- [x] Test recorded waits, the observed player displacement, owner/recipient exchange and invalid or replayed transitions.
- [x] Implement guarded saved boundaries while preserving original byte arithmetic and low coordinate bytes.
- [x] Compare the original pre-movement, post-movement and exchanged-owner states with supplied timing events.
- [x] Verify and document the checkpoint before preparing the return flight.

## Task 9: Compare return-flight setup

- [x] Test recalculated target, retained fractions and stale distance, direction and velocity, guards and snapshot roundtrip.
- [x] Reuse the shared trajectory calculations and compare the original return-flight entry.
- [x] Verify and document the checkpoint before proceeding to defender callbacks.

## Task 10: Compare return flight up to the defender encounter

- [x] Test the observed cell crossings, defender selection, unsnapped arrival and invalid continuations.
- [x] Reuse shared trajectory stepping and defender selection while preserving original pre-presentation coordinates.
- [x] Compare each original callback and the arrival boundary with captured timing.
- [x] Run all checks and document the evidence and remaining encounter work.

## Task 11: Compare the return-flight arrival snap

- [x] Test the captured unsnapped-to-centered transition, preserved fractions and defender context, replay rejection and snapshot roundtrip.
- [x] Add a guarded boundary matching the original ordering before defender execution.
- [x] Compare native output to original $E7CC, then verify and document the result.

## Task 12: Compare defender8's action3 interception

- [x] Trace the captured defender preparation and comparison, identify the action-specific variation table, and compare the shared arithmetic with controlled original CPU continuations.
- [x] Test the recorded NMI wait and the defender preparation/comparison boundary, including invalid inputs and snapshot roundtrip.
- [x] Connect the profiled preparation and comparison using explicitly supplied, guarded action3 table values.
- [x] Compare the saved native state with the original after comparison, run verification, and document the exact limits.

## Task 13: Compare defender result0 presentation wait

- [x] Trace the ordered original random/travel events and facing samples through $88AD.
- [x] Test and implement a guarded wait retaining the defender result and supplied samples.
- [x] Compare the saved state at $88AD, run verification and document the remaining continuation.

## Task 14: Compare defender result0 clock and cleanup

- [x] Trace original status, clock, NMI ordering and selected-defender cleanup.
- [x] Test and connect the existing contact clock and guarded list cleanup, including snapshots.
- [x] Compare against original boundaries, verify and document the next continuation.

# Native Control Setup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans inline, with one fresh final static review.

**Goal:** Turn persistent player-control handoffs into a represented control-ready state for either possession side.
**Architecture:** Reuse nearest-player search, emit cosmetic requests, copy and validate session state; optional control context and backward-readable snapshot4.
**Tech Stack:** C++17, private bounded Python/py65 observations, production Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-28-native-control-setup-design.md`

## Global constraints

No tests added/run. No original assets/data/RAM committed or pushed. No public website edits. Preserve old snapshot readers and existing pure helper behavior. Live input/AI/frame scheduling remains outside setup scope.

## Review focus

- Both possession sides, carrier/ball mismatch and invalid keeper/outside-field carriers.
- Nearest side0 defender search after clearing opponent, status skips, exact strict radius/scan ordering.
- Special-owner cleanup, coordinate fractions, script/ball flags and represented0469 high-byte update without corrupting other experience.
- Control context codec bounds, absent old context, phase9 requirements and copied-state atomic errors.
- Ordered presentation requests that never advance simulation, explicit handoff/control-ready distinction.

## Task1: Observe and expose primitive

- [x] Extend bounded original observation tool with exclusive control-setup mode and E145 endpoint.
- [x] Expose nearest available field-player selection from existing recovery helper without changing its callers.

## Task2: Persistent setup

- [x] Add optional seven-byte ControlContext and phase9, with bounded snapshot4/export (304bytes), retaining versions1–3.
- [x] Implement `prepare_session_control(input)` in a focused control_session module; require captured experience/recovery/control context and a supported field owner.
- [x] Add `setup-control` lab command, production build wiring and normal saved-state usage against original observations.

## Task3: Checkpoint

- [x] Update status/log/docs, fresh whole-plan static review, final production builds and local commit.

## Ledger

Ruling: Continue inline without routine design approval under existing user authorization. Developer prohibition on tests overrides skill TDD steps; record observations, normal usage, builds and static review separately.

Pre-flight: Existing phase1 is a handler-entry handoff, not an initialized controller. New phase9 distinguishes completed setup. RecoveryContext already carries0442/044C/044E, and ExperienceState slot10 already represents0468/0469; update those fields through their existing owners rather than duplicate storage.

Ruling: Require all three optional contexts for setup — old snapshots may load but cannot supply fields they never recorded. Cost of inventing them would be silent loss of preserved controller/experience state.

Task1 complete: Seven private original setup observations support both sides, unavailable-player selection, side normalization, stale ball/count/control values, special cleanup and field edges. Nearest search is extracted with the same radius/scan/exclusion arithmetic; existing recovery composition still calls it for both teams.

Task2 complete: All seven native/original304-byte saved states have matching inspected hashes. Presentation IDs/order agree with inspected original records, including the pre/post normalized side. Make production build completed. No tests added/run; CMake build, joined-session usage and static review follow.

Task3 complete: Make/CMake production builds returned0 with no warning/error lines. Joined rewarded contact→recovery→prepared control→setup retains the original base control-ready hash. A repeated setup and legacy session3 without control context are rejected without session mutation. One fresh independent static review found no material findings; original ROM bytes corroborate both branches, nearest search, cleanup, coordinate refresh and0469 write. Reviewer executed no code/builds/tests. No deferred findings. Save source locally, no public push. Next: logical button/decision handling and carrier movement before joining the live clock/frame loop.

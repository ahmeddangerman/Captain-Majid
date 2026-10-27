# Native Contact Clock Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement task-by-task continuously.

**Goal:** Derive contact status/elapsed, advance clock/keeper recovery and continue coherent terminal phases.
**Architecture:** Pure match-clock events, copied contact/session state, typed pending continuations, bounded private session IO and native development commands.
**Tech Stack:** C++17, Python/py65, Make/CMake production builds.
**Spec:** `docs/superpowers/specs/2026-10-27-native-contact-clock-design.md`

## Global Constraints

No experience/NMI inference. Clock events defer unknown presentation/period transitions. Private data only. Old formats retained. No tests added/run. Continue without routine user approval.

## Review Focus

Zero/main-clock versus overtime branches; wrapped high-byte sign and low-byte warning; keeper recovery before clock diversion; code2 placement before clock and count-clear afterward; code3 transfer after ready; pending event/resume guards; exact bounded session codec and old pack compatibility.

## Task 1: Clock/status rules

- [x] Capture original clock/request/period boundaries and contact prelude inputs.
- [x] Implement pure match-clock state/events and contact elapsed/status data/helper.
- [x] Add version5 private contact data with readers1..4retained; build/inspect production usage.

## Task 2: Persistent session

- [x] Implement phase-session terminal/prelude ordering and explicit pending added-time continuation.
- [x] Add bounded session state IO/export and native persistent lab commands.
- [x] Capture coherent original terminal/contact entry/exit states and inspect normal native session results.

## Task 3: Checkpoint

- [x] Document evidence/limits, request whole-plan fresh review, address material findings, commit locally and continue to next phase.

## Ledger

Ruling: Execute inline under standing authorization; active developer test constraint overrides skill test steps. Keep all evidence classified as original observation/production usage/static review, not a passing automated suite.

Pre-flight: Existing field state omits match-clock/shared-recovery fields, so use a parent session snapshot while retaining the239-byte field format. Prepared terminal functions expect post-prelude state; the new wrapper supplies it only when the clock continuation is ready.

Ruling: Original code2 places the ball before its contact-clock call, while code3 transfers ownership after that call. Preserve this distinction even on period-end/added-time diversions; do not force both into the same generic handoff order.

Task1: complete. Original nine-case clock observation used the existing cooperative task/frame/display acknowledgements. EF7F is a clock/display request, not audio stop. Numeric native usage agrees on represented outputs. Make/CMake production builds completed, tests disabled/not run.

Task2: complete. Session264-byte format includes the existing239-byte field section, clock/shared recovery and pending phase data. Four original/native exports have identical inspected SHA-256 values; see NATIVE_CONTACT_CLOCK.md. Interactive usage retains state, rejects paused movement/contact and exposes explicit frozen-input resume. Initial private capture accidentally included the32768-byte cartridge backing store; exporting only its first2048internal-RAM bytes corrected the capture boundary, not simulation logic.

Ruling: Use a deliberately frozen-input resume command for development only — the original presentation wait can refresh NMI/controller/random state, so runtime callers must supply post-wait state; cost if mistaken is incorrect live sequencing. Document this limitation, retain the pending boundary, and do not claim live timing equivalence.

Task3: documentation and production evidence recorded; fresh whole-plan review pending. Overall port percentage remains unmeasured.

Task3: complete. Fresh independent static review on Astra High found no material findings after checking all increment files, codec dependencies, private observation reports and original contact/clock bytes. No code executed or tests run by reviewer. Make/CMake production builds and normal session usage recorded above. Continue inline into rewards/recovery research; no public push or website changes.

# Keeper Recovery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Recover explicit keeper state over elapsed game time and make it usable in the shared shot scene.
**Architecture:** Pure state helper, opt-in pre-shot scene context, original caller observations in the existing tracer.
**Tech Stack:** C++17, Python/py65, existing Make targets.
**Spec:** `docs/superpowers/specs/2026-10-27-keeper-recovery-design.md`

## Global Constraints

Private ROM/data only. No tests added/run. Default scene unchanged. No invented elapsed/random scheduling or persistent match claims.

## Review Focus

Wrapped sign-bit decisions; no penalty recovery during a stun step; exactly one penalty reduction per call; shared flags clear on negative timer only; identical state supplied to comparison/outcome/status gate.

## Task 1: Original behavior and trace

- [x] Record isolated original state observations privately.
- [x] Extend original loop tracer with recovery entry/exit records and capture a continuous caller observation.

## Task 2: Native helper and development scene

- [x] Add `KeeperRecoveryState`, `KeeperRecoverySnapshot`, and `advance_keeper_recovery` in the existing keeper-state API/source.
- [x] Add scene state and explicit ordered elapsed steps, wire comparison/outcome/status consistently, and expose CLI options with strict byte parsing.
- [x] Build production targets and exercise normal CLI usage; document evidence and limits.

## Task 3: Checkpoint

- [x] Review source against the original observations and spec, record independent-review availability honestly, update work log and commit locally.

## Ledger

Ruling: Continue inline under the user's standing authorization; no routine approval gate. The active developer constraint overrides skill test steps, so evidence is limited to original research, production builds, normal usage and review.

Pre-flight: Task1 observes the exact keeper+5/+6/+7/+10 fields consumed by Task2. Scene time is an explicit pre-shot step, while approach distance remains the separate keeper comparison input. No interface conflicts found.

Ruling: Keep the optional scene state separate from the existing prepared-shot API. Callers can already supply recovered comparison/outcome/gate values; adding another scheduling layer there would imply unsupported original timing. The cost is manual timing until the full match clock caller is reconstructed.

Ruling: Original continuous tracing shows four recovery calls with elapsed1,1,2,1 in the constructed two-defender shot. Preserve an ordered list of explicit scene steps, rather than merging them into a total, because stun precedence and single penalty reductions make the operations non-additive. Each shot replays from initial state; this is not a persistent match.

Evidence: Seven isolated original D235 executions and two continuous original caller traces recorded privately. Production `make -j2 all` completed successfully. Normal CLI usage showed penalty29/duration1 after1,1,2,1, a still-stunned keeper diversion, and a recovered keeper comparison. No automated tests added/run.

Final review: Fresh independent read-only review covered the committed approach increment and current recovery changes, including static original-routine inspection; no actionable correctness finding reported. No tests/builds/original executions were run by the reviewer. Full scheduling/outcome equivalence remains unestablished.

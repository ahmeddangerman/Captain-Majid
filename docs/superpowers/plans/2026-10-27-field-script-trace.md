# Field Script Trace Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Record bounded original parser execution and gameplay writes around field scripts.
**Architecture:** Reuse ReferenceCartridge with observed writes and explicit snapshot state; run original parser calls and expose omitted work.
**Tech Stack:** Python3.11, py65.
**Spec:** `docs/superpowers/specs/2026-10-27-field-script-trace-design.md`

## Global Constraints

Private ROM/snapshots; no source/content copying; no hardware emulation or silent skipped calls; immutable input files; no tests added/run.

## Review Focus

Host writes incorrectly attributed to original execution; uninitialized cartridge memory; stack/scratch omissions; limits/termination; overclaiming parser observations as full-loop evidence.

## Task 1

- [x] Inspect original selector/parser returns and script termination/presentation boundaries.
- [x] Implement observed memory and bounded CLI trace.
- [x] Capture a constructed post-comparison snapshot privately and inspect script traces as original-code research.
- [x] Document observations and unresolved effects, obtain fresh review and commit locally.

## Ledger

Pre-flight: existing guarded memory adapter supplies banked PRG and rejects hardware; tracer must strengthen missing cartridge RAM handling and log its own host writes separately.

Ruling: No tests added/run, per higher-priority developer instruction. Original-code research execution is the deliverable; it is not a native-engine behavioral comparison.

Ruling: Use explicit parser-only boundaries instead of emulating missing hardware. This keeps observations attributable but cannot settle omitted presentation/frame writes or full-loop power behavior.

Evidence: Four original parser observations completed from two constructed loader/energy/comparison snapshots. Scripts6/7 for result1 and5/7 for result3 left watched power/result bytes unchanged; instruction/event counts are in FIELD_LOOP_SCHEDULING.md. A separate attempt to continue the original clock helper reached its scheduler frame wait, then stopped at the instruction bound. No complete-loop success claimed.

Final review: One Important finding, inconsistent mapper-mode entry for snapshots with `$22` bit6 set. Fixed by explicit rejection before execution and consistent mapper select/shadow initialization. Existing observations use mode0. No other actionable findings. Per developer instruction, the guard has static source/disassembly review evidence without a behavioral regression test or TDD claim.

Final: Ruling: Omitted presentation scratch resets and scheduler/frame effects remain outside this parser-only entry. They are explicitly disclosed as excluded context. Cost if wrong: parser requests may differ from real gameplay; this tool must not establish a full-loop rule on its own.

Task 1: complete. Original CLI trace ran to parser termination with one presentation request,608instructions and no watched writes from the result3/script7 snapshot. After review correction, a fresh original CLI observation is recorded separately. No engine behavior changed, no tests/suites added or run, no push/publish performed.

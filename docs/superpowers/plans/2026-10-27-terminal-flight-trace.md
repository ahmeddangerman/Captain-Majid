# Terminal Flight Trace Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Observe original terminal later-flight handoffs before the next phase executes.
**Architecture:** Extend existing tracer with witnessed terminal boundaries and separate state records.
**Tech Stack:** Python/py65; existing native laboratory for normal pending-route inspection.
**Spec:** `docs/superpowers/specs/2026-10-27-terminal-flight-trace-design.md`

## Global Constraints

No original calls skipped; no tests added/run; private snapshots/reports; no native terminal-handler claim.

## Review Focus

Require a later-flight witness; use fixed-bank/bank26 boundaries correctly; retain initial terminal behavior; keep keeper outcome absent; report contact entry and actual handoff states separately.

## Task 1

- [x] Add terminal witness/boundaries and contact-position/handoff records.
- [x] Capture original result2/3 observations with normalized private snapshots.
- [x] Document next native contract, review, update log and commit locally.

## Ledger

Ruling: Continue inline under standing authorization; no tests added/run under the active developer constraint. This is original-code research, not a native behavioral implementation.

Pre-flight: Existing original outcome records require `passed_keeper`; new terminal flight records must remain separate and work for both keeper/outcome requested endpoints. Initial field-terminal dispatch still returns before any later-flight witness exists.

Evidence: Two continuous original side11 observations reach player-control/recovery boundaries after220,746/207,872instructions and260host frames each. Both have no keeper/outcome. A separate original keeper-mode observation reaches the same witnessed player-control boundary after220,746instructions. Normal native level5 usage reports its first result2 as pending, with no keeper debit. No tests added/run; this increment changes no native implementation.

Final review: Fresh independent static review found no actionable findings and inspected both private outcome-mode reports. Initial field logic, witness/bank gating, ID guard, separate snapshots and disclosed pending native state were reviewed. Reviewer did not execute code or establish broader side/action coverage. Production native build evidence remains the prior unchanged-source checkpoint.

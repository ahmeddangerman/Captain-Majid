# Shot-to-Keeper Trace Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Observe one continuous original attacking-to-keeper calculation with bounded scheduler support.
**Architecture:** Extend the original loop observer with named entry/exit modes and stage checkpoints, sharing existing host/frame setup.
**Tech Stack:** Python3.11, py65.
**Spec:** `docs/superpowers/specs/2026-10-27-shot-to-keeper-trace-design.md`

## Global Constraints

Private ROM/snapshots only; guarded hardware; disclose modeled host acknowledgements; preserve existing observer; no tests added/run.

## Review Focus

Stage checkpoints under wrong bank; unsupported pre-loop action combinations; field terminal/diverted exits; random bytes changed by original gameplay; output boundaries overstated as goals/saves.

## Task 1

- [x] Identify valid pre-loop field choices and reach original keeper comparison from the attacker entry.
- [x] Add keeper trace mode and stage-specific observed state.
- [x] Capture further original cases and document evidence/limits.
- [x] Fresh review, diff check and local commit.

## Ledger

Ruling: Preserve the field-only observer's wider constructed-action support; restrict complete shot mode to observed field actions2/3. Original script2 indexes unsupported data for the constructed field action0 used in the earlier isolated-loop experiments. Those earlier loop observations remain valid at their explicit entry, but do not establish full gameplay legality.

Ruling: Stop keeper mode before story/outcome execution; a separate attempt beyond comparison reached the instruction bound. No success claim for a goal/save handler.

Pre-flight: Existing field-mode entry, limit and output keys remain available. New keeper mode must accept no-defender input and identify nonkeeper routes instead of waiting forever for an unreachable keeper.

Evidence: Four distinct side0 original traces reach856B, spanning keeper classifications0/2/3 and zero/two field contacts. Side11 and later outcome-script attempts reach instruction bounds. No native differential comparisons or automated tests were run. Private reports only in sibling analysis directory.

Final review: No blocking findings; static ROM inspection confirms checkpoint banks/addresses, field/diverted exits and raw keeper boundary. All four stored reports agree with documented values. `git diff --check` clean.

Final: Ruling: Unobserved action3 routes, side11 completion, later runtime/frame effects and native equivalence remain unjudged. Keep the claims limited to the four stored original comparison observations. Cost if wrong: a broader gameplay claim would be unsupported; this trace is not a playable match or complete goal/save execution.

Task 1: complete. Local source/docs only; no tests, native comparison suite, push or publication.

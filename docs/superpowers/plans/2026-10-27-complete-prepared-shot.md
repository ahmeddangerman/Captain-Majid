# Fully Prepared Shot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Run preparation and shot outcomes over one roster.
**Architecture:** Reuse field-only handoff and route selection; defer keeper preparation until its route is reached. Existing low-level APIs retain behavior.
**Tech Stack:** C++17, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-complete-prepared-shot-design.md`

## Global Constraints

Supplied content/randomness; immutable input; action0 only; no whole-ROM-loop claim.

## Review Focus

Each side's energy rules, stale power/subtype/ownership fields, keeper action in outcome, unreached invalid keeper, attacker slot validity, exceptions after earlier debits.

### Task1

- [x] Write failing joined-shot native tests for side0/11, goal/catch/punch, terminal/diverted keeper bypass, validation and atomic input preservation.
- [x] Extract reusable field-only and routing APIs, implement complete_prepared_shot.hpp/.cpp, register Make/CMake.
- [x] Run full suites and existing component oracles; fresh independent review; document scope and commit locally.

## Execution evidence

Missing joined API linkage observed before implementation. Targeted tests passed after composition. A subsequent punch-coordinate regression failed until keeper-comparison random bytes were connected to outcome random bytes; it now passes. Existing field and continuation APIs were retained through shared phase/route extraction. Full Make suite and CMake25/25 passed. An added field-power-loss/rebound assertion passed targeted builds afterward. Private component oracles matched384 outcomes,512 recoveries,69,632 skills,7,456 powers and5,248 keeper comparisons. Fresh independent review found no actionable correctness findings.

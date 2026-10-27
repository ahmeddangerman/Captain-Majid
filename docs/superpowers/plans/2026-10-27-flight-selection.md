# Flight Selection Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Select a later-flight burst from player position/status snapshots.
**Architecture:** Extend existing defender-selection API with cell encoding and a pure cell/threshold selector; reuse it in scene assembly.
**Tech Stack:** C++17, Python/py65, Make/CMake production targets.
**Spec:** `docs/superpowers/specs/2026-10-27-flight-selection-design.md`

## Global Constraints

Empty initial list; explicit frozen random bytes; no movement or scheduling; no tests added/run; original observations private.

## Review Focus

Cell boundaries and255sentinel; opponent IDs and5/4caps; nonzero status skips; subtraction borrow1/wrap and strict threshold; existing scene defaults and IDs preserved.

## Task 1: Research and selection

- [x] Record original selector observations including caps, status/cell and borrow boundaries.
- [x] Implement `field_cell_at` and `select_flight_defenders` using existing position records.

## Task 2: Scene and checkpoint

- [x] Build the scene's explicit layout, select candidates, and retain current burst arithmetic.
- [x] Build production targets and inspect normal crowded/controlled usage.
- [x] Document evidence/limits, review, update log and commit locally.

## Ledger

Ruling: Execute inline under standing user authorization. Developer no-test constraint overrides skill test steps. Original observations and production usage are limited evidence, not a native differential suite.

Pre-flight: Existing selection players order is IDs1..10 then12..21. New selection reuses this layout and the same side caps. The scene must select the same slots as the existing explicit burst while leaving its roster/staged comparison randomness unchanged.

Evidence: Six original87E1/CDE2 observations returned without skipped calls, recorded privately. Make and CMake production builds completed with no warning/error lines, tests disabled/not run. Normal usage retained crowded catch power21/ratio22 and controlled goal power224/kickoff4. No automated native differential suite was added/run.

Final review: Fresh independent read-only review found no actionable defects. Static original inspection supports cell bounds, caps, borrow and sentinel decisions; six private observations support documented counts. Scene alternate placement at the last cell and default zero-burst behavior were reviewed. Reviewer ran no tests/builds/CPU executions; broad native equivalence remains unestablished.

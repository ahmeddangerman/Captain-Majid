# Shot Approach Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Derive original shot distance, target cell and loose-ball diversion in the prepared calculation.
**Architecture:** Pure geometry/content helper, optional lazy complete-shot context, backwards-compatible pack reader with new exporter version.
**Tech Stack:** C++17, Python3.11/py65, existing Make/CMake targets.
**Spec:** `docs/superpowers/specs/2026-10-27-shot-approach-design.md`

## Global Constraints

Private content only; preserve absent-context/version1 behavior; explicit staged randomness; original byte arithmetic; no tests added/run.

## Review Focus

Two different side mirrors; multiplier carry and wrap; outer-zero fallthrough/missing inner entry; terminal/gate lazy evaluation; version1/2 format drift or conflating approach and keeper random state.

## Task 1

- [x] Inspect original distance/target/cell routines and final flight center writes.
- [x] Record private original observations and correct keeper snapshot role aliases.
- [x] Implement helper, optional prepared connection and pack version2.
- [x] Build production targets and run normal new-pack laboratory usage.
- [x] Request fresh review, document its incomplete status, finish author review and commit locally.

## Ledger

Ruling: Keep keeper random inputs separate from approach randomness because intervening choice/movement may consume random state. The helper reports its updated byte; it does not invent a general schedule.

Ruling: Preserve both existing version1 pack loading and absent-context native behavior. New data opts into geometry; unsupported table indices reject rather than copying unlabeled source instruction bytes as gameplay data.

Pre-flight: New optional input/result fields trail existing aggregates. Exporter appends exactly50geometry bytes in version2; reader still accepts exact version1 layouts. Shared scene uses new geometry only when present.

Evidence: Eight original DD81/DD47/CDC9 observations recorded privately. Four corrected keeper snapshots normalize keeper role bytes and retain prior instruction/frame/comparison outputs, now reporting final ball(204,124). Make production targets compile. Normal version2 native usage reports distance45/target204,124/goal and distance90/target196,124/loose-ball without a keeper. No automated tests or native differential suite run.

Ruling: The fresh reviewer stopped at a model usage limit after partial static inspection and no finding. Continue local development with an explicitly incomplete independent review; do not claim a clean final review or publish on this evidence. Author review checked the optional route gate, actual rolled subtype, separate random stages and appended pack layout. The cost is reduced independent confidence until review and behavioral checks can be completed.

Resumed evidence: `make -j2 all` completed successfully on the resumed runtime. Normal laboratory usage loaded version1 (power33/keeper79/punch), version2 (level80/no defenders: power224/distance45/goal), and a version2 ball-X120 loose route (distance90/no keeper). No automated tests were added or run. The earlier optional SDL2 build remains historical evidence; its temporary SDK is absent in the resumed runtime.

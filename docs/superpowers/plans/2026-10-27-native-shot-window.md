# Native Shot Window Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Give the existing native shot laboratory an optional graphical development view.
**Architecture:** Shared scene calculation with separate CLI and SDL2 presenters; optional dependency and software capture path.
**Tech Stack:** C++17, SDL2, Make/CMake core, optional CMake view.
**Spec:** `docs/superpowers/specs/2026-10-27-native-shot-window-design.md`

## Global Constraints

No original assets/source vendored; existing fixture assumptions explicit; rendering does not advance simulation; default builds need no SDL2; no tests added/run.

## Review Focus

Scene drift between CLI/view; key repeat/stale displayed result; rendering timing mutating simulation; SDL lifetime/failure cleanup; capture overwriting local data or becoming a mandatory dependency.

## Task 1

- [x] Extract shared resettable scene API and keep CLI behavior.
- [x] Implement optional SDL2 view/capture with placeholder presentation.
- [x] Build production targets and create a normal-use capture; document limits.
- [x] Fresh review and local commit.

## Ledger

Ruling: Choose SDL2 as an optional portable native presentation dependency. The cloud SDK uses dummy/software rendering under `/tmp`; default project builds do not acquire a new runtime requirement. Cost: real GPU/window behavior still needs platform execution.

Ruling: Use placeholder graphics and an invented simple pixel font. Original asset importer/rendering is unfinished, so displaying those as an authentic game scene would mislead.

Pre-flight: Extract the existing scene constructor without changing numerical inputs; both presenters consume the same returned result. Animation consumes only a stored result and a display clock.

Evidence: Optional CMake production build with local SDL2 SDK built all three executables, warnings enabled; default Make built both non-SDL executables. Normal software capture drew640×480BMP outside Git, inspected via PNG conversion. Shared CLI normal use still reports power33/keeper79/ratio80/punch. No automated tests or real window/GPU/keyboard execution.

Final review: No actionable findings. Static review confirms shared scene inputs, ignored key repeats, clearing stale results, presentation-only timing, SDL cleanup order, exclusive capture creation and optional dependency.

Final: Ruling: Real keyboard/window/GPU behavior, Windows portability and runtime failure paths remain unjudged. Preserve those limits; the software capture is evidence of drawing, not interactive/platform/frame-rate equivalence. Cost if wrong: a real window/backend may expose behavior absent from the cloud capture.

Task 1: complete. Final optional scene build succeeded, source/diff checks clean. No tests, original asset publication, SDK vendoring or pushes.

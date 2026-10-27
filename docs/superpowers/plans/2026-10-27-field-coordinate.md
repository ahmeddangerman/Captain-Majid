# Field Coordinate Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Preserve original fractional movement and edge-clamp arithmetic in a native coordinate helper.
**Architecture:** Small pure API/source, existing build integration and stateless research query.
**Tech Stack:** C++17, Python/py65, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-field-coordinate-design.md`

## Global Constraints

Raw16-bit arithmetic, high-byte-only clamp, no CPU-flag contract or full movement claim. No tests added/run. Original observations private.

## Review Focus

Word wrap before clamp; fractional low byte retained at boundaries; X/Y range distinction; strict enum/query bounds; no ordered-session mutation.

## Task 1

- [x] Record original boundary/wrap observations and implement helper/query/build integration.
- [x] Build production targets and inspect normal native research usage.
- [x] Document movement dependencies/limits, review, update log and commit locally.

## Ledger

Ruling: Execute inline under standing authorization; no tests added/run under the active developer constraint. This is a coordinate arithmetic component, not completed player movement.

Pre-flight: E912 writes the fractional byte before clamping the integer byte. A conventional saturation of the whole16-bit value would be incorrect. The query must accept two full words and a strict0/1axis, using existing parser utilities.

Evidence: Six isolated original observations recorded privately. Make/CMake production builds completed with no warning/error lines; no tests added/run. Normal batch queries returned12543,20735,12288 for the documented edge/wrap samples. A replica query returned12543 between identical state snapshots without advancing sequence0.

Final review: Fresh independent static review found no actionable findings in arithmetic, axis/query bounds, build integration or stateless routing. It inspected the private observations but ran no tests/builds/queries/ROM code; broad native equivalence and full movement remain unestablished.

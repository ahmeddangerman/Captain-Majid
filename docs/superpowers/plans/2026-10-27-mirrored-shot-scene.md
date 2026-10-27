# Mirrored Shot Scene Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Exercise the existing native calculation from both sides and capture original catch/live-flight boundaries.
**Architecture:** Parameterize shared scene slots/context, preserve explicit coordinates, and extend original observation metadata.
**Tech Stack:** C++17, Python/py65, existing Make targets.
**Spec:** `docs/superpowers/specs/2026-10-27-mirrored-shot-scene-design.md`

## Global Constraints

Side0 defaults retained; coordinates explicit; no invented frame/random/flight scheduling; no tests added/run; all original data private.

## Review Focus

Mirrored owner/keeper/defender slots; side-specific energy rules; recovery applied to the selected keeper; custom button masks accurately disclosed and bounded; later-flight power loss not confused with the initial no-loss caller.

## Task 1: Observation inputs

- [x] Add fixed button mask and separate flight-contact records to the existing tracer.
- [x] Record controlled/crowded side11 catch observations with normalized keeper role fields.

## Task 2: Shared native scene

- [x] Add side option, derive all slots and update CLI inputs/stamina output.
- [x] Build production targets and inspect normal mirrored catch and side0 usage.

## Task 3: Checkpoint

- [x] Document observations, review implementation, update log and commit locally.

## Ledger

Ruling: Continue inline under standing authorization. Developer test constraint overrides skill test steps. Keep new evidence labeled as original observations/normal usage, not a native differential suite.

Pre-flight: Side selection changes attacker/keeper/selected-defender IDs throughout scene assembly, recovered keeper lookup and printed stamina together. Approach uses the existing validated side byte and explicit coordinate inputs. The original catch reports must disclose129 rather than the previous hard-coded A-only metadata.

Ruling: A crowded original side11 snapshot triggered four additional flight contacts and reduced power33→21. A controlled snapshot moves opposing field players away and retains power33, isolating the catch already supported by native APIs. Record both; do not alter native power with an unexplained constant or equate the controlled scene with full flight simulation. The cost is explicit unsupported crowded-flight behavior until that caller is ported.

Ruling: Version1's scene fallback has fixed side0 route inputs, so a new side11 scene requires version2 geometry instead of inheriting misleading literals. Existing default version1 use remains available; the cost is requiring a new private export for mirrored scenes.

Evidence: Controlled/crowded original observations retain238,636/717,165instructions and350/1,142host frames with correctly disclosed129mask. The crowded report records four result1 contacts with loss3 each. `make -j2 all` completed successfully. Normal CLI usage inspected the mirrored catch (power33/ratio35, attacker1000/keeper980), side0 recovery (adjustment29/score109), and default version1 use. No automated tests added/run.

Final review: Fresh independent read-only review found no actionable findings; source, private reports and static original disassembly support mirrored slots, mask metadata, flight checkpoints and disclosed limits. No tests/builds/tracer executions were run by the reviewer. Complete flight scheduling and broad native combinations remain unverified.

# Defender Contact Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Extend the native core from contact subtraction to the original field-defender strength, comparison, and classification calculation.

**Architecture:** A pure C++17 function accepts simulation inputs and content parameters and returns explicit outputs. A private ROM harness supplies actual content tables and original CPU execution; terminal batch and local replica modes expose the same function.

**Tech Stack:** Existing C++17 core, Make/CMake definitions, Python/py65 comparison tools.

**Spec:** `docs/DEFENDER_CONTACT.md`, within `docs/ENGINE_DIRECTION.md`.

## Constraints and review focus

- Independent implementation; no third-party source or bundled ROM tables/assets.
- Preserve original 16-bit overflow, byte selection, threshold equality, zero-strength division, and special/weak branch precedence.
- Reject zero ranges and invalid threshold ordering before calculating.
- No claims of a full shot, goalkeeper result, or complete game-state reconstruction.
- Existing ordered session fixture remains available; this increment exposes a stateless defender calculation over the same local TCP harness.

## Tasks

- [x] Add the pure input/output API and failing native checks for ordinary, weak, special, equality, overflow, and invalid-input cases.
- [x] Implement the documented arithmetic and strict 15-field batch parsing; observe checks pass.
- [x] Add a boundary-rich ROM comparison, executing both original subroutines and the conditional loss stage. Compare every exposed output and resolve any mismatches from ROM evidence.
- [x] Extend replica commands and the TCP proof with defender calculations, including rejection that preserves the existing fixture state.
- [x] Review, run the full checks, update progress, and commit locally.

## Execution decisions

The user authorized continuation. Work on the existing `work` branch. Input random bytes are explicit until original random scheduling is studied. Costs of this boundary: isolated routine fidelity does not yet establish fidelity of complete gameplay or how human input timing affects outcomes.

## Completion evidence

`make all check` passes on Linux/GCC. The new original-ROM comparison matches 19,200 cases across all eight exposed outputs, including explicit incoming-power wrap boundaries added after independent review. The previous 6,656-case arithmetic comparison also remains green. The local TCP proof compares normal, weakened, special, and invalid defender inputs while preserving ordered-fixture state.

Independent review: no critical or important findings; one minor corpus-coverage gap fixed. No deferred findings. Windows and CMake execution remain unverified.

Ruling: accept coefficients, thresholds, and random bytes as explicit inputs and expose this calculation as a stateless research command until gameplay callers and random scheduling are mapped. This avoids claiming unverified full-match behavior; it costs another integration step before a playable shot.

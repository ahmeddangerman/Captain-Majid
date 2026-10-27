# Native Field Phase Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Continue between tasks without routine user approval.

**Goal:** Integrate original player movement, target decisions and terminal flight transitions into a persistent native development phase.
**Architecture:** Pure geometry/player API, replaceable runtime-profile/tactic content, typed field state and explicit phase transitions; bounded private snapshot export/loading and native lab.
**Tech Stack:** C++17, Python/py65, existing Make/CMake targets; optional presentation remains separate.
**Spec:** `docs/superpowers/specs/2026-10-27-native-field-phase-design.md`

## Global Constraints

Private original content only. Keepers and field positions have different roles. No tests added/run. Old pack/context behavior retained. Zero elapsed is a caller-level no-op. No invented full match/random schedule or overall percentage.

## Review Focus

Direction quadrant/equality and zero denominator; wrapped byte sign/counter behavior; target240 versus ordinary cell mirroring; secondary profile/AI metadata and level independence; per-player RNG order/owner/keeper skips; native terminal transitions require coherent post-processing state; old/new content and bounded state parser compatibility.

## Task 1: Geometry and player movement

Files: `player_movement.hpp/.cpp`, private original observations, focused research documentation.

- [x] Derive angle quantization, direction signs, target-cell stop and elapsed/status decisions from original routines.
- [x] Implement movement content/state/results and `advance_field_player`, reusing coordinate and cell helpers.
- [x] Build production targets, inspect native usage and commit the component with limits recorded.

## Task 2: Speed/AI content and team advancement

Files: focused movement-profile/target modules, pack exporter/reader, field-phase state module.

- [x] Derive secondary profile speeds, AI thresholds, template bounds and mirroring; record original observations.
- [x] Add version4 private content with old readers retained; implement target and speed loading.
- [x] Advance the field roster in original order with ownership skips and explicit elapsed/random state; build and commit.

## Task 3: Persistent native phase and terminal transitions

Files: field state loader/exporter, terminal flight integration and native phase laboratory, build/docs/log updates.

- [x] Capture coherent original phase entry/exit snapshots and define bounded typed field state serialization.
- [x] Implement persistent advance commands and supported terminal later-flight transitions with stage assumptions explicit.
- [x] Build production targets and inspect native original-snapshot usage; document remaining reward/clock/match boundaries.
- [x] Request one fresh independent review of the entire plan range, address material findings, update log and commit the checkpoint.

## Ledger

Ruling: Execute inline under the user's standing authorization and latest explicit instruction to continue task-to-task. Developer test constraint overrides skill test steps. Production/original research evidence must never be called a passing automated suite.

Pre-flight: Task1 consumes high-byte cell coordinates and raw16-bit axis updates; Task2 supplies speed words and targets, keeping profile and field/keeper roles separate. Task3 must stage terminal code3 after its preceding handler effects, because the original presentation/clock/status work can change input state before `$E074`.

Ruling: Retire the old7% in current status; retain historical log entries under an explicit retirement notice. Progress is capabilities/checkpoints until a defensible completion measurement exists.

Ruling: Continue on the existing isolated `work` branch with local source commits. Do not push original assets/content or publish the website; this request concerns the engine. User approval gates for routine implementation are waived; irreversible external actions remain outside this batch.

Task1 evidence: Eight original E854 observations recorded privately. Native production queries agree on the six inspected state/direction/iteration outputs, including129elapsed no-motion behavior. `make -j2 all` completed with no diagnostics. No tests added/run; one fresh whole-plan review remains scheduled after integration.

Task1 ruling: Zero elapsed follows E074's caller bypass instead of exposing raw E854's byte-counter underflow. This gives a coherent zero-step API; direct raw-entry zero behavior is outside its contract. Nonzero original observations retain exact reached arithmetic.

Task2 evidence: Six original A103 observations execute actual profile-bank calls and cover both sides/formations/tactics/RNG. Version4 exporter produces18,580bytes, including6,747new bytes. Native team update from the captured pre-E074 state matches all239represented serialized bytes after the original phase, observed through production CLI/capture/hash output. No automated tests added/run.

Task3 ruling: The first recovery-tail example used88BE before later clock/status prelude processing and disagreed on those fields. Use the actual post-prelude81C3checkpoint; native output then matches the original represented recovery state. Keep prelude ownership explicit instead of forcing elapsed/status constants. The cost is that full original branch processing still needs implementation before automatic shot-to-phase commits.

Task3 ruling: The ordinary native player iteration models captured interrupt-free inputs; real E074 scheduler yields can receive NMI/audio/controller updates between actors. Do not describe the239-byte agreement as hardware/full-game equivalence. Future scheduling must supply that boundary explicitly without tying simulation to presentation mods.

Final evidence: Make/CMake production targets build without warning/error lines, including the new field laboratory; tests disabled/not run. Normal CLI/capture use matches the represented original239byte states for player advancement and prepared control/recovery tails. Interactive advance2 then advance1 retains state (player2 X23339→25611→26747) rather than resetting.

Final review: One fresh independent read-only review of the entire plan found no material implementation defect. It confirmed original arithmetic/index/carry/role semantics, content compatibility gates, snapshot layout and calculate-before-commit behavior. Its current-summary documentation finding was addressed as part of the requested progress-log maintenance. Reviewer did not execute builds/tests; broad input equivalence, Windows/filesystem failure behavior and live scheduling remain unestablished.

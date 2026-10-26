# First Engine Kernel Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [x]`) syntax for tracking.

**Goal:** Run an independently written native action-strength kernel, verify it against the Japanese ROM, and prove ordered replay between two local processes.

**Architecture:** A portable C++17 library implements the confirmed arithmetic and a small deterministic session. A terminal replica accepts ordered commands; a Python development harness exchanges commands over loopback TCP with two replicas. This is an intermediate checkpoint toward the authentic shot interaction in the engine direction.

**Tech Stack:** C++17 standard library, Make, optional CMake, Python 3; py65 for private ROM comparisons only.

**Spec:** `docs/ENGINE_DIRECTION.md`; factual arithmetic contract in `docs/ACTION_STRENGTH.md`.

## Global Constraints

- Write our implementation independently; do not import third-party code, comments, translations, graphics, or audio.
- Keep ROMs and generated comparison data outside Git.
- Windows and Linux are targets; report actual platform coverage.
- No invented shot or goal formula: implement only the verified contact-loss arithmetic.
- Keep participant IDs separate from teams; validate commands before mutating state.
- No public server or online release in this checkpoint.

## Review Focus

- Preserve the original sign-bit clamp even for out-of-range synthetic inputs.
- Rejected/repeated/out-of-order commands must leave state unchanged.
- Serialization must include configuration and participants, not raw memory/padding.
- Invalid terminal input must not accidentally execute a command.
- Disconnected replicas and slow socket reads must not hang the development harness indefinitely.

## Task 1: Reference-backed action arithmetic

Files: `engine/include/captain/action_strength.hpp`, `engine/src/action_strength.cpp`, `engine/tests/core_tests.cpp`, `tools/verify_action_strength.py`, `docs/ACTION_STRENGTH.md`, `Makefile`, `CMakeLists.txt`.

Interface: `std::uint16_t reduce_action_strength(std::uint16_t power, std::uint8_t loss)`.

- [x] Record ROM entry `$819F`, exit `$81B8`, mapping banks 26/27, and input/output RAM addresses. No assembly copied into the project.
- [x] Write executable assertions for ordinary loss, underflow, byte borrow, and sign-bit boundaries; run and observe the missing implementation failure.
- [x] Implement the independently derived arithmetic contract.
- [x] Run unit assertions and compare a boundary-rich corpus against actual ROM instructions under py65; fail on any discrepancy.

## Task 2: Ordered session and native terminal replica

Files: `engine/include/captain/session.hpp`, `engine/src/session.cpp`, `engine/src/main.cpp`, extend `engine/tests/core_tests.cpp`.

Interfaces: `Session::apply_contact(participant_id, sequence)`, `Session::snapshot()`. The fixture starts at power 300, loss 37, participants 7/team 0 and 42/team 1, with team 0 authorized to advance the fixture. Inputs are developer commands, not player-selected shot outcomes.

- [x] Write tests for unauthorized/unknown participants, duplicate/future sequence numbers, replay equality, and distinct configuration snapshots; observe failure.
- [x] Implement validation before mutation, fixed-order serialization, and a bounded command sequence.
- [x] Implement strict line-based `contact <participant> <sequence>` / `state` / `quit` commands. Malformed input returns an error and preserves state.
- [x] Run the complete test executable and malformed-input checks.

## Task 3: Local network proof and usable instructions

Files: `tools/check_replica_sync.py`, `docs/ENGINE_PROGRESS.md`, update `README.md`, `docs/ROADMAP.md`, `.gitignore`.

- [x] Send the same command stream through two loopback TCP connections to separate native processes. Include a delayed replica, duplicate command, future command, unknown participant, and disconnect cleanup. Set timeouts on sockets and process waits.
- [x] Require matching full snapshots after every command, expected accept/reject replies, and final power/sequence values. Run with the native build.
- [x] Document that Python supplies the TCP development adapter; the C++ engine itself has no transport, matchmaking, graphical game, or completed shot yet.
- [x] Review the diff, run the complete checks once, and record evidence and limitations.

## Execution decisions

The user explicitly authorized continuing. Execute in the existing `work` branch, retaining their pending engine-direction edits. No merge, push, site publication, or worktree creation is needed. This plan covers the first foundation increment, not the full game roadmap.

## Completion record

All three tasks are complete for this bounded increment. `make all check` passes on Linux/GCC; the independent ROM execution comparison passes 6,656 cases. A separate reviewer found no critical or important issues and three minor issues; all three were fixed, including regression checks for setup failure cleanup and slow protocol input.

Decision: keep work on the existing `work` branch and use Make/GCC because CMake is absent in this environment. CMake definitions and Windows portability remain unverified. Cost: Windows-specific build/runtime issues may still need fixing on a Windows runner.

This increment deliberately stops at the verified contact-loss kernel; authentic shot resolution remains the next gameplay research/implementation checkpoint.

# Prepared Keeper Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Connect keeper profiles, selected moves, stamina and comparison.
**Architecture:** Shared type/profile lookup preserves field behavior; dedicated keeper content maps8-byte skill rows and4 actions to existing energy/keeper arithmetic.
**Tech Stack:** C++17, Make/CMake, private-ROM instruction analysis.
**Spec:** `docs/superpowers/specs/2026-10-27-prepared-keeper-design.md`

## Global Constraints

No original tables; caller content/randomness; immutable inputs; C++17.

## Review Focus

Generic keeper11 vs keeper0, primary vs effective type, slot/side mismatch, selector underflow, alternate stamina aliases, side energy wrapper and action3 selector mapping.

### Task1

- [x] Write keeper preparation tests and observe missing API.
- [x] Extract shared profile reference resolver, implement keeper preparation/comparison and register Make/CMake.
- [x] Run full native/CMake suites and existing profile/energy/keeper ROM checks.
- [x] Fresh review, update evidence and remaining scope, commit locally.

## Execution evidence

Missing API linkage observed before implementation; native keeper tests passed after implementation. Shared reference lookup preserves prior field slot restrictions. Keeper selectors are supplied after action-specific table-index decoding; the core bundles no tables. New preparation/comparison has native composition evidence, not a complete original keeper-loader differential run.

Full Make suite passed and CMake/CTest24/24 passed. Existing component ROM evidence:91,392 profile/preparation;69,632 skill;7,456 action power;5,248 keeper;9,728 energy;6,812 story;5,632 keeper state;3,584 random. Independent review found no actionable correctness findings. New wrapper remains native composition evidence.

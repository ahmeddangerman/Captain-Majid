# Local Shot Content Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Run a native shot calculation using private replaceable content loaded from a local pack.
**Architecture:** Offline verified-ROM exporter, strict C++17 content reader, standalone command-line laboratory using existing engine APIs.
**Tech Stack:** Python3.11/py65, C++17, Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-27-local-shot-content-design.md`

## Global Constraints

No original data committed; versioned bounded parser; immutable core content; explicit development scene; no full-match claims; no tests added/run.

## Review Focus

Wrong bank/table bounds; unbounded or truncated pack counts; shared profile mapping/stamina aliases; fixture assumptions presented as gameplay; generated original data entering Git.

## Task 1

- [x] Define pack and implement exporter/reader with protected local output.
- [x] Build the native laboratory and wire Make/CMake production targets.
- [x] Export private content and run normal laboratory usage; document behavior and limits.
- [x] Fresh review, fix important findings, commit locally.

## Ledger

Ruling: The user authorized ongoing work without step approvals. Choose a command-line development laboratory; do not create/change the public webpage or present a full playable match.

Ruling: Use an explicit observed side0 fixture with per-stage random bytes/distance/gates. A generic gameplay random schedule or team roster would be a guess; the scene must identify those limits. Cost: outcomes are useful for development but not proof of arbitrary match fidelity.

Ruling: No automated tests added/run under the developer instruction. Normal exporter and laboratory usage is the requested development deliverable, not a regression suite.

Pre-flight: Exporter order and C++ reader must match exactly; profile mappings copied to both existing field/keeper APIs; empty generic team types fail on use rather than inventing team data. Lab uses ordinary types and explicit outcome state.

Evidence: Linux `make -j2 all` completed without diagnostics. Separate CMake production configure/build with BUILD_TESTING=OFF built both executables. Exporter produced11,767bytes outside Git. CLI default scene loaded and calculated power33/keeper79/ratio80/punch; interactive usage fired the default shot and a level80/no-defender goal with power224, then exited. No automated tests added/run.

Normal usage finding: keeper variation pointer selector3 was initially indexed from8D95 instead of pointer-table base8D93. Direct original disassembly identified the off-by-one pointer. Exporter corrected, only its newly generated private pack replaced, and normal usage recorded the expected corresponding scene values. This was not a TDD or regression-suite run.

Final review: No actionable findings. Static review checked pack ordering/count bounds, original table bases/strides/selector mapping, shared profile copies, explicit scene limits and Git exclusions. Corrected keeper pointer confirmed by original disassembly.

Final: Ruling: Malformed-pack runtime behavior, Windows execution, arbitrary shot fidelity and full outcome-script equivalence remain unjudged. The reader has bounded source-level validation, Linux production builds and normal usage evidence only. Cost if wrong: unsupported platforms/inputs or changed scenes may expose issues not covered here; no broader claim is made.

Task 1: complete. `git diff --check` clean; source/docs and ignore/build definitions committed locally. Private data/generated pack remains outside Git. No automated tests, push or publication performed.

# Shot Power and Keeper Implementation Plan

> **For agentic workers:** Use superpowers:executing-plans for inline implementation.

**Goal:** Reconstruct standard skill index adjustment, randomized action power, and the pre-override goalkeeper comparison.

**Architecture:** Pure C++17 calculations, shared defender variation and threshold classification, strict research command parsing, and original-ROM comparison through py65. No original data tables bundled.

**Spec:** `docs/SHOT_POWER_AND_KEEPER.md`, under the agreed engine direction.

## Constraints and review focus

- No third-party source, comments, or game assets copied into this repository.
- Preserve both byte and word wrap, carry inputs, strict critical boundaries, zero divisors, state adjustment saturation, and weak/special precedence.
- Reuse the confirmed defender roll; retain the complete existing ROM comparison as regression evidence.
- Treat input random bytes, move coefficients, and keeper state as explicit until their gameplay callers are mapped.
- Do not claim final goals/saves, full player-stat selection, or completed shot gameplay.

## Tasks

- [x] Record source contracts and add failing native checks for skill adjustment, action carry/critical/weakening, and keeper comparison/state boundaries.
- [x] Extract shared defender variation/classification; implement new pure calculations without changing existing defender behavior.
- [x] Add strict batch and stateless replica research modes, with malformed input/state preservation checks.
- [x] Execute original skill adjustment and action/keeper routines against native outputs, covering boundary and seeded cases; retain the existing defender and loss comparisons.
- [x] Extend loopback comparisons; obtain one independent code review, resolve findings, update progress, and commit locally.

## Execution decision

Continue on `work` with user authorization. Implement index adjustment rather than copying player stat tables or claiming an unverified complete content loader. This requires a later content-selection integration step before a full authentic shot.

## Completion record

All tasks completed on Linux/GCC. Native checks, strict research CLI checks, and the loopback experiment pass. Original-ROM comparisons: 69,632 standard skill-index cases, 7,456 action-power cases, 5,248 keeper cases; the existing 19,200 defender and 6,656 contact-loss comparisons remain valid. A separate reviewer reproduced native checks and the new ROM comparisons and found no actionable issues.

Ruling: keep original random bytes and the action routine's incoming carry explicit. Static evidence shows that frame interrupts and sound-driver memory influence the original generator; a full gameplay trace must inform deterministic scheduling. Cost: a complete faithful shot still needs energy/content selection and scheduling integration.

No deferred review findings. Windows and CMake execution remain unverified.

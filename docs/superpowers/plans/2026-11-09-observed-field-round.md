# Evidence-fed field round implementation plan

> **For agentic workers:** Use superpowers:executing-plans to implement this plan. Steps use checkbox syntax.

**Goal:** Compare a field round against explicitly captured frame inputs without changing the existing batch path.

**Architecture:** Add a distinct 22-sample API and CLI file format. Reuse the existing field-round loop, injecting only caller-supplied random and optional travel events immediately before each poll; never infer their timing. Keep session serialization unchanged.

**Tech Stack:** C++17, Python 3, CMake/CTest, private original-emulator evidence.

**Spec:** docs/superpowers/specs/2026-11-09-observed-field-round.md

## Global constraints

Keep original content and captures outside Git. Preserve atomic failures and old snapshot readers. Do not call captured events a live scheduler.

## Review focus

- Truncated, extra or invalid sample records fail before producing a capture.
- An early menu uses only prior samples and keeps the existing stop boundary.
- Zero travel events preserve old arithmetic; explicit events preserve byte wrap.
- Random samples are explicit and never inferred from player counts.
- Existing batch output and phase317 replay remain unchanged.

### Task 1: Implement and verify the bounded replay

**Files:** `engine/include/captain/control_tick.hpp`, `engine/src/control_tick.cpp`, `engine/src/phase_session_lab.cpp`, focused tests, documentation.

- [x] Add a failing synthetic test for captured random/travel events and unchanged legacy batch behavior.
- [x] Add the explicit sample API, strict file parser and CLI operation; do not change old callers.
- [x] Compare all represented fields at the existing 22-poll original capture, documenting any remaining differences.
- [x] Run relevant tests, the full registered suite and existing possession replay; update the handoff and work log.
- [x] Commit and push only verified source and documentation.

Ruling: use the established shared work checkout and preserve the pre-existing untracked user folder; this continues the authorized branch without resetting or relocating existing work.

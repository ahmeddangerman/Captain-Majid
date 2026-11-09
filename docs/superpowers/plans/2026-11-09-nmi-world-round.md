# NMI-input field round implementation plan

> **For agentic workers:** Use superpowers:executing-plans for this task.

**Goal:** Calculate the field-round random bytes from explicitly observed NMI inputs.

**Architecture:** Extend the existing shared field-round loop with a distinct NMI-input sample type and return the final index separately. Preserve all older APIs and snapshot readers. Add a strict private event file and original-trace verifier.

**Tech Stack:** C++17, Python 3, CMake/CTest, private emulator traces.

**Spec:** docs/superpowers/specs/2026-11-09-nmi-world-round.md

## Global constraints

No private game data in Git. No automatic frame, sound or presentation claims. All input errors remain atomic.

## Review focus

- The random index wraps at255 and is returned even on early menu exit.
- A target-selection random update feeds the next supplied NMI step.
- Each incoming carry and RAM sample is explicit; no implicit sound state is invented.
- Malformed or incomplete event files are rejected before writing a session.
- Existing field-round and phase317 replay behavior remains unchanged.

### Task 1: Implement and verify the NMI-input mode

- [x] Add a failing test covering random progression, index wrap and early exit.
- [x] Implement the separate mode and strict input parser without changing snapshot layouts.
- [x] Compare with the four existing original traces, including final index and session fields.
- [x] Run the original arithmetic oracle, registered tests and existing possession replay; update the handoff.
- [x] Commit and push verified source and documentation only.

Ruling: retain the random index outside the existing snapshot until a separately verified persistent frame-state format exists; this preserves compatibility but requires callers to retain the printed index.

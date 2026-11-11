# Runtime Development Window Implementation Plan

> **For agentic workers:** Use superpowers:executing-plans inline. The user authorized continued implementation without approval interruptions.

**Goal:** Let a developer exercise supported provisional runtime paths in a Windows window using a private development snapshot and real keyboard input.

**Architecture:** A platform-neutral input adapter chooses the next existing `MatchInput` from the runtime phase, without supplying outcomes or gameplay rules. A Windows-only native window renders current runtime state, forwards keyboard input, and visibly pauses on unsupported capabilities. It loads existing private content and snapshots; no new public assets or reconstructed startup are implied.

**Tech Stack:** C++17, Win32/GDI, CMake and existing runtime/content readers.

**Spec:** `docs/superpowers/plans/2026-11-11-playable-match-engine.md` task 3; this window is a limited development tool and does not assert M2 acceptance.

## Global constraints

- Keep provisional timing clearly labeled; use actual computed runtime states and existing original-backed kernels.
- Do not embed ROMs, packs, snapshots or derived assets; start only from explicitly supplied private input.
- Stop visibly and preserve the state on unsupported transitions. Do not assume unknown snapshot metadata.
- Retain Linux builds and original-backed mode unchanged.

## Review focus

- Keyboard repeat, focus loss and simultaneous inputs must not produce stuck or duplicate confirm presses.
- Presentation acknowledgements must only occur at existing runtime boundaries and must not imply original NMI/audio/PPU execution.
- Pass and shot choices must come from the confirmed menu state, never an injected outcome.
- Unsupported or missing content must pause the window without advancing its sequence or concealing the reason.
- Resize, close, malformed input and optional metadata must be handled cleanly without overwriting private files.

### Task 1: Runtime input adapter

**Files:** Create `engine/include/captain/match_client.hpp`, `engine/src/match_client.cpp`; modify CMake/Make; test in `engine/tests/match_client_tests.cpp`.

**Interfaces:** `MatchInput client_input(const MatchRuntimeState&, std::uint8_t held, std::uint8_t pressed)` creates the next sequence input, routes supported presentation stages and confirmed shot/pass; it rejects an exhausted sequence. It never advances the runtime itself.

- [x] Write focused tests for movement/menu inputs, every acknowledged shot/pass/goal/defense stage, normal tick, confirmed actions and sequence exhaustion. Run and observe failure.
- [x] Implement and register the adapter with existing CMake and Make builds; run focused tests and verify supported runtime smoke paths.

### Task 2: Windows development window

**Files:** Create `engine/src/match_window_win32.cpp`, update CMake, and document private input launch in `docs/PLAYABLE_DEMO.md`.

**Interfaces:** `captain-match-window PACK SNAPSHOT --distance FILE --velocity FILE [--seed N] [--ball-fractions X Y] [--retained-defense-action N]` loads the same validated content and `MatchConfig` as the lab; timer events call `client_input` then `step_match`, and the window renders the authoritative `match_ball_view` and player state.

- [x] Implement a fixed-ratio resizable pitch, player positions, visible selected menu/target and score/clock when known. Mark the window as provisional and explain controls.
- [x] On unsupported step, preserve the last valid state and show the capability; pause/resume and focus loss must clear held buttons. Never run simulation while paused.
- [ ] Build on Windows and test a real private development snapshot through keyboard input and supported transitions; visually inspect the actual window. The process launches and off-screen rendering was inspected, but the computer-use helper is unavailable, so live interaction remains unverified.
- [ ] Run the existing registered checks, relevant adapter tests and Linux syntax/build checks. Update dated handoff/status/log with exact coverage and unresolved M1/M2/M3 blockers, then commit and push `work`.

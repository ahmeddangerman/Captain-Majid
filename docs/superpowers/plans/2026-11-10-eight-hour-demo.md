# Eight-Hour Local Demo Implementation Plan

> **For agentic workers:** Use superpowers:executing-plans to implement this plan task by task. This document schedules work; creating it does not execute that work.

**Goal:** Deliver a launchable, repeatable Windows interaction demo and, where original evidence permits, one bounded continuation toward a persistent match.

**Architecture:** Start with the existing optional SDL shot window and existing verified core calculations. Keep its resettable scenario separate from the persistent phase session. Extend the persistent path only where controlled original observations establish the transition; recorded timing and choices remain explicitly identified.

**Tech Stack:** C++17, CMake, optional SDL2 2.0.18 or later, existing Python research tools in the private environment.

**Spec:** User instructions in `AGENTS.md`, `docs/CLI_HANDOFF.md`, `docs/PORT_STATUS.md`, and latest `docs/WORK_LOG.md` entries. Supersedes the seven-hour plan dated 2026-11-10.

## Starting evidence

- Engine checkpoint: `b6984e8` on `work`. The recorded native/original path matches through the second post-return field round: clock154, owner18, random index144. This is evidence-fed execution; autonomous timing, AI and a complete match remain unsupported.
- Latest logged verification: Windows build and 62 registered checks passed. These are previous results, to be rerun after implementation.
- `engine/src/shot_scene_view.cpp` already contains a window, keyboard handling, placeholder graphics, outcome display and reset. Space shoots; arrows change attacker/keeper levels; D changes defender count; R clears the result. Actual Windows window operation needs verification.
- `calculate_shot_scene(const ShotContent&, const ShotSceneOptions&)` in `engine/src/shot_scene.cpp` uses explicit development inputs, including fixed random values. Each shot resets. It cannot establish persistent match play or universal original parity.

## Global constraints

- Preserve atomic errors, original byte arithmetic/order and snapshot readers.
- Compare every new gameplay behavior with controlled original ROM observations; record matches, differences and partial coverage before building further on it.
- ROMs, derived packs, RAM, observations and sessions stay in `../Captain-Majid-analysis`; no private assets in commits or public demo packages.
- Use existing original-backed behavior. Do not import external disassembly code or invent AI/timing rules to meet the deadline.
- Engine commits and authorized pushes go to `work` only. Leave `Custom Tools from Ahmed/` untouched.
- Website progress notes use the separate public-main checkout; publication verification is separate from engine verification.
- Overall percentage is unmeasured. A complete playable match remains the larger milestone.

## Eight-hour schedule

| Elapsed time | Work | Required checkpoint |
| --- | --- | --- |
| 0:00-1:00 | Build and launch the existing graphical scene | Actual window/input evidence, or exact launch blocker |
| 1:00-2:00 | Make the local launch repeatable and document controls | One documented command, valid private pack, clear missing-pack error |
| 2:00-4:00 | Stabilize one bounded interactive scenario | User input, calculated result, visible outcome, reset and repeat |
| 4:00-6:00 | Connect one evidence-backed match continuation if feasible | Verified continuation boundary, or a stable scenario plus precise blocker |
| 6:00-7:00 | Freeze scope, verify and repair | Full registered checks plus actual window walkthrough |
| 7:00-8:00 | Package, document and hand off | Reproducible local demo, public-safe source, dated status and pushed work |

## Task 1: Prove the graphical starting point (hour 1)

**Inspect:** `CMakeLists.txt`, `engine/src/shot_scene_view.cpp`, `engine/src/shot_scene.cpp`, `engine/include/captain/shot_scene.hpp`.

- [ ] Record current branch, dirty files, available compiler and SDL2 package location. Preserve the existing build configuration.
- [ ] Configure an isolated demo build directory with `cmake -S . -B build-demo -DCAPTAIN_BUILD_SDL_SCENE=ON`; supply the locally discovered SDL2 package path if necessary. Confirm the build directory is excluded from version control.
- [ ] Build `captain-shot-scene` with `cmake --build build-demo --target captain-shot-scene -j 2`.
- [ ] Launch the resulting executable with a compatible existing private pack. Record its exact local command and runtime dependencies outside public packaging.
- [ ] Observe a real window: shoot, change a setting, clear, repeat, resize and exit. A `--capture` image is supplementary evidence, not proof of window/input operation.

**Gate at 1:00:** If blocked, spend at most the next hour on the concrete build/runtime problem. If unresolved at 2:00, select the CLI interaction fallback and report the graphical blocker. Do not consume the whole session installing tooling.

## Task 2: Establish repeatable launch (hour 2)

**Modify only if needed:** `CMakeLists.txt`, `engine/src/shot_scene_view.cpp`. **Create:** `docs/DEMO_RUNBOOK.md`.

- [ ] Document prerequisites, executable location, private pack argument, controls and expected visible result in the runbook. Keep private pack contents and machine-specific evidence outside Git.
- [ ] Launch from a fresh process using the documented command; confirm no dependence on an already-running tool or hidden working directory.
- [ ] Check absent and invalid pack handling. Fix an observed launch problem with a focused regression check where practical.
- [ ] State the scene's scope in the runbook: placeholder art, reset after each shot, fixed development inputs and no autonomous match simulation.

**Gate at 2:00:** A repeatable window launch, or a reproducible CLI fallback with an explicit graphical failure. A launcher convenience script is optional and must not embed private paths or packs.

## Task 3: Stabilize the interaction (hours 3-4)

**Existing interface:** `calculate_shot_scene(const ShotContent&, const ShotSceneOptions&) -> CompletePreparedShotResult`.
**Likely files:** `engine/src/shot_scene_view.cpp`, `engine/src/shot_scene.cpp`, `engine/include/captain/shot_scene.hpp`. If calculation behavior requires a regression test, add `engine/tests/shot_scene_tests.cpp` and register it in `CMakeLists.txt`.

- [ ] Select one already-supported shot scenario and two supported parameter settings. Record the chosen settings and expected calculation results; do not assume the outcomes differ until measured.
- [ ] Verify that keyboard input changes the calculation request and the displayed outcome comes from that calculation. A key that advances a prerecorded result does not satisfy this interaction milestone.
- [ ] Repeat shoot/reset/change/shoot for at least ten cycles. Check stale result clearing, repeat-key handling and recovery from a rejected request.
- [ ] For any gameplay correction, first capture the relevant controlled original observation, add a failing regression check, implement the minimal correction and rerun the focused check. Preserve unchanged state on rejected persistent-session operations.
- [ ] Clearly separate scenarios compared with the original from development parameter combinations whose parity is unobserved.

**Gate at 4:00:** The minimal demo is launch -> input -> calculated outcome -> reset -> repeat. If this is still unreliable, use hours 5-6 to finish it; defer match continuation.

## Task 4: Advance the persistent match boundary (hours 5-6)

**Inspect:** `engine/src/phase_session.cpp`, `engine/src/phase_session_lab.cpp`, `engine/src/phase_session_io.cpp` and the current private continuation capture. Reuse existing command/session APIs; do not create a second gameplay engine inside the renderer.

- [ ] Start from the latest verified saved session and inspect the following control cycle and opponent decision. Choose the first missing transition that advances the scenario toward renewed player control.
- [ ] Bound original investigation to 45 minutes. Record supplied timing, random and presentation inputs and the fields to compare. If evidence does not establish the transition, stop implementing that branch.
- [ ] Where evidence is sufficient, add the focused regression/rejection check before changing the transition, preserve older snapshot readers, then compare represented state at every affected original boundary.
- [ ] Connect a visible continuation only if the existing session APIs support it within the remaining time. Preserve state between actions only where its semantics are verified; never call resetting the scene match continuation.
- [ ] If graphical continuation is too large, retain the stable interaction demo and deliver the verified continuation through the existing phase-session CLI. Identify its recorded inputs explicitly.

**Gate at 6:00:** Freeze features. Classify what exists: interactive scenario, interactive scenario plus verified continuation, or CLI engineering checkpoint. Do not label any of these a complete playable match.

## Task 5: Verification and repairs (hour 7)

- [ ] Run `cmake --build build -j 2`, then `ctest --test-dir build --output-on-failure` using the installed CTest binary. If CTest is unavailable, run every registered command from `build/CTestTestfile.cmake` using the established runner and report that method. Do not claim 62 tests if registration changes; report the actual count.
- [ ] Rebuild the demo target after any renderer/launch edits. Walk the documented launch and input flow in a real window, including resize, close, reset and ten repeated interactions.
- [ ] Recompare any changed gameplay transition with original evidence; verify rejected operations leave session state unchanged and existing snapshot readers still work.
- [ ] Fix failures within this hour and rerun affected checks. If Linux is available, run `make -j2 all` and `make -j2 check`; otherwise explicitly report Linux verification as not run in this session.

## Task 6: Delivery (hour 8)

- [ ] Keep the runnable local artifact and required runtime dependencies available. Record whether it needs a private pack; do not present a pack-dependent build as a self-contained public download.
- [ ] Update `docs/DEMO_RUNBOOK.md`, `docs/WORK_LOG.md`, `docs/CLI_HANDOFF.md` and `docs/PORT_STATUS.md` with exact commands, observed outcomes, evidence locations and remaining blockers.
- [ ] Inspect the staged diff and file list for private material; commit only task-owned public-safe source/docs and push `work`. Verify the remote branch result.
- [ ] If time remains after delivery, publish a factual Arabic progress note in the separate website checkout and verify publication independently. Do not postpone engine verification for the website note.
- [ ] Report the actual demo category, launch instructions, test results, next complete-match blocker and suggested model GPT-6.1 Terra High. Overall progress remains unmeasured.

## Review focus and completion criteria

- Missing/incompatible packs: clear launch failure, no crash or leaked private data (tasks 1-2).
- Repeated input and resets: visible state agrees with the selected calculation, no stale result (task 3).
- Unsupported persistent actions: atomic rejection and readable existing snapshots (tasks 4-5).
- Software-renderer fallback, resizing and window close: actual Windows walkthrough, separately from image capture (tasks 1 and 5).
- Artifact sharing: reproducible local run without committing or distributing derived content (task 6).

**Target outcome:** A local, bounded interactive demo with honest limitations. Persistent match continuation is conditional on the hour-4 gate and original evidence. Full matches, autonomous AI/timing, online play, full Arabic/Japanese text rendering, music and shader systems are outside this eight-hour commitment.

**Planning review:** Allocations total eight hours; final two hours are protected. The first runnable checkpoint is due in hour 1, with a hard fallback decision in hour 2. This plan changes no engine behavior and supplies no new verification claim.

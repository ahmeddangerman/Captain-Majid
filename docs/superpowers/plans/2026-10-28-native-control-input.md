# Native Control Input Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Execute inline with superpowers:executing-plans; one fresh whole-plan static review.

**Goal:** Apply explicit logical buttons and controlled movement to persistent control-ready state.
**Architecture:** Pure post-yield input poll, replaceable dribble speeds and existing axis/profile helpers; explicit menu/AI/clock pause phases.
**Tech Stack:** C++17, private Python/py65 observations, production Make/CMake.
**Spec:** `docs/superpowers/specs/2026-10-28-native-control-input-design.md`

## Constraints and review focus

No tests added/run, private data stays outside Git, no website changes or push. Review timer10→0→step timing; held versus edge input and both-button priority; original facing/opposite-axis/diagonal rules; selector32versus34 and preserved fractional/clamped coordinates; source action/AI/clock boundary order; codec5 bounds/backward compatibility and copied-state errors. Do not silently advance the clock or substitute rendering speed for simulation time.

## Tasks

- [x] Extend bounded observation tool with exclusive post-yield polling and explicit held/pressed masks/endpoints.
- [x] Import pack7 selector32 speeds; add motion byte/snapshot5 and phases10–12, preserving legacy readers.
- [x] Implement `poll_session_control(input,held,pressed,primary,movement,control_content)` and expose logical `poll-control` commands.
- [x] Inspect original/native represented states, normal persistent usage and production builds; update log/docs; fresh static review; local source commit.

## Ledger

Ruling: User's standing authorization requests continued implementation without routine design approval. Keep inline execution. Active developer instruction prohibits tests; observations/builds/usage/static review are separately labeled.

Pre-flight: Existing speed tables import only selectors33/34, while controlled carrier movement uses32. Add a private optional content extension instead of reusing team movement speed. Existing raw axis helper already matches E8F5 addition/clamping behavior.

Ruling: Stop before the subsequent clock/AI/menu work rather than infer its missing state/schedule. Cost of treating this as a full match tick would be inaccurate elapsed/stamina/AI transitions. The next step can connect the already implemented clock with appropriate new pending state.

Ruling: Snapshot4's missing motion-request byte defaults0 on load — setup did not read it and every supported poll overwrites it before use. Other existing optional contexts stay explicit; no missing campaign/player fields are invented.

Tasks1–3 complete: Sixteen original post-yield observations and native saved305-byte outputs have matching inspected hashes. Ordinary repeated usage from snapshot4 decrements ten polls and steps on the eleventh; extra poll is rejected without mutation. Make production build completed; CMake and final static review follow. No tests added/run.

Task4 complete: Make/CMake production builds returned0 without diagnostic lines. Fresh independent static review found no material findings and inspected original timer/button/profile/axis/refresh/stop-boundary bytes, observation records and representative hash pairs. Reviewer ran no repository code/builds/tests and made no source changes. No deferred findings. Save source locally, no public push or private data upload. Next: connect the typed post-input clock/menu/AI boundary to subsequent stamina/field progression with explicit scheduling.

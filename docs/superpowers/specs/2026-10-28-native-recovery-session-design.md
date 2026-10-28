# Persistent native loose-ball recovery

## Goal and approach

Continue supported terminal loose balls through position jitter, ordered player advancement, recipient selection and persistent ownership/position writes. Then expose an explicit prepared presentation tail into player control. Reuse the existing recovery and movement calculations; preserve rendering/random scheduling boundaries for later online integration.

Alternatives considered: call the old recovery helper before moving players (wrong original ordering), invent a full frame engine now (unsupported), or split the existing helper at the movement boundary and connect the represented state. Choose the last under the user's standing authorization to continue without routine design questions.

## Original boundaries

Entry8BDF calls9070, which conditionally clears a special owner flag and the low stamina byte for player20. It does not call the random generator. Jitter8C6D uses current E2/E3 and records an X jitter control byte; outside-field routes divert before player advancement. C606 then advances players/consumes elapsed and can shift E2. Recipient selection uses those updated positions and random byte. Ownership/side change, integer player coordinates and cleared attack subtype are written before8C2E. C624 initializes the ball variant only when zero. Stop at8C31 before goalkeeper-area logic/presentation.

Keep separate phases for recovered-owner handoff, boundary restart, keeper-area interaction and story transition. An explicit prepared continuation models otherwise frozen presentation inputs and returns player control only when the keeper-area and special-story gates are absent. No NMI/display/audio timing equivalence is inferred.

## State and compatibility

Add optional recovery context: opposing player, attack subtype, special-owner flags, ball variant, game mode and X jitter control. Extend the bounded session format to version3, retaining readers1/2 with missing context explicit. New recovery-phase snapshots require context. The connected operation requires zero selected defenders, matching the supported terminal result2 tail; do not invent the missing defender list for other callers.

Keep content version6 unchanged. Split jitter from stationary-recipient selection while keeping existing recover_ball behavior for its callers. Calculate copied state and commit only successful commands. Preserve coordinate fractions, clock/experience and keeper role fields; ownership sound is an event, not a simulation clock.

## Evidence and scope

Capture bounded original CPU recovery observations with disclosed cooperative host acknowledgements and explicit input checkpoints. Build production Make/CMake targets, inspect normal native commands and represented snapshots, then perform one fresh whole-plan static review. Do not add/run tests under the active developer constraint. Keep ROMs, tables, RAM and generated exports private. Full goalkeeper-area handling, restarts, story substitution, active ball flight and live random/frame timing remain separate work.

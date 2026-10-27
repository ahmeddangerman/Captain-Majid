# Native player phase and terminal flight integration

## Goal

Advance native field players coherently, then resolve the terminal later-flight branches using that player phase. Build on the existing raw coordinate helper and retain the original staged inputs where the full match clock/presentation has not been reconstructed. User authorized continuous implementation without routine approval gates.

## Boundaries

1. **Player movement:** reproduce supported `$E7D0/$E854/$E8A0` decisions with supplied angle thresholds/sign content, full16-bit X/Y, target cell, status, speed word, elapsed and current ball cell. Preserve byte/word wrap, one initial direction for the update, target-cell early stops and fractional edge clamping. A zero elapsed caller step is a no-op, matching the `$E074` gate; nonzero status preserves the player.
2. **Profile and target decisions:** load movement speeds and AI thresholds through the existing effective-type/profile rules. Movement selectors33/34 read raw words through the secondary profile mapping; they do not add level. Original `$A103` uses team formation/tactic templates, a mirrored coarse ball region, one consumed random bit per player, optional target mirroring and threshold overrides. Preserve scan order and caller-owned randomness.
3. **Field update:** reproduce the ordinary valid-field `$E074` player phase over all field players except the current owner. Keepers are role-specific and excluded. Resolve AI targets, load on/off-possession speed, move players, return the updated random byte and clear consumed elapsed. Preserve the original role distinction for record+5/+6/+7/+8; no keeper-coordinate aliasing.
4. **Terminal flight:** later code2 places the ball at the actual defender comparison-time position and clears selection count; later code3 transfers possession and runs the field update before player control. Keep reward/status/clock inputs explicit at the correct stage; do not substitute the initial-loop handler. Return a phase result suitable for later atomic state commits, with remaining unsupported pre-handler processing disclosed.

## Content

Extend the private pack to version4 while retaining versions1/2/3. Append movement angle thresholds64words, axis signs8bytes, ordinary/alternate movement metadata (secondary profile plus on/off AI threshold), raw speed-profile word pairs, coarse-region mirror15bytes and four on-ball/twelve off-ball formation templates of300bytes each. Counts are bounded; original data remains outside Git. Native algorithms accept replaceable values and reject unsupported formations/content when reached. The source payload hash remains provenance, not pack authentication.

## Application and state

Add a native field-phase laboratory over a typed position/roster/keeper/ball/team snapshot with bounded local state loading and an explicit advance command. State persists between its commands. Provide private snapshot export from explicit RAM checkpoints so captured original player phases can be inspected through native production code. This state describes the supported field phase, not a complete game/save/network protocol.

## Limits

No tests may be added/run under the active developer constraint. Evidence consists of private original observations, production builds, normal application usage and one fresh independent static review at this plan's end. No original code/tables/assets are committed. No fixed whole-port percentage, full playable match or online equivalence claim. NMI/audio/presentation scheduling remains outside the bounded research harness.

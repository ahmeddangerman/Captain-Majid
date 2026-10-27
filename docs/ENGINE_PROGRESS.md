# First engine foundation progress

**Date:** 2026-10-27

**Scope:** Native skill-index, action-power, field-defender, and goalkeeper comparison calculations, plus a development replay fixture. Stamina, keeper story/state, and a single original random step are now implemented; their native differential verification is pending. The authentic full shot interaction remains incomplete.

## Implemented

- Portable C++17 core with no rendering, audio, networking-library, or ROM dependency at runtime.
- Original action-power contact-loss behavior, independently implemented from the verified Japanese ROM contract in [ACTION_STRENGTH.md](ACTION_STRENGTH.md).
- Defender strength variation, weakened/special branches, score calculation, threshold classification, and conditional contact loss, specified in [DEFENDER_CONTACT.md](DEFENDER_CONTACT.md). Coefficients and thresholds are inputs; the engine bundles no original ROM tables.
- Standard level/move index adjustment, randomized action power, and the pre-override goalkeeper comparison, specified in [SHOT_POWER_AND_KEEPER.md](SHOT_POWER_AND_KEEPER.md). The shared defender roll is reused for goalkeeper strength.
- Stamina deduction with the original special-player surcharge, raw subtype exemption, wrapped subtraction, weakening, and outgoing carry, specified in [ENERGY_USAGE.md](ENERGY_USAGE.md).
- Keeper story classification override and explicit timer/penalty helpers, specified in [KEEPER_STATE.md](KEEPER_STATE.md). The shot caller still needs to select the appropriate helpers and apply complete consequences.
- One original random-generator step plus a canonical 2 KiB RAM sampler, specified in [RANDOMNESS_NOTES.md](RANDOMNESS_NOTES.md). Its original gameplay schedule is unresolved.
- A fixture session that separates participant identity from team, rejects unauthorized and unordered commands before state changes, and serializes every fixture state field explicitly.
- A native terminal executable with a batch arithmetic mode and strict replica command protocol.
- A Python development adapter that sends commands through loopback TCP to two separate native processes, then compares their complete state after every command.
- Make build/check commands and CMake build definitions for Windows/Linux development.

## Evidence

The initial missing implementations failed linkage as expected. The completed core and terminal checks pass on Linux with GCC. The ROM comparison executes the isolated original 6502 routine and matches the native output in 6,656 boundary and seeded cases. No third-party source, ROM data, graphics, audio, or translated comments are included.

The local TCP proof exercises 15 commands, a deliberately delayed replica, duplicate/future commands, unknown/unauthorized participants, malformed input, depletion of power, and connection cleanup. It requires matching full snapshots and expected replies after every command. Unit checks also require different fixture configuration to produce a different snapshot.

An independent code review found no critical or important issues in the defined scope. Its three minor findings were addressed: initialize sockets before spawning the native child, enforce a deadline for an entire protocol line rather than each arriving byte, and use the virtual-environment interpreter in the ROM comparison instructions. Regression checks reproduced the process leak and the absent deadline before the fixes and now pass.

The expanded defender calculation matches original ROM execution for 19,200 cases across all eight exposed outputs. These include 6,511 special-branch cases, 4,584 weakened cases, 3,093 zero-strength cases, and 1,111 conditional loss cases. The comparison reads 64 nonzero-range coefficient pairs from the user's ROM and checks all three field-defender threshold rows. These are isolated, deliberately constructed fixtures rather than captured full-match traces.

The defender increment received an independent review with no critical or important findings. Its one minor verification finding was addressed by adding explicit incoming-power wrap boundaries (1023, 1024, 1025 and other limits) to the original-ROM corpus, alongside the defender-power sweep.

The TCP experiment also compares ordinary, weak, and special defender calculations and rejection of invalid coefficients. These stateless research requests leave the original ordered fixture state unchanged.

The next original-ROM comparison passes 69,632 skill-index cases, 7,456 randomized action-power cases, and 5,248 goalkeeper cases. Skill checks cover every base-byte/level-byte pair with varying move modifiers plus seeded extra cases. Action checks include carry, critical, weakened, and truncation boundaries. Keeper checks include both team-side pointer paths, threshold equality, numerator wrap, zero strength, and state-adjustment saturation. Every exposed output is compared with original instruction execution.

The loopback proof also checks skill adjustment, action power, and goalkeeper comparison, including using a calculated action power as the next keeper query's input. These remain research queries, not integrated shot commands or complete match state.

Independent review of the skill/action/keeper increment reported no actionable findings and reproduced its full native checks and all three new ROM comparison sets. The earlier defender comparison also remains green after extracting the shared strength-variation code.

### Current increment: implementation ahead of verification

The new stamina, keeper story/state, and random-step APIs are exposed through strict batch modes and stateless replica queries. Research parsing lives separately from the session driver. The new queries do not access or update the ordered session.

The prepared private-ROM harness contains 9,728 stamina cases, 6,812 story cases, 5,632 keeper-state cases, and 3,584 random-step cases: 25,756 total. It uses mirrored RAM for the original random routine and checks the exact reference payload before executing it. This corpus has not yet been recorded passing against the native binary. Fresh full-suite and TCP verification of the new commands also remain pending. The earlier passing results above describe the preceding increments, not this integration.

On 2026-10-27, a forced Linux/GCC prototype build (`make -B all`) completed without compiler warnings. An independent static review found no actionable issues in arithmetic, carry handling, RAM mirroring, parsing, session isolation, or build integration; it also inspected the original instructions directly. No tests or differential verifiers were executed during this continuation. Compilation and static review do not replace those pending behavioral checks.

## Limits

- The fixture advances a contact-loss stage; it does not determine an authentic shot result or offer a playable match.
- The defender and goalkeeper calculations return original numeric result codes. The keeper story override is isolated; field-defender overrides, rewards, ball ownership/trajectory changes, and the full goalkeeper result consequences are not yet integrated.
- The skill helper computes standard indices only. Player/profile lookup, loading original strength/cost tables, caller-specific stamina accounting, and special-skill selection are still needed.
- Shot arithmetic receives explicit random bytes. A single original generator step is implemented, but its inputs and frame schedule still need study before integrating a complete deterministic match.
- Static inspection identifies a frame-interrupt random generator that mixes player and sound-driver memory. [RANDOMNESS_NOTES.md](RANDOMNESS_NOTES.md) records why a gameplay trace is required before choosing simulation scheduling compatible with replacement music and online play.
- The fixture's initial power 300 and loss 37 are developer-selected inputs. They are not claimed to be an actual player's statistics.
- This snapshot is a comparison format; save loading and network state recovery are not implemented.
- Python supplies TCP transport for this experiment. Native transport, protocol/version negotiation, identity authentication, encryption, reconnection, and host validation remain future work.
- A sender supplies a participant ID in this local development protocol; the ID is not authenticated. The adapter binds only to loopback and is unsuitable as a public game server.
- No private-room service, invitations, random matchmaking, ranked modes, or shared-team game rules exist yet.
- Windows compatibility and CMake generation are intended by the project structure but have not been executed in this environment. Current build evidence is Linux/Make/GCC only.

## Next checkpoint

Complete native verification of the current increment. Then trace actual player/profile and action-cost selection, caller-specific energy accounting, and random scheduling during gameplay. Map defender/keeper numeric result codes and story overrides to complete changes in ball possession, trajectory, and score. Join the supported parts into one authentic shot flow, with a small visual scene once its outcome is supported by the original behavior.

Early network comparisons continue alongside each simulation increment. Public matchmaking and the rules for additional participants remain part of the engine direction, without committing to an unsupported player count.

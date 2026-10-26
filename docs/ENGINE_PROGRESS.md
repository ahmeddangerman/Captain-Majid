# First engine foundation progress

**Date:** 2026-10-26

**Scope:** Native action-strength and field-defender calculations, plus a development replay fixture. The authentic full shot interaction remains incomplete.

## Implemented

- Portable C++17 core with no rendering, audio, networking-library, or ROM dependency at runtime.
- Original action-power contact-loss behavior, independently implemented from the verified Japanese ROM contract in [ACTION_STRENGTH.md](ACTION_STRENGTH.md).
- Defender strength variation, weakened/special branches, score calculation, threshold classification, and conditional contact loss, specified in [DEFENDER_CONTACT.md](DEFENDER_CONTACT.md). Coefficients and thresholds are inputs; the engine bundles no original ROM tables.
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

## Limits

- The fixture advances a contact-loss stage; it does not determine an authentic shot result or offer a playable match.
- The defender calculation returns an original numeric result code. Foul overrides, rewards, ball ownership/trajectory changes, and the full goalkeeper resolution are not yet implemented.
- Random bytes are supplied explicitly. Original random generation and its dependence on frame timing still need study before integrating a complete deterministic match.
- The fixture's initial power 300 and loss 37 are developer-selected inputs. They are not claimed to be an actual player's statistics.
- This snapshot is a comparison format; save loading and network state recovery are not implemented.
- Python supplies TCP transport for this experiment. Native transport, protocol/version negotiation, identity authentication, encryption, reconnection, and host validation remain future work.
- A sender supplies a participant ID in this local development protocol; the ID is not authenticated. The adapter binds only to loopback and is unsuitable as a public game server.
- No private-room service, invitations, random matchmaking, ranked modes, or shared-team game rules exist yet.
- Windows compatibility and CMake generation are intended by the project structure but have not been executed in this environment. Current build evidence is Linux/Make/GCC only.

## Next checkpoint

Trace the callers that supply player statistics, coefficients, and random values during actual shots. Map numeric result codes to complete changes in ball possession and trajectory, then reconstruct goalkeeper resolution. Add a small visual scene once enough original rules are mapped to display an honest interaction outcome.

Early network comparisons continue alongside each simulation increment. Public matchmaking and the rules for additional participants remain part of the engine direction, without committing to an unsupported player count.

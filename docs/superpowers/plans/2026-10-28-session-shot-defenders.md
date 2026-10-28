# Persistent shot defenders implementation plan

> Execute inline under the user's continuous-work authorization.

Goal: connect prepared shots to persistent selected defenses and terminal recovery.
Spec: ../specs/2026-10-28-session-shot-defenders-design.md

- [x] Add regression cases for no defenders, terminal stop, interruption, selected
  prefix validation, invalid/repeated calls and version8 snapshot preservation.
  First compile/run with a throwing stub and observe the expected failure.
- [x] Implement session_shot.hpp/.cpp and phase21/22 validation, retaining explicit
  choices/randomness and saved preparation. Permit phase20 terminal handoff.
- [x] Update codec version8 with legacy phase rejection and register build/tests.
- [x] Expose --shot-defenders-file and interactive shot-defenders-file, strictly
  reading one action/subtype/random pair per selected defender.
- [x] Run focused regression, production build and full existing suite. Review
  atomic errors, selected ordering, duplicate IDs, interruption state and clock
  pauses. Record evidence and next boundary in WORK_LOG/PORT_STATUS.

Review focus: terminal stop must not charge later defenders; no repeated attacker
cost; input/prefix errors atomic; added-time resume must not repeat rewards;
old format tags cannot acquire new phase meanings.

Evidence: throwing-stub regression failed; focused checks then passed. Reviewer
keeper-stun/random regressions each failed before fixes and pass afterward.
`make -j2 check` returned0, including all native checks, CLI/adapter checks,
loopback/replay proof and reference-cartridge checks. Legacy v7 prepared-shot CLI
loaded, new324-byte v8 checkpoint saved/reloaded, repeated and extra-row commands
rejected. CMake is unavailable. Final production Make build recorded in WORK_LOG.

Follow-on original timing audit identifies outstanding ordered nonterminal/final
clock scheduling; the feature is a calculation checkpoint, not an original
field-dispatch-equivalent save. This narrows the interpretation of phase21/22.

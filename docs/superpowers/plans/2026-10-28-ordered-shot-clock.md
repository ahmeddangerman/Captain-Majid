# Ordered shot clock implementation plan

Execute inline under continuous authorization. Spec: ../specs/2026-10-28-ordered-shot-clock-design.md.

- [x] Write tests for ordered contact/status/time, no double attacker debit,
  added-time pause/save/load/resume, no-defender and completed-prefix final clock,
  terminal/interruption skipping final clock, period-end refusal, old snapshots.
  Run against throwing stubs to observe failure.
- [x] Implement staged_shot.hpp/.cpp, progress state validation and session9 codec.
- [x] Add strict CLI step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2,
  resume-shot-clock and dispatch-shot ELAPSED commands/options.
- [x] Run focused checks and full suite, compare staged output to three recorded
  original dispatch boundaries, review and document limits; save local checkpoint.

Review focus: resume must not replay contact/final timing; no future comparisons
at pause; interruption reset precedes carrier status; period-end cannot advance;
old formats cannot silently become new dispatch-ready state.

Evidence and deferred presentation/coverage limits: ../../NATIVE_ORDERED_SHOT_CLOCK.md.

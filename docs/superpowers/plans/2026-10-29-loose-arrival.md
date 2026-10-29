# Unopposed flight arrival and loose recovery plan

Phase49 arrival callback with count0 returns unchanged through8835;50 early return
already skipped it. Both branch atDD11 using saved loose carry:51 stops before
loose presentation45;52 stops before keeper pointer/status routing. Reject queued
encounters instead of discarding them. Prepared51 continuation after acknowledged
presentation enters8BE5, not8BDF: no jitter or special-owner cleanup. Reuse numeric
recovery with explicit output6, clear flight contexts at ownership handoff. Existing
recovery continuation routes to control/keeper/story. No guessed presentation effects.

Snapshot21L retains355bytes/readers1..20; new51/52 needL. Flight contexts through52,
count0 required for51/52; loose flag must match each branch.

- [x] Failing real branch/recovery/no-jitter/context/save/replay regressions.
- [x] Implement route and shared recovery output selection; validation/I/O/CLI.
- [x] Compare originalDD11 branch and prepared8BE5 recovery for interior cases.
- [x] Full suite/build/review; record limits and publish only daily notes.

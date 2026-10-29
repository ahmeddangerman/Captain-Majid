# Persistent scored goal preparation plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Commit a normal/stunned keeper goal result into score and scorer reward,
then save59 before goal presentation1/celebration30. Do not pretend kickoff or celebration is finished.
**Architecture:** Native adapter for original85AC..85F0 uses existing experience
selector0 and byte-wrapped side0 score increment. Explicit initial left/right/
kickoff bytes avoid guessed scores; optional existing score must match supplied
baseline. New score context persists; snapshot25P365bytes appends4score bytes.
**Tech Stack:** C++17/Make, bounded original instruction comparison.
**Spec:** AGENTS.md, NATIVE_KEEPER_ARRIVAL.md and NATIVE_KEEPER_SAVES.md.

## Constraints / review focus

Side0 only; requires55/56 result0, or result1 with raw subtype&63<3. Special turnover
rejects.59 retains shot/trajectory/decision context and empty stunned or sole normal
keeper prefix; score required, no clock/status/RNG/ball mutation or duplicate reward.
Both N/O legacy readers retained; earlier formats reject59. Replay/invalid baselines
atomic. Original observer executes reward and score helpers, no skipped calls,
stops before display30/987B/possession switch/keeper cleanup/kickoff4.

## Task1

Interface: prepare_session_scored_goal(state, MatchScoreContext baseline,
const ShotContent&) -> PhaseSessionState59.
- [x] Native real RED with stub: normal/stunned goal, byte wrap, reward once,
  invalid result/special-turnover/baseline, save/replay/old reader/malformed score.
- [x] Adapter/phase59 invariants and P score serialization, CLI and interactive.
- [x] Original85AC..85F0 comparisons for score/reward boundaries; joined free goal
  trajectory through normal comparison and prepared post-display9 goal registration.
- [x] Full Make suite/build, fresh review, docs/log and daily note.

Ruling from original trace: C52A at85F2 waits in schedulerCAAE, so85B7 cannot
be reached without host presentation acknowledgement. Save at85F0 immediately
after score increment and before goal presentation1. Reward/score arithmetic is
complete, but that presentation and celebration30/987B remain explicitly pending.

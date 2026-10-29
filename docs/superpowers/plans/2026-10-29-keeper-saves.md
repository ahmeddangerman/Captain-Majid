# Persistent keeper save handoffs plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Commit non-goal result2/3 keeper rewards/status/ball ownership and connect
rebound/punch recovery without duplicate clock, energy, rewards or jitter.
**Architecture:** New keeper save adapter reuses award_player_experience and
resolve_shot_outcome, only after an explicitly acknowledged display9. Result2
stops before8BDF (generic recovery2). Catch3 stops beforeC633 (keeper possession57).
Punch3 stops before8BE5 (no-jitter recovery58); dedicated recovery wrapper rejoins
existing generic recovered-owner/control routing. Snapshot24O361bytes, readers1..N.
**Tech Stack:** C++17/Make and bounded original861C/8646 instructions, no skipped calls.
**Spec:** AGENTS.md, NATIVE_KEEPER_ARRIVAL.md. Only normal side0 shot result2/3;
goals/special turnovers and scoreboard remain pending. Existing standalone outcome
score/kickoff fields are unused in these guarded branches; no score capture guessed.

## Review focus

Keeper possession57 explicitly permits own keeper11, not field motion ownership.
Normal53/56 opposing keeper prefixes and all field prefixes remain strict. Catch
preserves original count1/prefix; rebound/punch clear count/unused prefix/context.
Punch58 retains elapsed1 and stale ball cell through original recovery; later control setup owns cell refresh.
Reward selector3 uses raw opposing keeper type; one debit, replay atomic. No-jitter
punch recovery must skip original8BDF cleanup/jitter and choose generic6 for either side.

## Task1: saved non-goal outcomes and recovery

Interface: resolve_session_keeper_save(state, const ShotContent&) -> KeeperSaveResult;
advance_session_keeper_punch_recovery(state, primary, movement) -> RecoverySessionResult.
- [x] Real RED tests with stubs: rebound/catch/punch, reward once, status, ownership,
  save/replay/malformed/legacy guards and punch no-jitter recovery.
- [x] Implement guarded adapters, tightly scoped keeper ownership validation andO writer.
- [x] CLI resolve-keeper-save and recover-keeper-punch, interactive equivalence.
- [x] Compare original every instruction through8640/865B/86AE for multiple random/
  status/experience boundaries; compare prepared8BE5 recovery separately.
- [x] Join prepared normal flight to save handoff; suite/build/review/log/daily note.

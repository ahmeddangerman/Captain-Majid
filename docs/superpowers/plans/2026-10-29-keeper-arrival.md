# Saved keeper arrival implementation plan

> **For agentic workers:** Use superpowers:executing-plans inline; verify each task.

**Goal:** Connect keeper-bound flight52 to original normal/stunned preparation,
then save the stunned decision before presentation12.
**Architecture:** Dedicated keeper_session adapter over PhaseSessionState; retain
flight metadata, snapshot22M358 bytes with optional3-byte keeper progress tail.
**Tech Stack:** C++17, Make, private py65 original-ROM observation.
**Spec:** AGENTS.md and docs/NATIVE_SAVED_BALL_FLIGHT.md; user requests faithful
native reconstruction, original comparisons, continuous execution, private source.

## Global constraints

No ROM/data in Git, no public engine push, preserve old snapshot readers. Prepared
numeric state never implies presentation/live-frame execution. Current shot menu
supports side0 only. No script or outcome acknowledgements are guessed.

## Review focus

- Keeper ID11 in selected prefix is valid only at normal keeper53, not field scan.
- Stunned classification preserves clock, random, stamina, score and ball state.
- Save/replay and oldL355-byte compatibility; forged phase/context combinations.
- Queue cannot be skipped to reach keeper; wrong phase and invalid thresholds atomic.
- Stun route chooses opposing keeper rather than scoring-side keeper.

## Task1: saved preparation and stunned decision

Interfaces: prepare_session_keeper_entry(const PhaseSessionState&) -> state;
resolve_session_stunned_keeper(const PhaseSessionState&, array<uint8_t,2>) -> state.
Normal entry52 executes DD1C..8509 (before first presentation): phase53,count1,
selected keeper(side^11), opposing player set; no keeper progress yet. Stunned
entry52 executes DD1C..8900: phase54,attack_action0,opposing keeper set,count0.
Stunned54 executes8900..8917 with explicit descending thresholds ending0:55,
keeper progress cursor0/result0..1. No RNG advancement or presentation12.

- [x] Add real failing tests with stubs: normal/stunned routes, retained bytes,
  each threshold boundary, other keeper status independence, invalid/replay inputs.
- [x] Implement keeper adapter and scoped validation; field prefixes remain strict.
- [x] SnapshotM358/readers1..L; add malformed tail/old reader/round-trip regressions.
- [x] Expose CLI + interactive prepare-keeper-entry/resolve-stunned-keeper T0 T1.
- [x] Original bounded entry/decision comparisons including all256random bytes;
  join existing whole prepared keeper flight through saved normal/stunned entry.
- [x] Make check/all, clean diagnostics, fresh review; docs/log and website notes.

Ruling from original comparison: scan8928 contains16,0, then unrelated instruction
bytes. Only two entries are reachable; initial speculative160/16/0 test table
incorrectly implied a third outcome. The first oracle rejected actual16/0/160,
exposing this before acceptance. Use exactly two explicit thresholds, last0,
and result0..1. Cost if another ROM changes this algorithm: extend content/parser.

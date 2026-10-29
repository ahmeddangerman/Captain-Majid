# Persistent prepared keeper comparison plan

> **For agentic workers:** Use superpowers:executing-plans inline.

**Goal:** Connect normal saved keeper53 to strength/keeper comparison and story
classification, saving56 before presentation9 with no lost timing/energy boundaries.
**Architecture:** Reuse prepare_goalkeeper and apply_keeper_story_override. Caller
explicitly acknowledges earlier keeper choice presentation and supplies action,
subtype and opponent-team byte. Private pack supplies existing skills/moves/curve.
Shot power, clock, RNG, coordinates and scoring stay unchanged. State56 retains
keeper prefix and flight, plus cursor/result/action/modified subtype/effect.
**Tech Stack:** C++17 Make; bounded py65 starts852F through8599, every call executes.
**Spec:** AGENTS.md, NATIVE_KEEPER_ARRIVAL.md. Normal side0 session only.

## Review focus and constraints

- No inferred menu choice or presentation/frame effects; input is explicitly prepared.
- Comparison uses saved menu context621 and shot distance062B, opposing status/record.
- Raw primary type (not mapped type) controls critical threshold; story uses explicit
  team2B plus saved446 and raw attack subtype43C.
- Preserve attack power, stamina(side0 skips keeper debit), random, score and clock.
- Snapshot23N361bytes: M3-byte decision tail still readable; N adds3choice/effect
  bytes. Stunned55 requires these new metadata bytes0; normal56 allows result0..3.

## Task1: normal keeper adapter and save/CLI

Interface: resolve_prepared_session_keeper(state, action, subtype, opponent_team,
const ShotContent&) -> PhaseSessionState56. Requires53, private keeper profiles/moves.
- [x] Real failing native test with stub, deterministic comparison/story cases.
- [x] Adapter, phase56 invariants, N serialization/readers, malformed/replay tests.
- [x] CLI and interactive resolve-prepared-keeper ACTION SUBTYPE OPPONENT_TEAM.
- [x] Original852F..8599 comparisons spanning actions/types/random/clock/status;
  joined prepared normal flight also compares its final keeper decision.
- [x] Full Make suite/build, clean diagnostics, fresh review, log/docs and daily note.

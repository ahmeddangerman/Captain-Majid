# Kickoff formation placement

`place_kickoff_formation` implements original DC07 placement as a pure engine
operation. It accepts all22 raw coordinate pairs, possession side, both formation
indices and explicit on-ball/off-ball cell tables for the selected game version.
It places the team with possession using the on-ball table and the other using
its off-ball table, mirrors coordinates when side11 has possession, preserves
fractional coordinate bytes, and clears the two keepers' Y low bytes as original.
Keeper raw fields overlap status fields in original memory; this interface keeps
all22 raw pairs explicit instead of inventing the missing keeper Y high bytes in
the persistent session. Invalid sides, formations and used cells reject.

The native focused test failed with an unimplemented stub and then passed.128
controlled original DC07-through-return comparisons cover both possession sides,
all16 combinations of the two teams' four formations and four sets of random
prior coordinate words per combination. All22 raw X/Y pairs match, including
keeper aliases. Every original instruction in the measured routine executes;
no frame, menu or presentation is involved. Evidence:
`../Captain-Majid-analysis/kickoff-formation-final/verification.json`.

```
PYTHONPATH=tools python3 tools/check_kickoff_formation_original.py \
 /private/Japanese-base.nes /private/new-output-directory
```

The C++ library includes this operation and uses explicit content; the session has
not yet captured keeper Y bytes or integrated it into live kickoff. Earlier reset,
selection, ball setup and UI remain separate. Default Make suite/build pass; CMake
definitions are updated but CMake is unavailable in this environment.

## Saved formation handoff

`--prepare-kickoff-formation FILE SIDE` accepts the saved post-goal phase60 with
an explicitly supplied kickoff side and private `CMJKICKFORMATION1` content. The
caller must supply state after the original earlier reset and team selection. The
operation reproduces DAE9..DAF5: it assigns the initial carrier (side+8), applies
DC07 to all22 records, retains score and existing context, and saves phase61 before
receiver selection. This is a prepared boundary; it does not execute the earlier
reset/menu or later receiver, animation, ball setup or live frames.

Keeper positions overlap original record fields. The session writes X low/high
through timer/penalty-duration aliases, clears the Y low/penalty alias, and saves
both resulting keeper Y high bytes in `KickoffFormationContext`. Old Y high bytes
are overwritten by the original routine and are not guessed as retained data.
Snapshot27R appends presence and two Y bytes (368 bytes); readers through26Q remain.
Malformed/truncated metadata, wrong phase, invalid side/content and replay reject.

A real session test failed with an unimplemented stub then passed.64 original
DAE9..DAF5 comparisons match every represented snapshot byte, covering both sides,
all16 formation pairs and two randomized raw record sets. The CPU observer runs
every original instruction in that boundary; comparison includes the two keeper
Y bytes, score, roster aliases and all field-player positions. It verifies quoted
interactive content paths and replay rejection. Evidence:
`../Captain-Majid-analysis/kickoff-session-first/verification.json`. The full
Make suite and production build pass; CMake is unavailable in this executor.

```
PYTHONPATH=tools python3 tools/export_kickoff_formation.py \
 /private/Japanese-base.nes /private/formation.cmjkick
build/captain-phase-session /private/content.cmjpack /private/kickoff.cmjsession \
 --prepare-kickoff-formation /private/formation.cmjkick 11 --capture /private/placed.cmjsession
```

The original team selection may select either side; the side argument is captured
input after that step, rather than being assumed from the earlier goal. Some state
such as the original second side field and complete reset effects is not yet
represented. A live playable kickoff requires those earlier/later integrations.

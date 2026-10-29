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

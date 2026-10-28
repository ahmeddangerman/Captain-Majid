# Selected restart routing

Phase30 now continues original9366 through its field/goal-area branch selection.
It clears situation/ball_variant044E to0, then chooses ordinary restart9379 or
opposing goal-area restart94CF. Original coordinate thresholds are preserved:
Y96..159 and X<80 or X>=176 define the goal-area region; only the opposing area
uses the special route. This does not execute the later restart setup/player UI.

`--route-selected-restart` (interactive `route-selected-restart`) enters saved
phase38 field setup or39 goal-area setup. Status reset, movement, rewards and clock
are not repeated. Replay rejects. Session15 marker F keeps336bytes, reads
versions1–14, and requires F for the new routing phases.

The regression failed against a throwing stub, then passed threshold cases,
input/status/time preservation, replay and new/legacy snapshot handling. Full
native/CLI/adapter/loopback/reference suite and production Make build passed with
clean compiler diagnostics using `CORE=/tmp/cm-restart-core.a`; CMake unavailable.
Independent review reported no material findings.

Four bounded original9366 observations and native CLI calls match324 prior
represented bytes after phase/version normalization; appended native metadata
is excluded from the original-memory claim. Cases: ordinary field, own goal area,
opposing goal area boundaryX176/Y96 and outside goal-area Y160. Private evidence:
`/workspace/Captain-Majid-analysis/restart-route-5zph1ykc`. The observer executes all
original calls before the first route, without scripts/frame/audio/PPU emulation.

Next: actual field/goal-area restart setup, keeper/story and remaining nonterminal
shot/goal paths. A complete playable match and online match remain unfinished.

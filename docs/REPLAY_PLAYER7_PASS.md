# Player 7 pass replay from phase 317

`tools/verify_player7_pass_end_to_end.py` checks one continuous sampled route, beginning with the saved first-defender contact at phase 317 and ending after player 7 receives the pass and control is set up. It advances the second and third defenders, pass dispatch and field update, target, numeric flight, reception and control. It checks the starting boundary as well as every generated boundary against the supplied original RAM snapshots. A mismatch stops the replay at the first case, boundary and named field, with native and original values.

The session, RAM, content pack, velocity data and generated outputs are private. Supply their paths on the command line; the output directory must be empty and outside the Git checkout. No ROM or private evidence is included in this repository. Requires Python 3 and a built `captain-phase-session` binary.

```text
python tools/verify_player7_pass_end_to_end.py \
  --exe PATH_TO_PHASE_SESSION --pack PRIVATE_TEAM_PACK --velocity PRIVATE_VELOCITY \
  --starts PRIVATE_PHASE317_DIRECTORY --start-suffix=-clock.cmjsession \
  --encounter-original PRIVATE_DEFENDER_TRACES --handoff-original PRIVATE_HANDOFF_TRACES \
  --flight-original PRIVATE_FLIGHT_TRACES --receive-original PRIVATE_RECEPTION_TRACES \
  --control-original PRIVATE_CONTROL_TRACES --flight-fractions X Y \
  --output EMPTY_PRIVATE_OUTPUT_DIRECTORY
```

Each starting filename must end in the supplied suffix; its preceding text is the case ID. Matching original filenames use that ID followed by the observed boundary suffixes, such as `-80dc-1.bin`, `-c612-1.bin`, `-e773-1.bin` and `-ready.bin`. See the replay steps in the script for the complete naming contract. The recorded 2-5 route uses fractions `167 221`; the recorded 3-6 route uses `82 109` and a starting suffix of `-clock1.cmjsession`. These are captured values, not generated random or animation timing.

The defender preparation and numerical flight functions are shared implementation. The route's participants, selected actions, comparison inputs, target and accepted phase transitions remain guarded to the observed encounters. This replay does not implement general opponent choice, arbitrary passing, live presentation, hardware timing, authentic match initialization or a complete playable match.

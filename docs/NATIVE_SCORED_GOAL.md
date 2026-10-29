# Persistent scored goal preparation

`--prepare-scored-goal LEFT RIGHT KICKOFF` accepts saved keeper55/56 after an
explicitly acknowledged keeper presentation. Result0 scores; result1 scores only
when raw attack subtype&63 is below3. Other results/special turnover reject. The
adapter awards the scorer original selector0 once, increments the side0 score byte
with wrap, and saves59 immediately after85ED, before goal presentation1 at85F0.

The explicit baseline captures previously unrepresented0028/0029/0629 bytes;
no score is guessed. An existing saved score must agree with the baseline. Right
score and kickoff remain unchanged at this boundary. Time, RNG, ball, possession
and keeper status are unchanged. Replay rejects. Native goal59 retains the normal
keeper prefix1 or stunned empty prefix, shot/trajectory/decision contexts and score.
No goal celebration, story helper987B, side flip, keeper cleanup or kickoff4 has
run yet. Interactive command uses the same name without dashes.

Snapshot25P appends presence/left/right/kickoff (4bytes),365total. Existing readers
1..O remain; O361-byte decisions load without score; O rejects59. Score context
can persist into subsequent phases, but59 requires it. Absent score bytes must be
zero; invalid presence/truncation/trailing data reject. Goal shape/result/prefix
and ordinary/stunned context validation remain strict, with no optional flight
access in no-context57/58.

Real native test failed with an unimplemented adapter then passed.72 original
observations match represented state and new score/reward bytes: normal/stunned,
result0/1 and raw subtype masks, scorer types1/33/64, score wrap and reward saturation.
All original instructions85AC..85F0 execute. Initial target85B7 was corrected after
tracing showed C52A waits in schedulerCAAE; reaching it requires host goal-presentation
acknowledgement. This checkpoint ends before that wait rather than silently skipping
it. Evidence: scored-goal-final/verification.json.

Eight full prepared numeric trajectories match every saved cell/callback. Eligible
normal/stunned cases also match goal preparation after explicit keeper presentation
acknowledgement: full-flight-scored-goal. These are frozen-input prepared comparisons,
not an end-to-end live match or completed kickoff sequence.

```
PYTHONPATH=tools python3 tools/check_scored_goal_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/keeper-decision.bin /private/new-goal-output
```

Next: prepared post-goal presentation/987B handoff, keeper cleanup and kickoff
state, special turnovers, keeper control, queued arrival encounters and live frames.


Final default Make suite/build pass with clean diagnostics. Fresh assertion-enabled
core builds pass goal and keeper-save tests, including cleared flight context
checks. CMake unavailable. Review found no gameplay defect; it caught a reusable
observer fixture that always wrote scorer type at owner2's record. A current-owner
assertion failed on a separate owner3 input (RED), the fixture now computes
0300+12*owner, and all72cases pass for both owners (144total, including
scored-goal-owner3-green). The source has a persistent current-owner assertion.
The assertionO1 build retains the previously recorded unrelated GCC optional-vector
move diagnostics; defaultO2 diagnostics remain clean.

## Prepared post-goal kickoff handoff

`--continue-prepared-goal-restart` accepts59 after the caller explicitly supplies
state following goal presentations and the post-goal menu, including ball mode0.
Original85BF..85DD switches possession side to11, clears defending keeper timer,
penalty and stun (not penalty duration), and sets kickoff4. The new saved phase60
retains the score and selected keeper prefix, while clearing obsolete shot contexts.
The previous scorer remains ball owner until the later kickoff setup. Replay and
invalid menu/phase input reject. Snapshot26Q remains365bytes; prior readers remain.

Twelve constructed original comparisons match every represented state byte across
normal/stunned keepers and status boundaries, with every85BF..85DD instruction
executed. Evidence: private post-goal-first-comparison. Full Make suite/build pass.
This is a prepared boundary: presentation, the post-goal menu and actual kickoff
setup are not implemented or claimed equivalent to live play.

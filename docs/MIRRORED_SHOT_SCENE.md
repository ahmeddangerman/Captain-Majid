# Both directions and the catch handoff

The shared development shot calculation accepts `ShotSceneOptions::side`0/11. Side0 retains attacker3/keeper11 and selected field defenders14..18; side11 uses attacker14/keeper0 and selected field defenders3..7. Profiles, stamina, context, keeper recovery and printed values use those actual slots.

Coordinates remain explicit and are never silently mirrored. Side11 requires version2 geometry content; version1 retains the previous default side0 fixture. For a mirrored native catch example:

```bash
./build/captain-shot-lab /path/to/private-v2.cmjpack \
  --side 11 --ball-x 95 --defenders 0 --keeper-level 80 --keeper-action 0
```

Normal usage produces power33, keeper strength177/ratio35, target(52,124), a catch with possession/owner0, timer7, penalty/duration/stun0, count1/active0/kickoff0, attacker stamina1000 and keeper stamina980. This preserves the original asymmetric energy rule: these stamina changes differ from the side0 examples. Interactive `side 0`/`side 11` selects a side; set ball coordinates explicitly too. The optional window still exposes its original side0 controls; this increment adds CLI/shared-API coverage.

## Original human-choice observation

The original side11 path asks the human-controlled keeper to choose. The tracer now accepts `--host-buttons N` (byte0..255), default128=A. A fixed129=A+Right mask selects the ordinary catch in keeper menu context4; downward input would select action2, which requires a different keeper type. Reports disclose the exact mask supplied to both held and edge bytes on each modeled frame. This is repeated synthetic input, not physical controller sampling or an online command protocol.

With both keeper role fields normalized and opposing field players positioned away from the ball route, the original observation reaches `$C633` restart handoff after238,636instructions and350host frame acknowledgements. The keeper comparison checkpoint is134,088instructions/144host frames. It produces the same inspected catch, target, power, keeper comparison, handoff fields and stamina as the normal native example above. The source report, RAM and original content remain private. No automated differential suite was run.

## A separate missing flight stage

A crowded constructed snapshot places opposing players near the ball route. It reaches the catch handoff after717,165instructions/1,142host frame acknowledgements, but power falls33→30→27→24→21 before keeper comparison. The four extra contacts are against slots1..4, action0, result1, candidate loss3 each. The later original caller applies the loss, unlike the initial selected-defender loop that preserves it.

`flight_contacts` records these separately: comparison entry `$888D`, result/candidate loss at `$88A8`, and power after the handler returns at `$8877`. The crowded keeper ratio is22 rather than35. The current native joined scene does **not** reproduce this live ball-flight/player-selection stage. Its existing isolated loss API does not supply that missing scheduling/selection integration. No fixed loss is inserted to disguise the gap.

## Evidence limits

Linux production compilation and normal CLI usage support this increment; independent review is recorded in the plan/work log. No tests were added or run. Complete match movement, random scheduling, later phases, persistent state, artwork/audio and online service remain unfinished. The original trace still uses explicit frame/display/PPU-queue acknowledgements without full NMI/PPU/APU emulation.

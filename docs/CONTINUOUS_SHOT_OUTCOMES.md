# Continuous original shot outcomes

The research tracer now executes from original attacker entry `$829D` through the immediate keeper outcome handoff. It executes original banked calls, scripts, player movement, comparisons and handlers without substituting no-op subroutine returns. This is a constructed CPU/scheduler observation with explicit host acknowledgements, not a full emulator or native equivalence suite.

## Use and boundaries

```bash
PYTHONPATH=/path/to/py65:tools python3 tools/trace_field_loop.py /path/to/reference.nes \
  --ram /path/to/private-shot-ram.bin --until outcome --ack-ppu-transfers \
  > /path/to/private-analysis/shot-outcome.json
```

Default field/keeper modes retain their endpoints and host policy. Outcome mode retains the existing action0, selected-field-action2/3, side, snapshot, hardware and budget guards. It records keeper comparison without stopping there, then returns before executing the next phase:

| Boundary | Original entry | Next phase |
| --- | --- | --- |
| `kickoff_handoff` | fixed-bank `$C636` | Kickoff after a goal. |
| `restart_handoff` | fixed-bank `$C633` | New possession after a catch or special turnover. |
| `rebound_recovery_handoff` | bank26 `$8BDF` | Field recovery after a rebound. |
| `punch_recovery_handoff` | bank26 `$8BE5`, after keeper comparison | Active-ball recovery after a punch. |

Pre-keeper terminal/diverted routes retain their own boundaries and have no keeper outcome. Budget/hardware failures remain errors. Reports include keeper-stage instruction/frame counts, outcome fields, roster stamina/levels and original watched writes. The defending keeper is selected from the initial side, even after possession changes.

## Goal trace stall

Original `$CCD2` appends three-byte PPU transfer descriptors using queue count `$0498`. The frame-side routine `$C8FB` pops one descriptor and writes PPU data. The harness omitted that consumer; long goal presentation overflowed the queue into script state, including `$0516`. Pulsing A did not resolve it.

Opt-in `--ack-ppu-transfers` acknowledges one pending descriptor per modeled host frame by decrementing `$0498`. All host writes are logged separately, alongside the existing `$0515` acknowledgement and repeated A inputs. The policy does not clear the whole queue. It does not execute PPU payloads, NMI bank effects, audio, interrupt randomness or physical frame timing. Old observations do not silently enable it.

## Original observations

All use side0, ball(160,100), ordinary attacker type1, keeper type33 and previously documented constructed snapshots. Complete gameplay reachability and move availability are not established.

| Scene: attacker level/keeper level/field defenders/subtype | Instructions | Host frames | Immediate result |
| --- | --- | --- | --- |
| 80/20/0/0 | 380,396 | 633 | Goal; score1:0, side11, ball(204,124), keeper timer/penalty/stun0, kickoff4. |
| 20/80/0/0 | 189,048 | 231 | Punch; side0/owner3, ball(200,112), timer7/penalty79/duration4, active1. |
| 20/20/2/3 | 384,912 | 491 | Rebound; side0/owner3, ball(204,124), timer11/penalty79/duration4, active0. |
| 20/20/2/0 | 305,198 | 381 | Punch; same immediate punch fields as above. |

Keeper comparison checkpoints remain84,247;84,261;201,286;201,243 instructions respectively. Queue acknowledgements consume90;12;54;42 descriptors. Attacker stamina is920 for ordinary shots and800 for subtype3; keeper stamina remains1000 under the side0 rule.

Original `$84FA` replaces field choices with one selected keeper. The goal retains count1; punch/rebound clear it. The development scene previously supplied its earlier field count to the outcome stage and now supplies1 at that checkpoint. The general prepared wrapper retains its explicit outcome-state contract; it does not reconstruct the selection list.

## Native evidence and limits

Normal laboratory usage with the private version2 pack produces matching inspected events, power, keeper strength/ratio, score/possession/owner, keeper state, ball position and stamina. The CLI exposes comparison adjustment/score and defender/active/kickoff fields for inspection. This is manual inspection of selected outputs, not an automated complete-state comparison.

Experience, general clock/player/random scheduling, presentation and persistent match commits remain outside the native joined calculation. The later [mirrored catch increment](MIRRORED_SHOT_SCENE.md) adds a continuous side11 human-choice observation and identifies additional live-flight contacts. The tracer still stops before kickoff/restart/recovery itself. No automated tests were added or run for this increment.

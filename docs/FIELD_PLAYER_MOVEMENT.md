# Native field-player movement

`advance_field_player` joins original direction quantization and player-coordinate updates corresponding to `$E7D0/$E854/$E8A0`. It receives full16-bit position words, a target cell/status, raw speed, elapsed byte, current ball cell and replaceable geometry data. This is a player update, not a complete world tick.

## Decisions

- Nonzero status preserves the player. Zero elapsed is a caller-level no-op, matching `$E074`; raw standalone `$E854` with zero elapsed is outside this wrapper contract.
- Direction is computed once. Target240 follows the supplied ball cell; other cells decode to their centers. The quantized direction preserves the original quadrant/equality rules and zero-denominator behavior.
- Direction data consists of64sorted word thresholds ending65535 and8axis signs0/1/2. No original values are bundled. Unused direction/sign data is not inspected on bypassed paths.
- Each iteration checks whether the current integer-position cell is the target (or ball cell for target240), then applies vertical/horizontal projected speed. Axis addition wraps before high-byte-only field clamping; fractional bytes survive.
- The original wrapped status-minus-elapsed sign gate is retained. An active elapsed129 observation performs no coordinate change through129iterations. This is an unusual explicit byte input, not a claim of ordinary game timing.

`calculate_player_direction` is also available independently. `FieldPlayerMoveResult` reports the new state, direction, whether direction was computed, and processed iterations. These counts do not advance a world clock.

## Evidence

Eight private original executions ran actual `$E854` to a synthetic return without skipped calls or hardware/frame interrupts. They include on-ball/off-ball captured positions, target-cell stops, a nonzero status, high elapsed and pure vertical directions. Normal native research queries produced the same six inspected values for these observations: X/Y word, target, status, direction and iteration count. This is manual development inspection, not an automated differential suite.

Representative observations:

| X/Y word in | Speed | Target/status/elapsed/ball cell | X/Y out | Direction/iterations |
| --- | --- | --- | --- | --- |
| 23339/25600 | 1136 | 230/0/2/5 | 25611/25600 | 255/2 |
| 23339/25600 | 1136 | 37/0/2/5 | 21067/23328 | 147/2 |
| 23184/25600 | 981 | 240/0/2/5 | 21222/27562 | 105/2 |
| 24320/25600 | 981 | 62/0/2/5 | unchanged | 62/0 |

## Native research query

`--player-movement-batch` and the stateless native `playermove` query accept X word, Y word, speed word, target, status, elapsed, ball cell,64word thresholds and8sign bytes. They return the six values above. Parsing is bounded and rejects extra tokens. The query does not mutate the ordered contact fixture; the Python transport adapter has not been extended.

Production Make compilation and normal native queries are recorded in the work log/plan. No automated tests were added/run. Team profile speeds, target AI, world ordering and terminal state commits are the next layers; their implementation/evidence is recorded separately.

Those next layers are now implemented for staged interrupt-free field phases in [NATIVE_FIELD_PHASE.md](NATIVE_FIELD_PHASE.md). Full clock/prelude and gameplay scheduling remain separate.

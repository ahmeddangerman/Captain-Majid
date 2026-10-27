# Flight candidates from a position snapshot

`select_flight_defenders` ports the empty-list selector at original `$87E1`. It reuses the20 existing field-player records in IDs1..10 then12..21 order. The caller supplies possession side, current ball cell, frozen random bytes and threshold. No coordinates move and no random state is consumed.

## Decisions

`field_cell_at` follows `$CDE2`: X48..207 and Y80..175 encode as `((X-48)/8)*12 + (Y-80)/8`; all other coordinates return255. Selection scans opposing players, skips nonzero status, and requires exact cell equality. The limit is5 for possession side0 and4 for side11, in scan order.

The original count comparisons leave carry clear before `SBC`, so the sample is the wrapped byte `random_first-random_second-1`. Include a candidate only when sample is strictly below threshold. The same sample applies to the whole snapshot. For explicit invalid states, ball cell255 can match an out-of-field player; this is not a claim that such states occur in normal gameplay.

## Original observations

Six isolated original executions entered `$87E1` with an empty list and returned through a synthetic stack sentinel without skipped calls. The snapshot supplies all positions/statuses; no hardware/frame interrupts execute.

| Side | Layout/random/threshold | Selected IDs | Instructions |
| --- | --- | --- | --- |
| 0 | All in cell62; random0/100, threshold255 | 12,13,14,15,16 | 812 |
| 11 | Same | 1,2,3,4 | 835 |
| 11 | Only1/3 eligible in cell62;5 has nonzero status | 1,3 | 1,198 |
| 0 | All in cell62; random0/255, threshold1 | 12,13,14,15,16 | 812 |
| 0 | Same, threshold0 | None | 827 |
| 11 | Invalid positions and explicit ball cell255 | 1,2,3,4 | 425 |

These are private original observations, not native differential test passes. Cell62 includes(95,100) and(88,103); (96,100) is in a different cell.

## Development scene connection

The existing `--flight-defenders N` option now constructs a position layout with N opposing players in the input ball's cell and all other players in a different valid cell. The selector's returned IDs form the explicit later-contact burst. With the existing staged values0/100 and threshold255, this retains the observed crowded IDs and normal power/catch output. Zero keeps the controlled scene and does not add a burst.

This connects position-based selection for one supplied snapshot. Continuous player movement, updating the ball cell each flight step, multiple contact bursts, interrupt randomness and persistent match timing remain missing. Terminal later-flight handler state is still pending as documented in [flight contacts](FLIGHT_CONTACTS.md). No pack change is needed.

Production builds, normal scene usage and fresh static review are recorded in the work log/plan. No automated tests were added or run for this increment.

# Original terminal later-flight handoffs

The research tracer now recognizes later-flight results2/3 that stop before any keeper comparison. It witnesses a terminal classification at `$88A8`, then returns before bank26 `$8BDF` ball recovery or fixed-bank `$C60F` field-player control. A stale result byte alone cannot select these boundaries. Existing initial field-terminal boundaries remain unchanged.

Both `--until keeper` and `--until outcome` can return these named diversions. `outcome` stays null and the keeper-stage record stays empty when no keeper was reached. `terminal_flight` contains a separate entry snapshot and actual handoff state; `flight_contacts` includes ball cell and ball/player coordinates at comparison entry `$888D`. These are memory checkpoints, not a claim that candidate selection and comparison happened at the same instant.

## Continuous original observations

The crowded side11 RAM snapshot was normalized for both keeper role fields and its attacker level changed. The tracer used fixed129=A+Right, explicit frame/display acknowledgements, opt-in single PPU descriptor acknowledgements, and executed all original calls. Reports and snapshots remain private.

| Attacker level | Power | First flight code | Boundary | Instructions / host frames |
| --- | --- | --- | --- | --- |
| 1 | 11 | 3 | `flight_player_control_handoff` at `$C60F` | 220,746 /260 |
| 5 | 15 | 2 | `flight_ball_recovery_handoff` at `$8BDF` | 207,872 /260 |

Both first compare defender1/action0/subtype0 with candidate loss3. The comparison-entry ball is(52,124), defender position(91,100), possession side11/owner14. Their handoffs differ:

- **Code3:** possession becomes side0/owner1; ball stays(52,124); selected count remains4; ball-active/elapsed alias is0.
- **Code2:** possession stays side11/owner14; ball becomes defender position(91,100); selected count becomes0; ball-active/elapsed alias is2.

The later code3 caller transfers possession even for defense action0. Borrowing the initial defender-handoff API's action0 animation route would be incorrect here. Code2 also copies the defender's comparison-time position, which can differ from the earlier selection snapshot. The timing alias must come from coherent stage state, not an invented constant.

## Native boundary

The native flight calculation currently stops at these classifications and labels a pending handoff without preparing a keeper. Normal level1/level5 laboratory usage illustrates that boundary; it does **not** implement the original handler writes above. These observations supply the next implementation contract.

Next work must carry comparison-time coordinates and phase state, then port the correct later handler and commit its possession/ball/count writes together. Continuous movement, clock/random scheduling and next-phase execution remain separate. No automated tests or native differential suite were added/run in this research increment.

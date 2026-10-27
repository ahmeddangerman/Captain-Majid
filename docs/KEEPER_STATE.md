# Goalkeeper story override and stored state

## Evidence and scope

These helpers were derived from the verified Japanese ROM by inspecting its bytes and executing the original CPU instructions locally. The external disassembly supplied function locations only; its source, comments, assets, and tables are not incorporated.

This increment follows the existing goalkeeper comparison. It implements the story classification override and the two stored keeper state updates used by save handlers. It does not yet execute a complete shot outcome. Goals, possession, ball placement, experience, recovery over elapsed time, scenario scheduling, music, rendering, and the transition to the next game phase require further integration.

## Story override

PRG bank 26, entry `$856B`, takes the comparison classification in CPU register Y and these bytes:

| Native field | Original input |
| --- | --- |
| `comparison_result` | Y |
| `possession_side` | `$05FB` |
| `opponent_team` | `$002B` |
| `story_state` | `$0446` |
| `attack_subtype` | `$043C` |
| `random_first` | `$00E2` |

`apply_keeper_story_override` selects an override when all these conditions hold:

- Possession side is zero.
- Opponent team is 5.
- Story state and attack subtype are nonzero.
- Attack subtype is exactly 3, or story state is less than 4.

An eligible override sets the classification to 2 when the random byte is below 128, or 3 otherwise. The subtype comparison uses the entire byte: subtype 131 does not satisfy the test for subtype 3. The `overridden` output reports whether the override branch was selected, including when its result agrees with the incoming classification.

The original code then stores the classification in `$0612` through `$8148` and clears `$0616`. The native result contains `result_code`, `overridden`, and `defender_index`, which is always zero. The CPU oracle stops at `$8599`, before scenario scheduling. This boundary does not establish a final goal or save.

## Stored keeper state

`update_keeper_state` operates on an explicitly supplied keeper record. The original record belongs to the team without possession, selected by `$C551`. For valid possession sides 0 and 11, the Japanese ROM points respectively to records `$0384` and `$0300`.

| Native field | Original input |
| --- | --- |
| `stored_timer` | keeper record +5 |
| `stored_penalty` | keeper record +7 |
| `stored_penalty_duration` | keeper record +6 |
| `timer_selector` | `$0443` |
| `random_first` | `$00E2` |
| `invoke_timer` | caller chooses whether to execute `$86BD` |
| `invoke_penalty` | caller chooses whether to execute `$86D3` |

When `invoke_timer` is true, add 7 to the stored timer if `timer_selector` is below 2, otherwise add 11. Keep only the low byte. The original routine wraps; it does not saturate the timer.

When `invoke_penalty` is true, the penalty update is eligible only if `random_first` is below 64 and the stored penalty is below 80. An eligible update stores `min(stored_penalty + 79, 127)` and sets the penalty duration to 4. An ineligible update preserves both bytes. Disabled helpers preserve their corresponding state. The two updates change independent fields, so combining them does not change the original handler ordering.

Outputs are `timer`, `penalty`, and `penalty_duration`. Helper invocation is the caller's responsibility until the shot outcome flow is integrated. This API does not infer invocation from a comparison classification.

## Original handler context

The classification dispatch begins after `$8599` and its scenario call. Relevant branches are:

| Handler entry | Consequences relevant to these helpers | Further work outside this module |
| --- | --- | --- |
| `$85AC` | Neither state helper runs. | Experience, goal increment, scenarios, possession change, kickoff setup, and clearing the conceding keeper's +5/+7/+10 fields. |
| `$8605` | Neither state helper runs. | Mask attack subtype with 63; values below 3 enter `$85AC`, otherwise possession changes and play restarts. |
| `$861C` | Calls `$86D3`, then `$86BD`. | Experience, defender count reset, optional ball placement when defender index is nonzero, and field recovery. |
| `$8646` | Calls `$86BD`. If defense action `$043D` equals 1, later calls `$86D3` as well. | Experience, possession transfer for other defense actions, or randomized deflection placement for action 1. |

The penalty duration's later countdown and penalty reduction occur in a separate fixed-bank routine beginning at `$D263`; this increment preserves that byte without implementing the countdown.

## Oracle checkpoints and validation

Map PRG banks 26/27 at `$8000/$A000` and fixed banks 30/31 at `$C000/$E000`. The isolated checks need no PPU, audio device, interrupt, frame loop, or rendering execution.

- Story: enter `$856B` with Y set, stop before `$8599`, and observe `$0612/$0616`. Research checked 526,332 cases, including every story-state/subtype byte pair for each initial classification and both random sign bits under the eligible side/team, plus the side/team gates.
- Timer: enter `$86BD` with a normal subroutine return sentinel at `$0200`; observe keeper +5 after return. Research checked every timer/selector byte pair for both valid sides: 131,072 cases.
- Penalty: enter `$86D3` with the same sentinel; observe keeper +6/+7 after return. Research checked every random/penalty byte pair for both valid sides, varying the previous duration: 131,072 cases.

These research runs compared the stated contracts with original CPU writes. Native unit checks cover the branch boundaries, unchanged fields, raw subtype handling, and byte wrapping; the private differential verifier compares native outputs against these original routines. CPU flags and scratch pointers are outside the public result contract.

The prepared `tools/verify_shot_state.py` corpus contains 6,812 story cases and 5,632 keeper-state cases. A completed native differential run for this increment is still pending; the exploratory contract checks above are separate evidence.

## Research interfaces

- `--keeper-story-batch`: six bytes per line in `StoryOverrideInput` field order; returns classification, override flag, defender index.
- `--keeper-state-batch`: stored timer, stored penalty, stored penalty duration, timer selector, random byte, invoke timer (0/1), invoke penalty (0/1); returns timer, penalty, duration.

The loopback replica adapter exposes the same calculations as `story` and `keeperstate` queries. These interfaces supply explicit data and do not schedule random generation or apply game transitions.

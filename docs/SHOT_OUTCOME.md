# Shot outcome at the goalkeeper handoff

The verified Japanese ROM chooses a goalkeeper result code, applies a possible story override, then dispatches through the four branches at `$85AC`, `$8605`, `$861C`, and `$8646` in PRG bank 26. The native `resolve_shot_outcome` reproduces the immediate score, possession, keeper state, and ball-position writes through each branch's handoff to the next match phase. The original bytes were inspected directly. No external source, ROM table, image, or audio asset is bundled.

| Keeper code | Condition | Immediate result |
| --- | --- | --- |
| 0 | Always | Goal: increment the shooting team's goal byte, flip possession, clear the defending keeper's timer, penalty, and stun, set kickoff flag to 4. |
| 1 | `(attack_subtype & 63) < 3` | Same goal branch. |
| 1 | Otherwise | Flip possession and hand off to the restart phase. |
| 2 | Always | Rebound: clear defender count, update keeper timer and possible penalty, continue into field recovery. |
| 3 | Defense action is not 1 | Keeper catch: update timer, flip possession and set ball owner to the new side. |
| 3 | Defense action is 1 | Keeper punch: update timer and possible penalty, place the ball using explicit random bytes and eight caller-supplied intervals, set ball-active flag, clear defender count, continue field recovery. |

The story override can change codes 0–3 before this dispatch. The score counters are bytes and wrap at 255. The goal handler leaves penalty duration unchanged while clearing the other keeper fields. Rebound and punch timer changes wrap by one byte. The native function keeps all fields that the observed branch leaves untouched.

The caller supplies the eight punch intervals. Their values are read from the user's private ROM by the differential verifier; the engine does not bundle them. Zero for the interval selected by a punch is rejected because the original loop would not terminate. The result is a handoff state, before later field recovery, player experience, animations, music, and scenario logic.

`resolve_explicit_shot` links four existing calculations: energy use determines weakening; weakening affects action power; action power enters goalkeeper comparison; its classification and subtype enter this outcome dispatch. Player statistics, move costs, and random bytes remain explicit inputs because the complete content selector and frame schedule have not been reconstructed. Field defender contacts are also not in this sequence. This is a deterministic shot calculation with explicit data, not yet a playable match command.

`tools/verify_shot_outcome.py` checks 384 deterministic cases against isolated original instructions. It executes the original story decision and branch arithmetic, substituting no-op returns only for experience, sound, and scenario calls outside this immediate match-state handoff. It compares 15 observable values: event, result code, possession, ball owner, goals, keeper state, ball position, defender count, ball-active flag, and kickoff flag. The original-ROM cases are not complete gameplay traces.

The development `--outcome-batch` mode accepts 29 bytes per line: 21 `ShotOutcomeInput` scalar fields in header order followed by the eight punch intervals. It returns the 15 output fields in `ShotOutcomeResult` header order. The `outcome` replica query uses the same fields and does not mutate the ordered fixture. Event codes are 0 goal, 1 special turnover, 2 rebound, 3 keeper catch, and 4 keeper punch.

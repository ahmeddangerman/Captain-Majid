# Loose-ball recovery

## ROM behavior

The result-code-2 branch enters `$8BDF`. After its random-generator call, `$8C6D` nudges the ball's X and Y coordinates using the two current random bytes. In the ordinary interior field, the routine then checks the selected defenders at `$8C42`:

- A field defender with defense action 5 takes the ball when `$E2 < $40`. The first matching defender in the selected list wins. Goalkeepers (IDs 0 and 11) are skipped.
- Otherwise, the game searches each team for an available field player. It skips the current ball owner, the opposing player, and records whose +10 flag is set. It scans player IDs 1–10 and 12–21 in order, expanding a square search radius by eight until it finds a player for each side.
- The smaller radius wins. A tie goes to side 0 when bit 7 of `$E2` is clear and to side 11 when it is set.
- At `$8C12`, the chosen player becomes the ball owner and sets the possession side. The ball coordinates are copied to that player's record and the attack subtype is cleared.

The ROM's normal-field coordinate check covers X `$30–$CF` and Y `$50–$AF`. Outside this area, separate goal-line and restart handling takes over.

## Native implementation

`recover_ball` reproduces the ordinary interior-field path when the ball-update flag `$05FF` is clear. Its input supplies the two post-generator random bytes, the selected defender list, the ball owner/opposing player, and the 20 field-player records in ROM order: IDs 1–10 followed by 12–21. It returns the jittered ball coordinates, recovered owner and side, cleared subtype, and whether the direct defender capture occurred.

When connected to `resolve_defender_outcome`, result code 2 still identifies the recovery route. Supplying a recovery fixture also applies the recovered owner and possession side. Without that fixture, the outcome remains an unresolved recovery handoff.

The helper does not generate the random bytes, move an active ball, or handle boundary restarts and presentation. Those parts depend on surrounding match state and remain separate.

## Comparison evidence

`tools/verify_ball_recovery.py` matched 512 deterministic cases against the verified Japanese ROM, starting after the original random-generator call and stopping at `$8C2E`. It compares the native handoff output and confirms that the original routine copies the returned ball coordinates into the selected player's record. The native helper returns these coordinates for its caller; it does not mutate a player roster. The ROM input is supplied privately; it is not stored in the repository.

The research command `--ball-recovery-batch` accepts: ball X/Y, current owner, opposing player, `$E2`, `$E3`, active-ball flag, defender count; then 20 `(X, Y, unavailable)` player records; then the selected defenders as `(player ID, defense action)` pairs. It returns jittered X/Y, owner, side, subtype, and direct-capture flag.

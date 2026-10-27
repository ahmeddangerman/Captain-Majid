# Player movement foundation

`advance_field_coordinate` implements the original `$E912` coordinate step. It accepts a raw16-bit coordinate and displacement, adds with word wrap, then clamps only the high byte: X48..207 or Y80..175. The low fractional byte is retained even if the integer coordinate clamps. A conventional clamp of the entire word would produce different results.

`FieldAxis::horizontal/vertical` selects the bounds; invalid enum values reject. Displacement is a raw word bit pattern, so65535 means an addition of-1 modulo65536 without relying on signed overflow or implementation-defined signed conversion. CPU flags and scratch registers are not outputs.

## Original observations

Six isolated original executions entered `$E912` with A/X holding the displacement bytes, Y5/7 selecting X/Y, a supplied player-record pointer and a synthetic stack return. No calls were skipped and no hardware/frame interrupts ran.

| Axis | Coordinate | Displacement | Result |
| --- | --- | --- | --- |
| X | `3001` | `FFFF` | `3000` |
| X | `3000` | `FFFF` | `30FF` |
| X | `CFFF` | `0101` | `CF00` |
| Y | `5000` | `FFFF` | `50FF` |
| Y | `AFFF` | `0100` | `AFFF` |
| X | `F000` | `2000` | `3000` |

Values in the table are hexadecimal raw words. The last case demonstrates wrap-before-clamp with a constructed out-of-field input; it does not establish normal gameplay reachability. Reports remain private and are not automated native differential passes.

## Development query

`captain-prototype --field-axis-batch` accepts decimal coordinate, displacement and axis0/1 per line, returning the decimal result. The native replica also accepts stateless `fieldaxis` queries. They do not change the fixture's state/sequence; no new game/network command is implied. The Python loopback adapter has not been extended for this query.

```text
12288 65535 0
```

returns12543 (`30FF`). Both word inputs are bounded0..65535, axis0/1, with no extra tokens. Native API callers can use the same helper without the research protocol.

## Remaining movement and phase work

Original `$E8A0/$E912` shows field X at record+5/+6 and Y at+7/+8: fractional low byte followed by integer high byte. Keeper roles reuse these offsets differently and must not be represented as field positions. `$E854` also depends on status, direction/speed, target cell and elapsed scheduling; this helper does not port that whole routine.

Additional private terminal-phase observations show why possession alone is insufficient. The code3 branch flips side/owner before `$C606→$E074`; that player update changes position/target-related bytes in nineteen field records and clears the elapsed alias. The old attacker's status also changes during the handler's preceding processing. Code2 reaches recovery with elapsed2. Random bytes remain0/100 in these particular observations; that is not a general random-scheduling guarantee.

A further private run records38original `$E854` movement entries across the observed player updates before the terminal handoff. Each records the actual speed word, elapsed input, status, target and full position words; the first direction is captured after `$E7D0`. Several final entries use elapsed2, speed981, target240 and direction108. These are captured inputs for the next movement port, not a universal speed/direction rule. The native coordinate helper does not yet consume that whole context.

Next implementation needs coherent position words, targets/statuses, stage timing and ball state alongside the existing roster. It must apply the original player update and terminal/restart/recovery transition together before committing persistent match state. The current native terminal flight remains pending.

Production builds, normal queries and static review provide limited evidence. No automated tests were added/run; no playable movement, AI, full match or online service is claimed.

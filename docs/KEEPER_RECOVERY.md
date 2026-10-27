# Goalkeeper recovery over elapsed game time

`advance_keeper_recovery` implements the state decisions of the original fixed-bank `$D235/$D263` caller. It takes explicit state and one elapsed byte, returns new state, and does not schedule time or change its input. No ROM or original tables are included in the native helper.

## State

`KeeperRecoverySnapshot` contains keepers in slot0/11 order and the shared flags/timer at `$0449/$044A`. Each `KeeperRecoveryState` contains timer(+5), penalty duration(+6), penalty(+7), and stun(+10). These role fields are not keeper coordinates.

- With shared flag bit7 set, subtract elapsed from the shared timer with byte wrap. If the resulting bit7 is set, clear the entire shared flag byte. Timer zero alone does not clear it.
- For a keeper with nonzero stun, subtract elapsed with byte wrap and replace a result with bit7 set by zero. Stop this keeper's update here, even if stun becomes zero.
- Otherwise, a zero penalty leaves duration unchanged. A nonzero penalty subtracts elapsed from duration with byte wrap. If the resulting bit7 is set, add3 to duration with wrap and subtract25 from penalty with wrap, replacing a penalty result with bit7 set by zero.
- The stored timer(+5) never changes in this caller. A penalty reduces at most once per call; this is not a catch-up loop.

The sign-bit rules intentionally preserve unusual explicit byte inputs. They are not equivalent to clamping mathematical subtraction at zero for all0..255 values. For example, stun1 with elapsed200 becomes57 after byte wrap. Such observations do not establish those inputs as ordinary gameplay states.

## Original observations

Seven isolated original executions entered `$D235` with a synthetic stack return and reached that return without skipped calls. They use fixed banks30/31 and no hardware, frame interrupts or rendering. Representative outputs:

| Elapsed | Keeper input: timer/duration/penalty/stun | Output |
| --- | --- | --- |
| 1 | 9/0/79/2 | 9/0/79/1 |
| 1 | 10/0/79/0 | 10/2/54/0 |
| 3 | 9/0/79/2 | 9/0/79/0 |
| 5 | 10/0/79/0 | 10/254/54/0 |
| 200 | 9/0/79/1 | 9/0/79/57 |

The existing bounded loop tracer now records each recovery entry and its real return at `$D1A0`. The constructed side0 two-defender shot reaches keeper comparison after four elapsed calls: **1,1,2,1**. Its original zero-state observation retains201,243instructions. A second snapshot initialized both keeper timers9/penalties79/durations0, slot0 stun2 and shared flags128/timer3. It reaches the same boundary after201,330instructions with:

- Slot0: timer9, duration0, penalty54, stun0.
- Slot11: timer9, duration1, penalty29, stun0.
- Shared flags0/timer255. The keeper comparison reads penalty29 and produces adjustment29, ratio80, score109, raw result3.

These are private original-code observations, not native differential passes. Host frame/display acknowledgements and all prior tracer limits still apply.

## Native development scene

The resettable shared scene accepts initial keeper state and an ordered pre-shot recovery list. For example:

```bash
./build/captain-shot-lab /path/to/private.cmjpack \
  --keeper-timer 9 --keeper-duration 0 --keeper-penalty 79 \
  --recover 1 --recover 1 --recover 2 --recover 1
```

Each `--recover` appends one step. Interactive commands use the same names without `--`; `clear-recovery` clears the list. Every shot replays the list from the supplied initial state, then supplies the resulting timer/penalty to comparison and the same duration/stun to outcome and the keeper-status gate. It does not persist a previous shot's result. The state options refer to the selected opposing keeper: slot11 for side0, slot0 for side11. The other keeper/shared state remain zero in this scene; the pure snapshot helper supports both keepers and shared state.

An elapsed sequence matters: a single elapsed5 call is not equivalent to1,1,2,1. Stun recovery and penalty recovery follow separate branches, and penalties reduce at most once per call.

The shot distance input is separate. This increment does not subtract recovery elapsed from the shot's distance adjustment or infer time from rendering. There is still no complete match clock, player movement schedule, next-phase simulation or general gameplay random schedule. Production compilation and normal laboratory usage provide limited native evidence; automated tests were not added or run.

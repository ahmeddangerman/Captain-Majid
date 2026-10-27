# Field loop scheduling and power-loss boundary

## What was inspected

Direct instruction inspection of the private Japanese ROM covered bank26 caller `$8385..$83D4`, helpers `$9085/$9095/$90DD`, fixed-bank script entry `$C54E/$CBB0`, coroutine scheduler `$CB0F/$CAA5`, time helper `$C603/$D193`, and script execution in banks16/17. This was static inspection, not a captured gameplay run or differential comparison. No original code or tables are included here.

## Confirmed boundaries

- After comparison and the possible result4 override, the caller invokes `$9095` with different carry for results0/1 versus terminal results. For field players, its result-dependent branch writes either defender record+10 or attacker record+10, then delegates to the time helper. Keeper slots skip those field-player writes.
- `$C603` is a time/state helper, not an event callback registration function. It adds its argument to `$05FF`, changes clock state and updates keeper status/recovery state. This work can affect later route gates even when no shot power changes.
- `$C54E` selects a scenario script through `$CBB0` and yields via `$CB0F` to the cooperative scheduler. The scheduler saves/restores stack and PRG-bank context. A direct CPU-only trace cannot silently skip this call and claim full-loop equivalence.
- Fixed-bank `$EC08` handles the pending script flag, maps banks16/17 and enters their script engine. Its initial table selector reads the pending script ID at `$0518`. Thus reading script bytes using the gameplay bank mapping would inspect the wrong content.
- The loop calls scripts5/6 and7, then checks the result and opposing keeper action before advancing `$0616` or leaving. An exhausted nonterminal loop calls `$9085`, which delegates to the time helper again.

## Initial power-loss question (before scheduler observations)

The existing loss routine at `$819F..$81B8` writes `$061C/$061D`. It is reachable from the inline result tables after `$814C` and `$88A8`; these are distinct action paths. The inspected straight-line main defender caller has no direct call to that loss routine.

That observation does **not** prove the complete main loop preserves power: script execution, indirect calls, other banks, interrupts and subsequent continuation have not been traced as one execution. A byte-pattern scan for direct word stores is only a locator and is not exhaustive evidence of all writes.

At this initial boundary, the native `calculate_defender_contact` and prepared sequence both applied their previously isolated code1 loss behavior. The scheduler observations below subsequently distinguish the two caller contracts and correct the prepared sequence.

## Next trace

Capture the selected script/coroutine effects and all writes to the shot-power word across two successive defenders, including status/time updates, then compare against the native sequence. Keep gameplay random scheduling separate from replaceable music and presentation. Do not stub script calls as presentation-only without checking their state effects.

## Parser observations (2026-10-27)

`tools/trace_field_script.py` now runs the actual banks16/17 selector and parser with watched RAM writes. It requires an explicit internal RAM snapshot and verified private ROM. It reports its host actions: script/bank initialization, synthetic stack/entry registers, and acknowledgement of presentation requests. It does not execute presentation, scheduler or frame callbacks; its results cannot establish what those omitted phases do.

Two constructed snapshots were captured after the actual defender loader, defensive energy call and `$834C..$8385` selection/comparison/interruption path. Setup: owner3, defender14, primary type1/level20 and stamina1000 for each record; attack/defense actions0, subtypes0, situation0, side0, two selected defenders, random bytes100/100, ball(120,100), and explicit incoming power23 or1. Calls to the real loader/energy/comparison were separate bounded calls with synthetic returns, not a continuous gameplay capture. No original content was added to the repository.

| Original result | Script | Original parser instructions | Presentation requests | Power before → after | Writes to power/result bytes |
| --- | --- | --- | --- | --- | --- |
| 1 | 6 | 332 | 0 | 23 → 23 | None |
| 1 | 7 | 358 | 0 | 23 → 23 | None |
| 3 | 5 | 207 | 0 | 1 → 1 | None |
| 3 | 7 | 608 | 1 | 1 → 1 | None |

Each script starts independently from its constructed post-comparison snapshot. These are four observations of the original parser, not four native-engine test passes. The comparison's rolled strength was13 and candidate loss3 in both snapshots. The native isolated code1 contact would apply that loss immediately; this partial original path has not done so at the inspected boundaries. That is a question for the full caller trace, not enough evidence to alter the main-loop rule.

Continuing the original caller from `$8385` reached `$C603/$D193`, then `$EF7F`, which maps banks24/25 and calls their `$800C` entry (`$8851`). Inspection shows this path yields through `$C515` while updating display data. The CPU-only attempt reached scheduler frame wait `$CAAE` and was stopped at its instruction bound; it did not complete the clock helper or reach the caller's next script. No call was skipped to produce a false successful loop trace.

Private snapshots and JSON are stored only in the sibling analysis directory. No native tests or ROM differential suites were added/run in this increment. Next, capture or model that scheduler/display boundary with explicit evidence before resolving complete power-loss timing.

Fresh review identified a snapshot precondition: original bank helper `$CE2D` preserves mapper-mode shadow `$22`. The tool now rejects PRG mode bit6 set and initializes select/shadow `$23` consistently for mode0. The observed snapshots all used `$22=0`. This correction has static review evidence; no behavioral regression test was run.

## Selected-defender loop through the scheduler (2026-10-27)

`tools/trace_field_loop.py` executes the original `$82F4..$83F5` selected-defender caller, using its actual loader, comparison, interruption, clock, display and script calls. A constructed scheduler has foreground game task5 and background presentation task1 at `$EB86`. Original code can start its other tasks. No subroutine calls are skipped.

At original frame wait `$CAAE`, the host marks the frame flag, acknowledges the display queue by clearing `$0515`, and supplies repeated A input at `$1C/$1E`. These writes are recorded separately from original instruction writes. NMI, real PPU/APU timing, sound, rendering and hardware/controller sampling are not emulated; random bytes stay at their explicit constructed inputs unless original CPU code changes them. Unsupported hardware still fails closed. This is a bounded original CPU/scheduler experiment, not a full original gameplay capture.

Snapshots use the earlier ordinary player/profile setup, action0, subtype0, situation0, random100/100, ball(120,100), primary type1/level20/stamina1000, and clock456. Defender slots14 upward for side0 and3 upward for side11 are supplied in the original five-entry choice arrays. Entry is after attacker/pre-loop setup, so supplied power is explicit and the tool accepts1..5 selected defenders.

| Side | Defenders | Input power | Comparison results | Original instructions | Host frame acknowledgements | Final power | Elapsed/clock |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | 1 | 23 | 1 | 9,165 | 9 | 23 | 3 / 453 |
| 0 | 2 | 23 | 1,1 | 15,030 | 15 | 23 | 4 / 452 |
| 0 | 5 | 23 | 1,1,1,1,1 | 32,611 | 33 | 23 | 7 / 449 |
| 11 | 2 | 23 | 1,1 | 15,066 | 15 | 23 | 4 / 452 |
| 11 | 5 | 23 | 1,1,1,1,1 | 32,701 | 33 | 23 | 7 / 449 |
| 0 | 2 | 150 | 0,0 | 15,046 | 15 | 150 | 4 / 452 |
| 0 | 2 | 1 | 3 (terminal) | 41,334 | 70 | 1 | 2 / 454 |

All seven observed paths reach result dispatch and perform **no writes** to `$061C/$061D`. The result1 rolled strength is13 and its candidate loss3. The previous native prepared sequence would reduce23 to20 then17; the original selected-defender caller preserves23 across both contacts. This is sufficient to separate that caller's comparison from the unrelated isolated loss continuation. The prepared sequence now uses `compare_defender_contact`; the existing isolated API retains its old loss behavior.

These are original-code observations, not native differential test passes. The production target compiles after the correction; existing expectations were aligned but no automated tests were added/run. A full shot still requires attacker/pre-loop setup, post-loop player movement, keeper choices/comparison and final phase effects together. Do not extend the seven observations to every action, alternate form or random schedule.

## Continuous attacker-to-keeper mode

The observer's `--until keeper` mode now starts at `$829D`, after decisions and before the actual attacking loader/energy/power call. It executes pre-loop scripts, field contacts, post-loop player movement, shot approach, goalkeeper choice, keeper loader/energy and keeper comparison up to `$856B`. Stage outputs include actual random bytes, costs/stamina, power, keeper range score and classification. Terminal field results or diverted continuations return named boundaries without claiming a keeper attempt.

Complete mode restricts selected field actions to2/3. The earlier field action0 snapshots enter after pre-loop setup and do not establish that choice's gameplay legality: script2's field-action dispatch has no supported branch for that constructed choice. That unsupported full-entry attempt stopped on a guarded hardware-range read; no result was accepted. This corrects the scope of the earlier loop experiments.

Four distinct side0 original observations reach keeper comparison, using the same constructed roster/clock/random setup, ball X160 instead of120, valid pre-loop field action2, and opposing keeper primary type33. The attacker's level/subtype and keeper level vary below. Player X/Y positions are explicit: field players(160,100), keepers(16,128)/(240,128).

| Defenders | Attacker level/subtype | Keeper level | Instructions | Host acknowledgements | Shot power | Field results | Keeper ratio/result |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2 | 20 / 0 | 20 | 201,243 | 215 | 33 | 1,1 | 80 / 3 |
| 0 | 20 / 0 | 80 | 84,261 | 65 | 33 | none | 35 / 3 |
| 0 | 80 / 0 | 20 | 84,247 | 65 | 224 | none | 255 / 0 |
| 2 | 20 / 3 | 20 | 201,286 | 215 | 58 | 0,0 | 140 / 2 |

Keeper action1 is selected in these observations. Keeper random bytes are(0,100), after starting attacker/field bytes(100,100): original post-loop player movement consumes random state even without NMI. The distance adjustment input is45 at `$062B`; it is distinct from elapsed loop time at `$05FF`. These values must not be silently replaced with the initial random pair or clock delta in native composition.

A side11 attempt exhausted the instruction bound before keeper comparison. A separate attempt to continue side0 through the later goal/save scenario also exhausted its bound. Neither is a successful full shot outcome. The current evidence connects preparation/comparison stages on observed side0 paths, with host frame actions disclosed; it does not yet validate the native joined wrapper as a whole or provide a playable interaction.

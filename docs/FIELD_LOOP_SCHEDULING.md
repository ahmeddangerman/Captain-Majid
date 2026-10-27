# Field loop scheduling and power-loss boundary

## What was inspected

Direct instruction inspection of the private Japanese ROM covered bank26 caller `$8385..$83D4`, helpers `$9085/$9095/$90DD`, fixed-bank script entry `$C54E/$CBB0`, coroutine scheduler `$CB0F/$CAA5`, time helper `$C603/$D193`, and script execution in banks16/17. This was static inspection, not a captured gameplay run or differential comparison. No original code or tables are included here.

## Confirmed boundaries

- After comparison and the possible result4 override, the caller invokes `$9095` with different carry for results0/1 versus terminal results. For field players, its result-dependent branch writes either defender record+10 or attacker record+10, then delegates to the time helper. Keeper slots skip those field-player writes.
- `$C603` is a time/state helper, not an event callback registration function. It adds its argument to `$05FF`, changes clock state and updates keeper status/recovery state. This work can affect later route gates even when no shot power changes.
- `$C54E` selects a scenario script through `$CBB0` and yields via `$CB0F` to the cooperative scheduler. The scheduler saves/restores stack and PRG-bank context. A direct CPU-only trace cannot silently skip this call and claim full-loop equivalence.
- Fixed-bank `$EC08` handles the pending script flag, maps banks16/17 and enters their script engine. Its initial table selector reads the pending script ID at `$0518`. Thus reading script bytes using the gameplay bank mapping would inspect the wrong content.
- The loop calls scripts5/6 and7, then checks the result and opposing keeper action before advancing `$0616` or leaving. An exhausted nonterminal loop calls `$9085`, which delegates to the time helper again.

## Power loss remains unresolved for this caller

The existing loss routine at `$819F..$81B8` writes `$061C/$061D`. It is reachable from the inline result tables after `$814C` and `$88A8`; these are distinct action paths. The inspected straight-line main defender caller has no direct call to that loss routine.

That observation does **not** prove the complete main loop preserves power: script execution, indirect calls, other banks, interrupts and subsequent continuation have not been traced as one execution. A byte-pattern scan for direct word stores is only a locator and is not exhaustive evidence of all writes.

The native `calculate_defender_contact` and prepared sequence still apply their previously isolated code1 loss behavior. This increment does not change that behavior or claim it matches the main caller's exact timing. A complete-shot comparison must resolve this before the calculation becomes an ordered match command.

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

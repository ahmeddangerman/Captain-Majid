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

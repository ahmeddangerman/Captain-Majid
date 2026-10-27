# Field result4 interruption

`resolve_field_interruption` reproduces the field caller's post-comparison override and immediate reset. `DefenderRoute::restart_decision` handles code4 separately from field capture. The joined shot can enable this step by supplying `FieldDefenseContext::interruption`, containing current ball coordinates, player statuses and keeper penalties.

## Rule and writes

Original `$8E33` skips the override when the selected list is empty, keeper action is4, or a field defender action is5/6. It preserves nonzero results. For result0, randomE2 at most63 changes result to4; the threshold is15 when Y is `$60..$9F` and X is below `$50` or at least `$B0`. Both comparisons are inclusive at the random threshold.

The actual call `$C55A` jumps to `$CF4F`, which clears player record+10 for all22 slots and record+7 only for keepers0/11. It is a state reset, not an opaque presentation call. Nonkeeper record+7 bytes are preserved.

Result4 dispatches to `$9366`: it clears situation `$044E`, checks the same position region and uses the possession side to select `$94CF` for the opposing goal area or `$9379` otherwise. These are returned as `goal_area_restart` and `field_restart`. Their later setup/presentation and player-choice behavior remain unimplemented; no further possession change is inferred at this boundary.

## Connected state

With explicit interruption context, the prepared contact loop applies the override after ordinary comparison. Each visited contact retains its raw comparison code alongside the interruption result. The overall effective code4 stops the loop, skips keeper preparation and returns `restart_decision`. `interruption_state` carries the updated status/penalty state; the returned roster retains already-applied stamina. The restart decision carries situation0 for the later match caller to commit. Caller inputs remain unchanged.

Absent explicit interruption context, the older research fixtures continue their existing ordinary0..3 calculation. This preserves their established contracts without inventing missing state. The field-only setup's action5/6 bypasses, main-caller power-loss timing and full restart phases remain further work.

## Evidence

The private `verify_field_interruption.py` comparison matched **27,904 constructed fixtures**:1,173 actual resets,5,571 field routes and1,150 goal-area routes. It executes `$8E33` and the real `$CF4F` reset through return without skipped calls, then executes `$9366` only to the first route boundary before presentation. It checks the precise write footprint, bypasses, nonzero results, raw coordinate/random boundaries and both sides. These are routine fixtures, not captured full matches.

Native tests cover reset and preserved input, existing4 without repeated reset, route4 ownership preservation, and a joined shot that stops before invalid keeper data. Run the private oracle with py65 and the approved ROM; all game bytes stay outside Git.

# Fully prepared shot calculation

`resolve_complete_prepared_shot` runs attacking preparation, ordered field contacts, their handoff, continuation gates, keeper preparation and immediate goal/save outcome. It copies a supplied roster and returns the updated roster at `result.flow.field.roster`. The caller must commit that roster together with the returned possession/ball/score writes; this API does not modify a persistent match session.

## Order

1. Validate action0 and a field ball owner on the possession side. Accept at most ten selected field defenders.
2. Resolve attacker record from that roster, load its selected move, debit according to the attacker-side rule, and roll power. Derive selected-defender count from choices. Replace stale context power/subtype with the attacker result.
3. Store attacker stamina in the working roster, then run prepared contacts and field recovery/possession handoff. Stop on terminal field results; no keeper data is inspected on that path.
4. Use the existing continuation gates. Active-defender, field-proximity and keeper-status diversions do not load/debit a keeper.
5. For an eligible attempt, select keeper0/11 from the possession side, read its record from the post-contact roster, load its move/profile, debit energy, and compare against remaining shot power.
6. Resolve goal/save using the keeper result, attacking subtype, selected keeper action, actual side/owner, and the comparison's timer/penalty and current random bytes. Store keeper stamina in the returned roster.

`result.attack` reports attacker preparation. `result.flow` reports field calculation/handoff and optional continuation. `result.keeper` exists only for a reached keeper attempt. A field or diverted route preserves those earlier phase outputs without inventing a goal/save.

## Reused APIs

`resolve_prepared_field_handoff` extracts the field-only phase from `resolve_prepared_field_shot`. `select_action0_shot_route` extracts route decisions from the former action0 continuation helper. Existing entry points keep their earlier behavior, verified by their regression tests.

Random values, post-loop gates, recovery records and initial ball/score/keeper state remain explicit. Selected opposing defenders still come from the caller. The caller may supply field comparison tables once at `input.field.context.comparison_content`; the prepared sequence then selects each reached defender's coefficients, eligibility and thresholds from actual actions/slot. Omitting this content retains per-choice research values. The wrapper carries keeper-comparison random bytes into its outcome; a punch-placement regression failed before that connection and now verifies the expected coordinates.

## Evidence boundary

Native integration tests cover both sides' stamina rules over one roster, goal/catch/punch, stale fixture replacement, terminal and diverted keeper bypass, invalid owner/action, reached keeper errors and input preservation. Component ROM oracles remain separate. This is a joined calculation over constructed inputs, not an original full-match trace or playable match command. Distance effects, runtime content import, deterministic random scheduling, rewards/presentation, boundary restarts and canonical session commits remain further work.

# Action-0 keeper handoff design

## Intent

Connect the verified action-0 continuation after field contact to the existing goalkeeper comparison and goal/save outcome code. A keeper attempt is reached only when the post-loop active-defender count is zero, the field-proximity branch does not divert play to the loose-ball path, and the opposing goalkeeper's +10 status field is zero.

## Goals

- Preserve the ROM's gates before calling the keeper calculation.
- Pass the post-contact attack power and subtype into the existing keeper and outcome calculations.
- Return goals, rebounds, catches, or punches from the existing outcome handler when the keeper route is reached.
- Keep player selection, skill/content tables, and random bytes explicit.

## Non-goals

- Do not resolve action 1, 2, 3, or 4 continuations here.
- Do not infer player statistics, content tables, contact selection, random scheduling, field movement, animations, or restarts.
- Do not claim this is a playable match or an end-to-end original-gameplay trace.

## Approaches

1. Add the route gates to `resolve_explicit_shot`. This would make its current direct-to-keeper contract ambiguous.
2. Add a separate continuation helper that accepts the already-computed post-contact power/subtype and explicit ROM gate values. **Chosen:** it preserves the existing helper while connecting the confirmed action-0 route to the verified keeper/outcome stages.
3. Build a full field-contact and match command now. That needs unresolved selectors and phase transitions, so it would require guesses.

## Contract

The helper accepts only field result codes 0/1 and action code 0 for the supported keeper route. It returns a non-keeper route when the ROM gates divert play. If all gates allow the keeper attempt, it calls the existing keeper comparison and applies its result through the existing outcome handler. Other input values and calculations remain caller-supplied.

## Constraints

- C++17; preserve Windows/Linux portability.
- Keep ROMs, extracted tables, game assets, and translated content out of the repository.
- Continue using original-ROM comparisons for supported arithmetic and handoffs.

# Field-contact handoff design

## Intent

Join the ordered field-contact calculations to the already reconstructed result-code 2/3 handoffs. A caller should be able to submit its selected contacts and current match values once, then receive both the contact sequence and the matching continuation result.

## Goals

- Run `resolve_field_contacts` in caller-supplied defender order.
- Dispatch the resulting code through `resolve_defender_outcome`, including optional ordinary ball recovery for code 2.
- Preserve code 0/1 as `continue_action` for a later action-specific continuation.
- Return both stages so callers can inspect contact calculations and possession/owner results.
- Keep selected defenders, statistics, random bytes, recovery roster, and match state as explicit inputs.

## Non-goals

- Do not select defenders, players, skills, or threshold tables from ROM data.
- Do not combine this wrapper with the action-0 keeper continuation or action-1/other action paths.
- Do not mutate a persistent match state, update active-ball movement, or handle boundary restarts.
- Do not change the fixture `Session` into a playable match session.

## Approaches

1. Add the dispatch to `Session`. Rejected because `Session` is an explicitly limited contact-loss fixture, not a game-state owner.
2. Add a small `field_handoff` orchestration module that composes the existing sequence and outcome helpers. **Chosen:** it connects the verified stages without guessing selectors or widening either calculation module's responsibility.
3. Expand the existing outcome helper to accept and calculate all contacts. Rejected because it would mix contact calculation with possession and recovery handling.

## Contract

`FieldContactHandoffInput` contains the ordered `DefenderContactInput` list, initial power/result, and all `DefenderOutcomeInput` context except its calculated result code. The helper runs the field sequence, supplies its final result code to the existing outcome resolver, and returns the sequence plus outcome. Codes 0/1 keep the existing `continue_action` result; codes 2/3 take their current recovery or possession route.

## Constraints

- C++17 and the current Make/CMake build structure.
- Preserve Windows/Linux portability.
- Keep ROMs, extracted tables, and translated content out of the repository.

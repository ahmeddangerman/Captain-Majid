# Prepared defender sequence

`resolve_prepared_field_contacts` connects player profiles, selected defense moves, energy deduction and ordered comparisons. It returns a complete updated roster value and each visited contact's preparation/energy/comparison outputs. It does not mutate the caller's roster or choices.

## Order and state

For each choice, validate its field slot1..10 or12..21, resolve its profile from the **current** roster, prepare its move, run the `$8FF3` defensive energy wrapper, then compare using the resulting weakened flag. Carry forward shot power; result0/1 visits the next choice, result2/3 stops. Unvisited choices consume no energy and are not evaluated. Duplicate choices intentionally use the already-updated stamina on each visit. Empty choices preserve power and initial result.

The selected-defender caller uses `compare_defender_contact`, preserving incoming shot power even for result1. `contact.loss` remains the calculated candidate loss; it is not deducted here. The old `calculate_defender_contact` isolated-loss API belongs to a different caller. The correction follows bounded original scheduler observations and propagates unchanged power to later contacts and the keeper. Original frame timing remains outside that evidence.

The wrapper overwrites comparison inputs for incoming power, defender strength, roster slot, subtype and weakened flag. When `context.comparison_content` is present, it also selects eligibility, variation range/offset and thresholds using the actual attack action, visited defense action and player ID. Random bytes and comparison context remain explicit. Without content, those table-derived fields remain supplied by each research fixture. An invalid visited choice rejects without exposing partial state updates.

Content is owned by the context and inspected only for reached comparisons. Empty sequences and choices after a terminal result do not access unused entries. A reached missing entry or zero variation range rejects through the existing selection/arithmetic helpers. Prepared field-player actions5/6 remain unsupported; their original bypass routes are not implemented by this option.

## Record fields that must stay connected

The profile loader `$803A` reads record+1/+2 as flag/alternate-type bytes, while `$8FF3`/`$8FFB` reads the same bytes as stamina. The connected sequence therefore derives both profile bytes from pre-debit stamina; independently supplied flags would diverge on subsequent actions. Original ordering loads the profile first and debits energy afterward.

Energy's special identity check uses the primary type at record+0. It must not use the roster slot or the effective type from generic team lookup. In particular, a generic primary type0 resolved to type32 does not receive the type32 surcharge. The energy wrapper skips debit for possession side0 and clears weakening. Its skipped-path carry is a caller-supplied fixture value; the original CPU flag schedule across the full loop is not reconstructed here.

## Verification and boundary

Native integration tests cover depletion/weakening, exact exhaustion, side skips, primary-type surcharge, generic lookup, repeated records, pre-debit profile aliases, carried power, subtype reset, terminal codes2/3, empty input and rejection without input changes. Existing private-ROM harnesses independently verify profile/preparation and energy behavior. This increment has native composition tests, **not** a new full-ROM loop differential comparison.

The API returns the updated roster for a later match caller to commit. It is not yet an ordered match command, random scheduler, or playable shot interaction. Field handoff and shot wrappers compose this API separately. The optional content connection was compiled in the production executable and received code review; no new behavioral tests or ROM comparison were run for this connection.

Existing power expectations in the sequence and prepared-shot checks were aligned to the corrected caller behavior. They were not run in this increment; production compilation and fresh code review are the available native evidence. Historical suite results above predate this correction.

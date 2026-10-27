# Prepared attacking action

`prepare_attacking_action` connects attacker profile and move lookup to stamina deduction and shot power, using the existing replaceable field-profile and move-content structures.

## Original flow traced

- `$C54B` index6 loads the attacker through `$819D`. The player ID is `$0441`; action/situation select a skill via `$8206`.
- For action0, when `($043C & $7F) >= 3`, the selector uses situation0. The move itself still uses the raw subtype masked to six bits, matching the common preparation helper.
- Move rows come from the separate attacker tables beginning at `$9460`, with effect, modifier, stamina cost and five upper flag bits. The common skill curve supplies the base action strength.
- `$8F72` clears weakening, debits the attacker only on side0, and sets cost0 for action2 when no defenders are selected. `$8F9A` uses the resulting weakened state and action subtype to calculate power.

## Native behavior

The caller supplies player record/profile content, action, situation, raw subtype, side, selected-defender count, random bytes and incoming carry. The result retains resolved profile, skill/move preparation, the five raw move flags, energy result and action-power result. For side11, attacker stamina is unchanged, weakening clears, and the action2 cost override is skipped. For side0, the existing energy calculation applies with the primary player type and raw subtype; action2 with no defenders overrides cost to0. Shot power receives the carry returned from energy on side0, including its cleared carry when weakening; side11 preserves the caller-supplied carry. The raw upper move flags are returned without assigning an unverified meaning.

## Checks and limits

Native checks cover action0 selector boundaries and high-bit behavior, move flags/cost, side-specific energy, type32 surcharge, action2 zero-cost rule only on side0, energy carry into action power, and energy-to-power weakening. A regression now pins random bytes100/100 and an incoming carry to the correct multiplier228 after weakening. Existing original-ROM oracles still pass 69,632 skill-index, 7,456 action-power, 5,248 keeper and 9,728 energy cases. The later [banked attacking trace](BANKED_ATTACK_TRACE.md) now compares the entire `$8F72` caller and its real banked `$819D` loader in3,882 constructed cases.

The caller still supplies actual player/action/situation/content/random values. Shot-distance adjustments, automatic content import, gamepad choices, random scheduling, keeper move/stamina preparation and persistent match commands remain incomplete.

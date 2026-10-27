# Prepared goalkeeper

`prepare_goalkeeper` connects a raw keeper record, supplied skill/move content, the defensive energy wrapper and goalkeeper comparison. It returns the resolved profile, move/skill preparation, energy result and comparison. Its inputs are unchanged.

## Instruction evidence

The private Japanese ROM's bank28 entry `$828F` selects a keeper skill and calls common player lookup at `$803A`. For keeper slots0/11, `$80B2..80CC` reads an8-byte row from `$AE86 + profile*8`, with standard selector minus23 as the row offset. Field players retain their separate24-byte rows at `$9FCE`.

Defense action3 uses selector-table index5, while actions0..2 use their own index. The content adapter must supply the resulting per-action selector, rather than copy the raw six-slot table directly. This increment supports actions0..3. The move tail `$825B` selects subtype low6, loads effect, modifier and masked10-bit cost, and reads the strength curve through `$8278`.

Bank26 `$852F` loads keeper data, `$8534` invokes the side-aware energy wrapper, and `$853A..8548` sets critical threshold243 except primary record types33/64, which use205. Generic effective types must not replace the primary type for that check. The comparison still consumes explicitly supplied random bytes, context, variation coefficients, timer/penalty and score thresholds.

## API and content

`PlayerProfileMappings` now holds shared ordinary/alternate/generic type lookup. `resolve_player_profile_reference` returns only effective type and profile ID. Existing field resolution keeps its slot restrictions and field skill rows. `GoalkeeperProfiles` supplies separate8-byte keeper rows. Generic primary type0 is supported for keeper11 through generic slot offset0; generic keeper0 is rejected as an unsupported original path.

The caller supplies the actual opposing keeper: slot11 for possession side0, slot0 for side11. A mismatch, invalid side/action/selector, missing type/profile/move entry rejects before indexing. Energy uses raw record+1/+2 as stamina and primary type as the identity. Side0 skips defensive debit and clears weakening; side11 debits normally. Strength, weakened flag, move subtype, keeper slot and critical threshold override stale comparison fixture fields. Keeper subtype remains separate from attacking subtype.

## Evidence and limits

Native tests cover profile/skill preparation, action3 mapping through supplied content, raw flags/cost, special thresholds, generic and alternate lookup, stamina weakening, subtype masking, side skip, caller preservation and invalid references. Existing field-profile, skill, power, keeper and energy private-ROM comparisons remain component evidence. There is no new complete keeper-loader differential harness in this increment.

The new function is separate from the joined shot caller. Next work connects attacking preparation and this keeper preparation at the correct stages, preserving the prepared defender roster. Runtime content import, automatic response choice, random scheduling, full gameplay traces and persistent match/session commits remain incomplete.

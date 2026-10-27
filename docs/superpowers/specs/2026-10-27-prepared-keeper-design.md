# Prepared goalkeeper

Resolve keeper profile from raw record and supplied type mappings, load an eight-byte keeper skill row, choose the action's skill (selector23..30), apply level/move modifier and supplied192-byte strength curve, then energy and keeper comparison. Keepers are slots0/11; generic type0 supports slot11 via team offset0. Preserve existing field lookup behavior by extracting shared type-to-profile resolution.

Support keeper defense actions0..3, supplied selectors and move lists. Use subtype low6 for move lookup. Original `$828F` maps action3 to selector-table index5; the content adapter supplies the final per-action selectors. Cost masks high bits to2; effect is returned. Apply `$8FF3` wrapper; side0 skips debit, side11 charges. Primary type33 or64 selects critical threshold205, all others243. Comparison strength, subtype, slot, weakened and critical threshold come from preparation. Random bytes, context, timer/penalty and thresholds remain explicit.

Reject side/keeper mismatch, invalid selector, absent mapping/profile/move before indexing. Input immutable. No automatic content importer or full shot/match integration claim. No bundled original tables.

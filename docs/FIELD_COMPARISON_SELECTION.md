# Caller-specific field comparison selection

`select_field_comparison` chooses the original caller's variation range/offset, eligibility and four classification thresholds from replaceable content. `configure_field_comparison` supplies them and the selected slot to an existing contact input without changing power, base strength, randomness, weakness, context or subtype.

## Original instructions

Bank26 `$834C` loads a group for attack action and multiplies it by4 into byte `$3B`. Field defenders add the supplied group for their defense action; keeper slots0/11 add2. The sum wraps to a byte and indexes eligibility at `$83E1`. A further shift wraps the coefficient offset separately, selecting a pair from the variation table referenced by `$8D95`. Eligibility is the sign flag left by loading that byte, rather than a nonzero test.

The comparison at `$8EE9` returns X=Y=0. Therefore `$8376` classifies against the four thresholds at `$83F1` from index0, irrespective of the earlier selection index. Returning this fixed caller threshold row is intentional.

The native content stores action group arrays, eligibility bytes, variation pairs and thresholds. Missing selected entries reject safely. Coefficient pair index is the original selection index masked to7bits, because the doubled byte offset wraps. A zero width can be selected as data but is rejected by the existing comparison arithmetic.

## Evidence

The native unit check uses synthetic data for field/keeper groups, eligibility-bit semantics, caller preservation, wrap boundaries and invalid references. `verify_field_comparison_selection.py` executes the original `$834C..$837F` through the actual arithmetic and classification, without skipping calls. It matched **550 constructed fixtures**, including selected content and adjusted strength, rolled strength, loss, score, classification, subtype and special status. The raw four thresholds are read privately as inputs.

The private oracle supplies a full byte-indexed eligibility view and variation-offset view, including bytes beyond labelled table boundaries for constructed unusual action combinations. Equality does not establish that all tested action/slot combinations can occur during play. No original content is committed.

## Remaining boundary

This starts after player/move/energy preparation and stops before `$837F` stores the classification and calls `$8E33`. That later helper can change result0 to **result4** after a proximity/random condition and scenario call. That condition uses the ball position: carry is set for Y in `$60..$9F` and X below `$50` or at least `$B0`; the random threshold is15 with carry and63 otherwise, inclusive. Result4 dispatches separately at `$847C`. The currently joined native field sequence models ordinary comparison codes0..3; it does not reproduce that override. Caller power-loss timing, move bypasses5/6, post-loop active-defender state and original frame random scheduling also remain to be traced before a complete original loop comparison or ordered shot command.

Run with private ROM and py65:

```sh
PYTHONPATH=/path/to/py65 python3 tools/verify_field_comparison_selection.py /path/to/reference.nes
```

# Per-defender subtype sequence design

## Goal

Make the native ordered field-contact sequence start each selected defender with that defender's own subtype value, matching the Japanese ROM loop.

## ROM evidence

In the field-contact loop, `$82FC` begins each defender iteration. At `$8306–$8318`, the game indexes the selected-defender arrays using `$0616`, then copies `$0606,X` into `$043E` for the current contact. If the contact result remains below 2, `$83BC–$83CE` increments `$0616` and jumps back to `$82FC`. A special contact may modify `$043E`, but the next iteration reloads the next defender's saved `$0606` value.

## Native behavior

Each `DefenderContactInput` already carries a subtype. `resolve_field_contacts` will use the current input's subtype for that contact and will no longer replace it with the preceding result's subtype. Incoming power remains carried between contacts, and codes 2/3 still stop the sequence.

## Verification

Add a regression where the first contact enters the special branch and sets subtype bit 7, then a later contact starts with a different subtype and does not take the special branch. Assert that the second result reflects its own input subtype. Keep the existing contact arithmetic and ROM comparison unchanged; they validate individual calculations, while the loop's byte trace validates per-defender reinitialization.

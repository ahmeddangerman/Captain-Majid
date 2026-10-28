# Continue a prepared shot without preparing it twice

The ordinary human menu already commits attacker stamina, power, subtype and move fields into the persistent development session. The complete-shot calculator currently starts before that work and repeats it. Extract the stages after attacker preparation into a reusable API so a caller can continue from the menu's captured prepared state without a second attacker debit.

The continuation consumes a `PreparedFieldShotInput` whose roster already contains the post-attacker stamina, and whose field context carries the prepared action0 power/subtype. It retains the existing explicit defender choices, random bytes, post-loop gates, optional approach/flight inputs, keeper decision and keeper random inputs. It returns the existing field handoff, flight, keeper and goal/save results. It does not infer defender AI, selection, frame timing, random scheduling, next-phase recovery, or persistent session commits.

`resolve_complete_prepared_shot` remains the convenience API for a pre-attack roster. It prepares the attacker once, then delegates the remainder to the new continuation API. Existing standalone results must remain unchanged.

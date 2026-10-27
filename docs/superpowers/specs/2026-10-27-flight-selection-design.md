# Flight candidate selection from a position snapshot

Port original `$87E1` selection from an empty list. Add a shared field-cell encoder matching `$CDE2`: valid X48..207/Y80..175 maps to column*12+row, otherwise255. Scan the ten opposing field players, skipping nonzero status, and select only players whose encoded cell equals the explicit ball-cell input. Preserve scan order and caps5/4 for possession side0/11. The original capped comparison clears carry before the random subtraction: inclusion sample is wrapped `random_first-random_second-1`, accepted only below the threshold. No random state is consumed.

Expose a pure selector using existing20-player position/status records. Connect the development scene's nonzero `flight_defenders` option to a constructed layout: place N opposing players in the ball cell, the others in another valid cell, then use selection output to build its one burst. Keep defaults and observed IDs unchanged. This is selection from one snapshot, not continuous movement or multiple flight ticks.

No new content pack fields, original data/assets or tests. Record bounded original routine observations privately, build production targets, inspect normal scene usage and request fresh static review. Retain explicit terminal flight limitations and no full-match claim.

# Persistent selected shot defenses

Continue the approved native shot integration from phase20. Use the saved roster,
owner, prepared power/subtype and ordered selected IDs. Accept explicit defense
choices and per-contact random bytes; select comparison coefficients from content.
Do not prepare or charge the attacker again. Resolve interruptions from saved
field status and keeper penalties, and commit their returned writes.

Result2/3 enter the existing rewarded terminal clock/recovery flow, retaining its
added-time pause. Result0/1 pause at new phase21 (shot field contacts complete).
Result4 pauses at phase22 (shot restart pending); derive its restart decision
from saved ball coordinates using the existing interruption helper. Neither new
phase executes flight, keeper, restart presentation or AI. Repeated invocation
must reject without mutating input. Require choices to match the selected prefix
exactly, including unreached suffix IDs, with no duplicate players.

Use a small session-specific wrapper rather than copying match state into a caller
constructed shot fixture. A full shot wrapper would need score/story/flight state
not represented by the current codec. Session8 retains the324-byte layout and
readers1–7; old version tags must reject phase21/22. No original data is published.

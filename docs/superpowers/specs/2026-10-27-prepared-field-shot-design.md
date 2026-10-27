# Prepared field shot handoff

Compose the approved prepared defender sequence, immediate field handoff, and conditional action0 goalkeeper continuation. Supply profiles/content externally in C++17. Return the updated defender roster, field outcome and optional continuation results; do not mutate input or claim a persistent playable match command.

`PreparedFieldShotInput` supplies roster, choices, field context, current owner, optional recovery fixture, continuation gates, keeper comparison and outcome fixture. `resolve_prepared_field_shot(input,profiles,defense)` resolves contacts with initial field result0, derives the terminating defender ID/action from the last visited choice, and dispatches the actual field result. Field results2/3 never evaluate keeper data. Results0/1 use the existing action0 route gates; diverted routes carry no keeper/outcome.

The continuation receives remaining shot power and the attack subtype from field context, never the last defender's subtype. Keeper roll subtype remains the separately supplied keeper move; attacking subtype goes to the outcome handler unchanged. Outcome side/owner come from field context/current owner and the actual opposing keeper slot is derived from side. For result2 recovery, overwrite owner/opposing player with current owner/last visited defender; positions, random bytes, selected-defender recovery list and ball-update state remain explicit recovery-phase inputs. A missing recovery fixture returns the unresolved recovery route.

Only possession sides0/11 are accepted. Empty contact lists use original result0; no dummy defender is consulted. Reject invalid visited or reached keeper/recovery inputs without input mutation. Unreached phases are not validated. Attacker power generation, keeper profile/energy, random scheduling, boundary restarts and persistent match/session commits remain outside this increment.

Approach: compose existing verified helpers, preserving explicit phase outputs instead of inventing a duplicate canonical match state before the remaining phases are traced.

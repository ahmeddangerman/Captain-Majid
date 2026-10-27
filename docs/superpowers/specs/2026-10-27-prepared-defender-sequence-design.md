# Prepared defender sequence

Connect the approved profile/preparation, energy and contact APIs in ROM order: load each visited player's profile/move from current stamina bytes, apply the defensive energy wrapper, compare using its weakened flag, carry remaining shot power, stop on result2/3. Use C++17 and supplied content only.

A22-slot roster supplies primary type, level and stamina. Choices supply slot, defense action, situation, subtype and explicit comparison parameters. Context supplies possession side, attack action/subtype, incoming power/result and incoming energy carry. Return a new roster plus per-visited preparation, energy and contact results. Input roster/choices remain unchanged, including on rejection. Duplicate choices re-read the updated roster; choices after a terminal contact are untouched.

Derive profile flags/alternate-type from current stamina low/high bytes because the original loader and energy routine read the same record+1/+2 locations. Charge the primary record type (not roster slot or resolved profile type); use skip_home_side=true and current attack action/subtype. Override comparison power, slot, subtype and weakened state with computed values. Comparison coefficients, thresholds and random bytes remain explicit. No persistent match command, full-ROM loop scheduling, possession handoff or runtime content import is claimed.

Approach: pure connected sequence returning an updated state value, allowing later match/online callers to commit it atomically. Mutating the supplied roster during calculation would leave partial updates if a visited choice is invalid.

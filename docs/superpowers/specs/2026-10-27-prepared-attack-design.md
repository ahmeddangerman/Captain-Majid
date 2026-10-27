# Prepared attacking action

Connect attacking player profile/move lookup, attacker-side energy behavior, and action-power calculation. Use supplied content and existing helpers; add no ROM tables. Original `$819D` loads action/situation skill selector, uses action0 selector0 when `(subtype & 0x7f) >= 3`, loads effect/modifier/cost/upper move flags, then uses the common strength curve. `$8F72` debits stamina only for side0, overrides cost to zero for action2 with no selected defenders, clears weakness, and passes into `$8F9A` action power.

Input is one raw field-player record, action/situation/subtype, side, defender-presence byte, random bytes and incoming carry, plus caller content. Return resolved profile/preparation, effective energy, move flags and action power. Side11 skips attacker deduction; side0 applies direct energy arithmetic. Type32 surcharge uses primary record type. The zero-cost condition is action2 and zero selected defenders. Attack subtype is the input/output attack subtype. No shot distance, move selection UI, full RNG schedule, goalkeeper preparation or persistent session commit is claimed.

Approach: reuse field profile/move and standard skill primitives, with a thin attack-specific selector rule and action caller. Keep move flags explicit without assigning an unverified gameplay meaning.

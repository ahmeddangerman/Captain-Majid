# Prepared comparison content

## Intent

Connect the existing original-caller table selector to prepared field contacts, including the joined action0 shot. A runtime content adapter will eventually supply these tables from the user's private game data.

## Approach

Append optional owned `FieldComparisonContent` to `FieldDefenseContext`. For each reached contact, use the actual attack action, defense action and player ID to replace variation range, offset, eligibility and thresholds. Keep random bytes and comparison context explicit. Reuse `configure_field_comparison`; do not duplicate its indexing rules. Without content, preserve existing explicit-fixture behavior.

Alternatives considered: require content immediately (breaks existing research callers), or keep configuring each choice outside the sequence (leaves stale action/slot values possible). Optional sequence-owned selection is the implementation choice under the user's standing instruction to continue.

## Constraints

- C++17, immutable inputs and returned working roster.
- No ROM bytes, external disassembly source, assets or extracted tables added.
- Lazy evaluation: empty choices and choices after a terminal result do not inspect unused table entries.
- Selected missing entries, invalid actions or zero variation range reject through existing helpers. No guessed defaults.
- Original field action5/6 and keeper action4 bypass setup remain outside this field-player wrapper.
- Do not change power-loss timing without full caller evidence.
- Per current session instructions, add/run no tests without a new user request; build the production target and report that limitation.

## Evidence boundary

The selector already has isolated ROM comparisons. This connection has build and code-review evidence only in this increment. It does not establish a complete original shot trace.

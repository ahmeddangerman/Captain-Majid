# Field defender preparation

## Intent and approach

Replace hand-entered defender base strength with the verified standard field-defender skill/move lookup. Implement pure C++17 functions using decoded content supplied by the caller; this follows the current core's separation of game rules from original tables and allows replacement content. Embedding ROM tables would tie the core to one content set; implementing a complete NES memory mapper would expand this increment beyond the loader.

## Evidence and scope

The fixed-bank `$C54B/$CE6E` helper switches to banks 28/29 and dispatches through their `$8000` jump table. Caller index 7 reaches `$8224`, the field defender loader. It selects the skill through `$824C + defense_action*3 + situation`, resolves the current player's profile through `$803A`, and selects a four-byte move via `$9554` and the byte-wrapped `subtype*4` offset. `$825B–$828E` outputs move effect (`$0445`), stamina cost (`$043F/$0440`, upper byte masked to 2 bits), and strength (`$32/$33`) from the standard skill curve at `$9E4E`.

This increment supports defense actions 0–4, situations 0–2, and standard non-stamina skill selectors 1–30. Unavailable selectors and missing moves are rejected. The input player profile contains decoded base skills and the level byte; original player-type/profile resolution remains a separate content-loading task. Actions 5/6 bypass the ordinary field comparison and are outside this loader.

## Interface

`FieldDefenseContent` holds the 5×3 skill-selector map, per-action vectors of raw four-byte move records, and the 192-byte strength curve. `FieldDefenderPreparationInput` contains the player's 31 base skills, level, defense action, situation, and subtype. `prepare_field_defender` returns selected skill, adjusted skill index, byte-valued base strength, move effect, and 10-bit stamina cost.

Subtype selects move `(subtype & 63)` because the original doubles the byte twice before indexing. Reuse `standard_skill_index` for the exact wrapped level and modifier arithmetic. A small calculation wrapper supplies loaded strength and subtype to the existing defender-contact function, preserving its other explicit research inputs. Energy accounting remains caller-controlled.

## Evidence required

Meaningful native checks must cover situation-dependent skill selection, each action, unavailable selectors, absent moves, high subtype bits, cost masking, byte wrap/capping, and use of loaded strength in the contact calculation. A private Py65 verifier will execute the complete original `$8224` loader with actual profiles and move tables, compare every exposed output, and report its bounded coverage. No original content is committed.

# Standard field defender preparation

The engine can now select a field defender's standard skill and chosen move, then load the base strength, move effect, and stamina cost from caller-supplied content. `calculate_prepared_field_defender` connects that strength and selected subtype to the existing contact calculation.

## Original path

Before ordinary field contact, the Japanese game calls `$C54B` with index 7. The fixed-bank dispatcher at `$CE6E` temporarily maps PRG banks 28/29 and dispatches through `$8015` to `$8224`. This is a gameplay data loader. It resolves the selected player's profile through `$803A` and selects a skill through the 5×3 table at `$824C`, using defense action `$043D` and situation `$044E`.

The loader reads a move pointer through `$9554 + action*2`. It multiplies subtype `$043E` by four using byte arithmetic to select a four-byte record. `$825B–$828E` loads the move effect, applies the move modifier to the level-adjusted skill index, reads strength through `$9E4E`, and loads the 10-bit stamina cost.

## Native data contract

- Five defense actions (0–4) and three caller-selected situations (0–2).
- A decoded player profile containing 31 base skill bytes and a level byte.
- Replaceable content: 15 skill selectors, five move lists, and a 192-byte strength curve.
- Each move has four bytes: effect, skill modifier, cost low byte, and cost high/flags byte. Only the low two bits of the fourth byte contribute to cost.
- The move index is `subtype & 63`; both high subtype bits disappear when the original byte is shifted twice.
- `standard_skill_index` supplies the verified byte wrapping, level carry, and two index caps. Strength is the curve entry at that index.

The preparation API returns selected skill, adjusted index, base strength, move effect, and stamina cost. It rejects unavailable or unsupported skill selectors (outside 1–30), unsupported action/situation values, and missing moves. The contact wrapper substitutes loaded strength and selected subtype while preserving the caller's random values, comparison thresholds, and post-energy weakened flag. Preparation itself does not charge stamina.

## Validation and limits

`tools/verify_field_defender_preparation.py` runs the complete original `$8224` loader and compares all five native outputs. It also requires the original routine to resolve the supplied player's record pointer. The comparison passed **16,064 cases**, covering all 34 normal nonzero player types, both field-player ID ranges, all available action/situation combinations, each move in the five tables, level boundaries, and high subtype bits, plus seeded cases. Original tables are read privately from the verified Japanese ROM and are never bundled in the repository.

The native input begins after player-profile decoding. Generic type-zero/team-position profiles, alternate player types, special skill paths, and a runtime content importer still need implementation. The verifier constructs inputs using actual normal profiles; these are isolated loader fixtures rather than captured match traces. Defender choice, tactical defense actions, situation selection, per-contact stamina scheduling, thresholds, randomness, and ordered match state remain caller responsibilities.

## Development interface

`--field-defender-preparation-batch` accepts one whitespace-separated request per line:

1. Defense action, situation, subtype, level.
2. 31 base skill bytes, followed by 15 selectors in action-major order.
3. For each of the five actions: move count (0–64), then that many four-byte move records.
4. 192 strength-curve bytes.

It returns selected skill, adjusted index, strength, move effect, and stamina cost. `defenderprep` uses the same fields as a stateless replica research query. This interface is for development fixtures; it does not advance the ordered session.

Run the private comparison with Py65 available:

```sh
python3 tools/verify_field_defender_preparation.py /path/to/reference.nes
```

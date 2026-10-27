# Native shot approach

## Goal

Replace explicit distance/route/target literals in supported prepared shots with the original `$DD81/$DD47` calculation and field-cell codec.

## Approach

Add a pure C++17 helper using supplied14-byte outer and36-byte inner distance tables. Preserve byte wrapping, the different side11 mirrors in the two routines, the distance multiplier's carry, loose-ball routing and endpoint random-bit consumption. Return target cell and its center coordinates. Add optional approach context/result to the complete prepared shot, evaluated after nonterminal field results and only when the post-loop active-defender gate permits it.

The helper's random input is an explicit post-player-movement byte. Keeper random bytes remain explicit after keeper choice; do not assume the helper closes the entire random schedule.

Extend private pack exporter/reader with `CMJSHOT2` containing the two approach arrays after existing data; retain version1 reader behavior and the old explicit laboratory scene for old packs. New packs let the shared scene derive distance/target/loose-ball route. Add scene ball-position options to the CLI, not the public webpage.

## Constraints

- No original data committed; missing/out-of-table geometry rejects rather than reading instruction bytes as content.
- Native defaults with no approach context retain current behavior. Terminal field results and active-defender diversions do not inspect approach content.
- Ball target coordinates describe the original cell center after `$E73E`; motion/intervening player updates, clock and random scheduling are not implemented by this helper.
- Correct research snapshot semantics: keeper record+6 is penalty duration, not an X coordinate. Normalize constructed keeper runtime bytes, record that correction, and do not present them as field positions.
- No automated tests added/run. Original-code observations, production builds, normal usage and fresh review provide limited evidence.

## Next boundary

Player movement/AI and general gameplay randomness still must supply coherent approach/keeper inputs. This is not a full original outcome-script trace or persistent match.

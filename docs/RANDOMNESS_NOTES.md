# Original random scheduling: open integration question

## Static evidence

Direct inspection of the Japanese ROM shows that the NMI path calls `$C9C5` at `$C7E4`. That generator reads the index at RAM `$00E1`, mixes bytes at indexed addresses `$0300 + index`, `$0700 + index`, and `$0780 + index`, updates `$00E2/$00E3`, and increments the index. It uses CPU carry as part of its arithmetic.

The first indexed range overlaps player data; the second overlaps the original sound queue/driver RAM. The third can cross `$0800`, which mirrors the NES's lower 2 KiB RAM. A future reference harness for this generator must implement that mirroring; flat py65 memory alone would not establish correctness for every index.

This is static evidence about the inputs and call site, not a completed trace of every frame or state that affects random outcomes. The existing verified arithmetic routines do not execute this generator; they receive explicit random bytes and, where required, an incoming carry value.

## Engine implication

The agreed engine direction requires identical simulation results for identical ordered commands and starting state. Actual rendering speed, replacement music, and audio-device timing must not advance gameplay randomness independently on each client.

Before integrating complete shots, capture original gameplay traces to establish which frame/decision boundaries consume these values and what canonical state supplies them. Decide explicitly how to reproduce those inputs inside the simulation while keeping replacement graphics and music independent. Neither copying the interrupt schedule to wall-clock time nor silently substituting an unrelated random generator establishes faithful deterministic gameplay.

## Next evidence needed

- Frame/state traces around shot selection, energy accounting, defender encounters, and keeper decisions.
- Capture generator input bytes, carry, output bytes, and frame/command identifiers.
- Verify whether alternative cosmetic/audio settings affect the logical reference inputs.
- Use traces to define reproducible simulation scheduling and save/replay state before advertising shot-level online fidelity.

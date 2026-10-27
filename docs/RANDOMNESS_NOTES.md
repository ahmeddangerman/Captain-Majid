# Original random scheduling: open integration question

## Static evidence

Direct inspection of the Japanese ROM shows that the NMI path calls `$C9C5` at `$C7E4`. That generator reads the index at RAM `$00E1`, mixes bytes at indexed addresses `$0300 + index`, `$0700 + index`, and `$0780 + index`, updates `$00E2/$00E3`, and increments the index. It uses CPU carry as part of its arithmetic.

The first indexed range overlaps player data; the second overlaps the original sound queue/driver RAM. The third can cross `$0800`, which mirrors the NES's lower 2 KiB RAM. The prepared reference harness implements that mirroring; flat py65 memory alone would not establish correctness for every index.

This is evidence about the inputs and call site, not a completed trace of every frame or state that affects random outcomes. One generator step is now implemented independently in `reference_random.cpp`, with a 2 KiB mirrored RAM sampler. The shot arithmetic continues to receive explicit random bytes and, where required, an incoming carry value until the frame schedule is established.

## Single-step contract

`advance_reference_random` accepts the original index and two stored bytes, three sampled RAM bytes, and incoming carry. It returns the advanced index, two new random bytes, and outgoing carry. The previous second random byte is overwritten without being read. Index and intermediate byte overflow are preserved.

`sample_reference_random` reads the three source bytes from a canonical 2 KiB RAM image using the original index. In particular, `$0780 + 128` reads RAM `$0000`, and `$0780 + 255` reads RAM `$007F`.

The native `--random-step-batch` research mode takes index, first, second, player byte, sound byte, tail byte, and carry (0/1); it returns new index, first, second, and carry. The replica command is `random` followed by the same fields. This exposes one algorithm step and does not choose when an online simulation should advance it.

`tools/verify_shot_state.py` prepares 3,584 original-instruction comparisons for this step, including all indices, both carries, and RAM aliases at the mirror boundary. A completed native comparison run for this increment is still pending; the presence of the harness is not a passing result.

## Engine implication

The agreed engine direction requires identical simulation results for identical ordered commands and starting state. Actual rendering speed, replacement music, and audio-device timing must not advance gameplay randomness independently on each client.

Before integrating complete shots, capture original gameplay traces to establish which frame/decision boundaries consume these values and what canonical state supplies them. Decide explicitly how to reproduce those inputs inside the simulation while keeping replacement graphics and music independent. Neither copying the interrupt schedule to wall-clock time nor silently substituting an unrelated random generator establishes faithful deterministic gameplay.

## Next evidence needed

- Frame/state traces around shot selection, energy accounting, defender encounters, and keeper decisions.
- Capture generator input bytes, carry, output bytes, and frame/command identifiers.
- Verify whether alternative cosmetic/audio settings affect the logical reference inputs.
- Use traces to define reproducible simulation scheduling and save/replay state before advertising shot-level online fidelity.

# Shared pass-distance calculation

Objective: replace the manually supplied pass-distance byte in the existing sampled replay with an optional calculation that is valid for a bounded field domain, without adding a score-specific encounter or changing existing session snapshots.

Evidence: original $DF8B converts a target cell to its center, finds the larger unsigned absolute difference from the ball coordinates, divides by eight, and indexes a 20-byte table. A controlled original CPU experiment matched this candidate for 22,560 boundary and deterministic random cases with ball x48..207, y80..175 and target cell0..239. Table bytes remain private and replaceable. The original $E059 recipient selection, ball flight, live timing and unsupported out-of-field bytes are separate concerns.

1. [x] Add a pure C++ calculation with an explicit 20-byte content table and bounded input validation. Add focused tests for axis boundaries, byte subtraction across 128, table indexing, invalid inputs and table-file errors.
2. [x] Add a private-output-only table exporter with a format marker, verified ROM identity, exclusive creation and no original bytes in Git. Implement a strict reader; preserve existing pack and snapshot versions.
3. [x] Add an optional phase-session operation that calculates distance from a supplied table and then uses the existing guarded pass-target capture. Keep the existing explicit-distance operation unchanged. Allow the phase317 replay to choose the calculated operation with a private table path.
4. [x] Compare the generic kernel with controlled original CPU execution over boundary and deterministic random inputs, then replay the two already-observed encounters end to end with calculated distance. Do not infer other target/recipient selection or AI behavior.
5. [x] Run all registered tests; document precise coverage, limitations and evidence paths; commit and push to work.

Verification: all3,686,400 valid coordinate/cell inputs matched original CPU execution; both existing phase317 replays matched their previous checkpoints; all56 Windows CTest checks passed. The local WSL GCC/Python environment showed independently reproducible intermittent failures, so a complete Linux Make check is not claimed.

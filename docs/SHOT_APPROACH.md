# Shot distance, target and loose-ball route

`calculate_shot_approach` reproduces the supported geometry/content decisions at fixed-bank `$DD81/$DD47`, plus the cell-center decoding used after ball flight. Inputs are ball X/Y, side0/11, the full rolled attack subtype byte, an explicit post-movement random byte and supplied distance tables. No original content is bundled.

## Distance

The distance routine normalizes side11 X with byte negation (`256-X`, modulo256). Below160, it subtracts48 with byte wrap and selects an outer entry after shifting right3. A nonzero outer entry supplies the coefficient. A zero entry falls into inner selection with the accumulator still zero; it does not reload the old X.

Inner selection subtracts160 with byte wrap and shifts right3 for the column. Y with bit7 set is inverted, then subtract80, mask56 and multiply by3/4 for the row offset. Sum row/column to select the inner coefficient. The native14/36-entry arrays cover the labelled source tables. An unsupported selected index rejects; instruction bytes beyond those tables are not treated as arbitrary geometry content.

Distance adjustment is the low byte of `coefficient*9 + ((coefficient>>5)&1)`. The extra bit preserves carry from the original third ASL. It matters for high replacement coefficients. This is the keeper adjustment input at `$062B`, not match clock time at `$05FF`.

## Route and destination

The route routine uses a different side11 normalization: bitwise inversion (`255-X`). If the full subtype byte equals0 and normalized X is below128, it chooses the loose-ball path. Add79 to normalized X, invert again for side11, encode with target Y124, and leave the random byte unchanged.

Other contexts choose cell233/234 for side0, or5/6 for side11, according to the random byte's low bit. Then shift that byte right1. Subtype128 does not satisfy the zero-byte condition; do not mask off the special bit here.

A cell represents `column*12+row`; its center is `(column*8+52, row*8+84)`. The actual flight routine writes that decoded center at `$E7C0..$E7CC`. This helper returns the final target center; it does not simulate the flight/player callbacks.

## Prepared shot connection

`CompletePreparedShotInput::approach` is optional. For a nonterminal field result and zero post-loop active-defender gate, the wrapper computes the approach using the actual rolled attack subtype. It replaces the loose-ball gate, keeper distance adjustment and outcome starting ball coordinates. Active-defender diversions and terminal field results do not inspect the context or its tables. With no context, prior explicit-fixture behavior remains available.

The result reports the optional approach separately. Keeper random bytes remain explicitly supplied after keeper choice; they are not silently replaced with the helper's returned byte. General player movement, keeper-choice randomness and frame scheduling remain unresolved.

## Original-code observations

Private research ran the actual `$DD81`, `$DD47` and `$CDC9` routines to return, without skipped calls:

| X,Y | Side | Subtype | Random in | Distance | Cell | Target center | Random out | Loose |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 160,100 | 0 | 0 | 100 | 45 | 233 | 204,124 | 50 | no |
| 160,100 | 0 | 0 | 101 | 45 | 234 | 204,132 | 50 | no |
| 120,100 | 0 | 0 | 101 | 90 | 221 | 196,124 | 101 | yes |
| 95,100 | 11 | 0 | 101 | 45 | 6 | 52,132 | 50 | no |
| 135,100 | 11 | 0 | 101 | 90 | 17 | 60,124 | 101 | yes |
| 52,84 | 0 | 0 | 100 | 171 | 125 | 132,124 | 100 | yes |
| 204,172 | 0 | 3 | 101 | 45 | 234 | 204,132 | 50 | no |
| 120,100 | 0 | 3 | 101 | 90 | 234 | 204,132 | 50 | no |

These are original-code observations, not automated native differential passes. Continuous original traces now record approach entry/result and final ball position too. Their four known side0 observations still have the same instruction/frame counts, with distance45, destination233 and final ball(204,124).

The constructed continuous snapshots were corrected to zero keeper record+6/+8 rather than initializing those bytes as field coordinates. Keeper+6 is penalty duration. Rerunning the four original observations retained their recorded counts and comparison outputs; the correction avoids assigning a misleading meaning to those role-specific bytes.

## Content and current evidence

Exporter version2 writes `CMJSHOT2` and appends14outer plus36inner distance bytes, producing an11,817-byte pack for the reference. The reader still accepts exact version1 layouts. The shared scene opts into geometry with version2; old packs keep the explicit default scene, and changing ball position requires version2.

Normal native laboratory usage with a new private pack produced a level80/no-defender goal at target(204,124) with power224 and distance45. A ball-X120 scene produced distance90 and a loose-ball route with no keeper calculation. Production builds/static review provide limited native evidence; no automated tests or differential suite were added/run. General random scheduling, movement/clock effects, later loose-ball phases and full original outcome scripts remain further work.

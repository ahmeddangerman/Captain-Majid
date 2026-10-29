# Prepared keeper-area interaction setup

The saved recovery flow can now enter original8B4A..8B73 numeric setup before
its interaction presentation/menu. This follows phase7 and ends at new phase65,
`keeper_area_prepared`. It does not execute the subsequent menu or player actions.

The native `prepare_session_keeper_area` function requires a captured ordinary
(action0) recovered-ball state in an original keeper area: Y96..159 with X<80 or
X>=176. It preserves an existing ball variant, or chooses random.bit0+1 when the
variant is0. The selected prefix is cleared, the interaction route is2 when the
area side matches possession and1 otherwise, and the original counter is set255.
It then uses the existing exact-cell field-player selection, status exclusion,
wrapped random subtraction, scan order and side-dependent5/4 limits.

Both keeper Y bytes, score, raw positions, fractional coordinates and other saved
state survive. Neither movement nor a match clock is advanced. Invalid inputs and
replay reject without mutating their source. The development CLI accepts
`--prepare-keeper-area`, and interactive mode accepts `prepare-keeper-area`.

## Save format

Snapshot45 (`j`) is372bytes. It appends keeper-area presence and counter to the
previous370-byte layout. Presence is required exactly at phase65, with counter255;
unused counter bytes must be zero. The route must agree with the captured area
and possession side. Earlier snapshots through i remain readable and cannot
encode phase65. Truncation, malformed presence, missing context and bad route/counter
states reject. These checkpoints are not a complete match save or network protocol.

## Original comparisons

- 64 recovered-ball entries from the controlled rebound suite match original
  setup before8B73. The preceding outcome/recovery/control comparisons remain
  included. Evidence: `keeper-area-original-first`.
- 24 constructed cases check both sides and areas, generated/preserved variants,
  random subtraction, status skipping, scan order and selection limits. They cover
  both routes and selected counts0/4/5. Evidence: `keeper-area-routes-first`, via
  `tools/check_keeper_area_original.py`.
- All instructions in original8B4A..8B73 execute, including DCDF and87E1 calls.
  The routine ends before its presentation. The recovered-ball fixtures originate
  from explicitly supplied non-goal keeper results and do not establish naturally
  occurring save classifications or a complete playable match.

Focused regression failed on the initial stub, then passed. Save roundtrip, prior
i input, forbidden old output, malformed/truncated context, invalid area and replay
checks pass. Full Make check/build pass. ROM and generated evidence remain private.

Next: trace8B73 presentation/story/menu boundaries and the following action route.

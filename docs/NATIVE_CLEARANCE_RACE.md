# Clearance arrival race

Phase33 now runs the deterministic DE96 clearance race. This resolves player
movement/arrival rules, not live animation or a frame scheduler.

The original selects the nearest eligible field player on the active side, then
the other side, excluding the carrier and saved opposing player. Both target the
clearance cell. Each round moves the active candidate first with selector35 speed,
then the other with selector36 speed. A reached destination cell wins immediately;
the later candidate does not move after an active-side win. The byte counter is
decremented only after an unsuccessful pair; initial zero allows256 rounds.

Use `--clearance-race-file PRIVATE.cmjrace` or interactive
`clearance-race-file PRIVATE.cmjrace`. Export replacement speed data privately:

```
python3 tools/export_clearance_race.py ORIGINAL.nes PRIVATE.cmjrace
```

The512 unsigned16-bit values form256 active/other speed pairs, using the existing
movement speed-profile mapping. Only reached candidate profiles are loaded.
Missing/extra/oversized values, invalid reached profiles and replay reject
atomically. Generated content is ignored by Git and kept outside the source repo.

## Saved handoffs

- Phase34: winner owns the ball, side changes if needed, coordinate high bytes
  snap to the destination while fractions remain. This stops at DF0D before map
  and presentation. Team-music and script28/62 are explicit pending requests.
- Phase35: timeout at DED4 before script52/recovery. Original transitional carrier
  and active side remain distinct; no downstream recovery is inferred.

Elapsed becomes1. Match clock, stamina, experience and random bytes are not
advanced. Selection/progress and clearance distance remain captured. Session13
marker D is336bytes, appending presence and original430/431/432 byte values
(remaining counter, active candidate, other candidate). Versions1–12 remain
readable. New phase/version, ownership and optional-context rules are validated.

## Evidence

The focused regression failed against a throwing stub, then passed for candidate
exclusions, active-first priority, either side winning, no movement after a winner,
timeout/zero-counter wrap, fractions, replay/errors, old version12 setup and new
winner/timeout snapshots. Review found no material issues; its suggested direct
selector35/36 coverage was added with distinct nonzero speeds. Full native/CLI/
adapter/loopback/reference checks and production Make build passed using a
same-source core archive (`CORE=/tmp/cm-race-core.a`). Compiler diagnostics were
clean. CMake remains unavailable.

Four constructed original CPU observations and native CLI operations match324
prior represented bytes after phase/version normalization. Saved distance062B and
race bytes430–432 were also compared independently; native shot-progress metadata
is excluded from the original-memory claim:

| Case | Native336-byte SHA256 |
| --- | --- |
|Active side wins immediately|494b51bdc21c0325187ef519f30e8db6bb72a3a8c448bd29b8c725c0e1808a77|
|Other side wins immediately|9cc0e2cfd06d98cf001fb54854db880b19f61f6906c6b040cf722a62fc10d621|
|Timeout after4 rounds|bddce2b81bcc32d03a4a177fcc8ef0a5e49577397c3226fd88499aff6a9d6aed|
|Initial zero wraps; winner at round22|c675b4a85d934424abc2101708a99a2f2c6fb6139430241f68e04e5ea632ee69|

Private evidence: `/workspace/Captain-Majid-analysis/clearance-race-guq11zhq`.
The observer's `--race-only` entry starts DE96 and stops at DF0D/DED4. These use
explicit cooperative-task acknowledgements with no skipped original calls; no
NMI/audio/PPU or true hardware/frame timing is emulated. Entry states are
constructed from the previous original clearance setup, not a full match trace.

Next: connect winner presentation/control and timeout recovery handoffs, then
restart/keeper/story and remaining shot paths. Full playable match, rendering and
online integration remain unfinished.

Ordinary winner/control and timeout/recovery handoffs are now connected; see [NATIVE_CLEARANCE_HANDOFFS.md](NATIVE_CLEARANCE_HANDOFFS.md). Live presentation is still outside the prepared continuation.

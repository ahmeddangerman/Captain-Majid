# Shared pass-distance calculation

The native kernel reproduces the observed original `$DF8B` distance calculation for a ball within x48..207 and y80..175 and a destination cell0..239. It converts the cell to its center, takes the larger absolute coordinate difference, divides by eight and indexes a replaceable 20-byte table. Inputs outside that bounded domain are rejected; arithmetic, target cell and table are explicit inputs. The kernel does not select a pass recipient or establish opponent AI behavior.

The ROM-derived table is private. Export it outside this checkout from your own verified Japanese reference ROM:

```text
python tools/export_pass_distance.py PRIVATE_ROM PRIVATE_OUTPUT.cmjpdist
```

The exporter checks the reference payload and refuses both a destination inside the Git checkout and an existing file. The `CMJPDST1` format consists of an eight-byte marker and 20 table bytes; the native reader rejects wrong markers, truncated input and trailing data. Keep this file with other private analysis artifacts.

The development phase-session CLI can calculate the distance while applying the existing guarded capture:

```text
captain-phase-session PRIVATE_PACK PRIVATE_SESSION \
  --calculate-sampled-field-pass-target PRIVATE_TABLE RECIPIENT DESTINATION COUNTER \
  --capture PRIVATE_OUTPUT_SESSION
```

The old `--capture-sampled-field-pass-target RECIPIENT DESTINATION COUNTER DISTANCE` remains available. Both operations still accept only the existing sampled handoff contexts; calculating distance does not generalize their recipient, target or action selection. Existing pack and snapshot formats are unchanged. The phase317 replay accepts the optional `--distance-table PRIVATE_TABLE` argument to use the calculated path.

`tools/verify_pass_distance_original.py PRIVATE_ROM` compares deterministic edge and random inputs against the original CPU routine. Add `--exhaustive` to cover all valid ball-coordinate and destination-cell combinations; it streams bounded batches and keeps ROM data out of the checkout. The exhaustive run matched all 3,686,400 inputs in the bounded domain. These are constructed routine inputs, not live-match startup, audiovisual timing, network play or a complete playable match.

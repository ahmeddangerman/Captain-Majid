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

The additional `--resolve-sampled-field-pass-target PRIVATE_TABLE RECIPIENT PREVIOUS_CELL COUNTER` derives the destination from the explicitly supplied field recipient's high coordinate bytes before calculating distance. Recipient255 preserves the previous cell byte; out-of-field recipient coordinates resolve to255 and are rejected by the bounded distance calculation. Field IDs1..10 and12..21 are supported, while keeper and other IDs reject. `tools/verify_pass_target_original.py PRIVATE_ROM` compares this bounded resolver against original `$E059` execution: all65,536 coordinate pairs for field player7,2,000 field-player boundary fixtures and all256 sentinel previous bytes matched. The original routine's scratch registers and live input/AI choice are not represented.

The old `--capture-sampled-field-pass-target RECIPIENT DESTINATION COUNTER DISTANCE` remains available. Both operations still accept only the existing sampled handoff contexts; calculating distance does not generalize their recipient, target or action selection. Existing pack and snapshot formats are unchanged. The phase317 replay accepts `--distance-table PRIVATE_TABLE` to use the calculated path, and `--resolve-recipient` to calculate the destination from the explicitly supplied recipient position.

`tools/verify_pass_distance_original.py PRIVATE_ROM` compares deterministic edge and random inputs against the original CPU routine. Add `--exhaustive` to cover all valid ball-coordinate and destination-cell combinations; it streams bounded batches and keeps ROM data out of the checkout. The exhaustive run matched all 3,686,400 inputs in the bounded domain. These are constructed routine inputs, not live-match startup, audiovisual timing, network play or a complete playable match.

## Ordinary pass target initialization

`initialize_ordinary_pass_target` is a bounded arithmetic kernel for the state after an explicitly chosen ordinary pass type. It takes the carrier's 16-bit fixed-point position, uses the high bytes for the refreshed ball position, calculates the field cell, initializes the target cursor to that cell, sets ball mode 129 and attack subtype 0. It rejects a carrier outside the supported interior field. It does not perform the pass-type submenu or choose a recipient. `--begin-ordinary-human-pass-target` applies this kernel to a confirmed side-zero pass after the caller explicitly acknowledges ordinary pass-type selection; it saves phase331 using the existing extended snapshot format. `--poll-ordinary-human-pass-cursor HELD PRESSED` advances the target cursor; free and single-teammate targets can be confirmed separately; the multi-teammate chooser and cancellation remain unsupported.

`tools/verify_ordinary_pass_initialization.py` accepts a private Japanese ROM and at least two private original menu RAM seeds. Across one captured startup encounter and an earlier independently verified encounter, 32 constructed original CPU cases covered eight carrier positions including field edges and two fractional-byte patterns. Ball coordinates and cell were intentionally stale: the original refreshed them from the carrier before initializing the cursor. These were cooperative scheduler observations with supplied input, not live NMI/presentation simulation. Sixteen full saved-session comparisons over eight positions in two independent encounter contexts matched original represented fields; the existing scenario-guarded target sessions remain unchanged.

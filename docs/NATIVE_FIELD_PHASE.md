# Native field phase and persistent development laboratory

The native engine now advances a represented field roster through target decisions and player movement, and resolves supported prepared terminal flight tails. This goes beyond the resettable shot calculation. It remains a development phase with explicit timing/prelude inputs, not a complete match.

## Player and team update

`advance_field_phase` copies a `FieldPhaseState`, iterates field IDs in original order and skips keepers and the current ball owner. It selects formation/tactic targets, consumes one low random bit per visited player, loads the on/off-possession movement profile, and advances the player. Stunned/nonzero-status players receive target decisions but retain their position/status. Stats and keeper role fields are preserved; consumed elapsed becomes0.

This models the supported ordinary in-field `$E074` phase under **interrupt-free staged inputs**. The original scheduler yields during the phase; live NMI/audio/controller/background refresh is not reproduced. The default native loop does not invent those updates. Full gameplay scheduling and interruption between players remain further work.

`MovementProfiles` uses the secondary type/profile mapping for raw speed words and metadata bytes for decision thresholds. Selectors33/34 are on/off-possession speeds, with no level adjustment. Existing effective-type/generic/alternate rules are reused; generic team content remains unavailable in the current export.

`select_field_target` follows `$A103`: four on-ball formation templates, twelve off-ball formation/tactic templates, mirrored coarse regions, one random bit, side11 cell rotation, and threshold overrides. On-ball local slots1..4 can receive an end-field target; off-ball overrides chase ball cell240. Invalid reached formation/tactic/mirror bounds reject.

## Replaceable private content

`CMJSHOT4` appends6,747bytes after the unchanged version3 sections:64direction words,8axis signs,35ordinary metadata triples, a bounded alternate metadata list, bounded raw speed pairs,15region-mirror bytes, and4+12templates of300bytes. The reference pack is18,580bytes. Readers retain versions1/2/3; new field operations require version4. All original content remains private. Source hashes record provenance, not authenticity.

## Typed field snapshot

`FieldPhaseState` contains22stat records,20field motion records, two separate keeper-role records, team formation/tactics, ball/possession/current-cell state, staged random bytes, selection count, movement region and elapsed. Field X/Y words correspond to record+5/+6 and+7/+8; keeper offsets retain their different meanings.

The private `CMJFIELD1` format is239bytes with a9-byte magic, ten header bytes, four team bytes,22four-byte stat records,20six-byte motion records and two four-byte keeper records. Loading rejects truncation, trailing data, unsupported side/owner/configuration; writing is explicit field serialization. It excludes experience, script/NMI/audio state, complete match clock/score and other unported fields. It is not a game save/network format or a claim that full RAM is equivalent.

Export an explicit2048-byte checkpoint:

```bash
python3 tools/export_field_state.py /path/to/private-ram.bin /path/to/private.cmjstate
```

The exporter validates the supported state shape, not a ROM identity. Outputs use exclusive creation and are ignored by Git; keep them outside the repository.

## Persistent native laboratory

```bash
./build/captain-field-lab /path/to/private-v4.cmjpack /path/to/private.cmjstate \
  --advance --capture /path/to/new.cmjstate
```

`--advance` uses the snapshot's elapsed byte; `--elapsed N` overrides it. `--interactive` retains the current state between `state`, `advance N`, `terminal CODE DEFENDER ACTION SUBTYPE`, `save NEW.cmjstate` and `quit` commands. Saves refuse existing files. The operation commits its result to the lab state only after calculation succeeds. This command layer does not enforce a full game's legal action sequence.

## Prepared terminal flight tails

`resolve_prepared_terminal_flight` accepts results2/3, an opposing field defender, actions0/2 and subtypes0/128. **The caller must supply actor stats/status, elapsed and RNG after the original reward/clock/presentation prelude.** Those effects are not inferred from a comparison code.

- Code2 copies the defender's current integer coordinates to the ball and clears selected count. It retains possession, elapsed and the prior ball-cell byte; recovery owns that cell's later recalculation.
- Code3 transfers possession/owner and runs the native field phase before returning player control. It does not teleport the ball to the new owner's position.

Use `--terminal 2|3 --defender ID` for one initial operation; optional defense action/subtype default0. The older complete prepared shot still reports terminal flight pending because it lacks this coherent persistent prelude/world context. No synthetic elapsed2 or fixed status decrement was inserted to conceal the gap.

## Evidence and remaining work

Eight original player observations and six banked target-decision observations are recorded privately. A native field advance from captured pre-`$E074` state produced the same239serialized bytes as the captured original post-phase state, including nineteen player updates. The prepared code3 tail likewise matched the captured player-control represented state. Code2 matched from its post-prelude checkpoint; an earlier checkpoint did not agree because reward/clock/status processing was still pending. That input-stage distinction is recorded rather than treated as an arithmetic defect.

These are production application runs and manual snapshot/hash inspection for particular interrupt-free observations, not an automated differential suite or full game equivalence. Production Make/CMake builds and independent whole-plan review are recorded in the work log/plan. No tests were added/run under the active constraint. Full prelude processing, live frame/random scheduling, subsequent recovery/player-control rules, persistent full-match commits, presentation and online play remain unfinished.

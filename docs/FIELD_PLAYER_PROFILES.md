# Field player profiles

`resolve_field_player_profile` selects a field player's skill profile from supplied content. `prepare_profiled_field_defender` connects that profile and the player's level to existing defense action/move preparation. Neither function changes match state or charges stamina.

## Contract

`FieldPlayerRecord` supplies the player ID, primary type, flags, alternate type, and level. IDs 1..10 and 12..21 are supported. Keeper IDs 0/11 and other IDs are rejected.

`FieldPlayerProfiles` supplies ordinary type mappings, alternate type mappings, field skill views, generic position offsets, and the caller-selected team's bytes. Type zero uses `generic_slot_offsets[player_id - 11]` to read a team type for opposing field IDs 12..21; generic own-team records are unsupported. A generic entry may resolve to ordinary type zero.

Types below35 index the ordinary mapping. Types35+ index the alternate mapping at `type-35`; when flag bit7 is set, the record's alternate type replaces that type. A replacement below35 is rejected. Ordinary types ignore this flag. Missing team bytes, alternate entries, and skill profiles throw `std::invalid_argument` before indexing.

Each skill view has31 bytes, covering the existing standard preparation selectors1..30. For the original data, views begin24 bytes apart: slots24..30 read into the following row, as the original code does. Replacement content may supply independently owned31-byte views. Stamina slot0 and special selectors31+ retain their separate, unimplemented profile paths.

## Original instruction evidence

Private analysis of the approved Japanese ROM:

- Bank28 `$803A` resolves the player record through `$C50C`; `$8040..8050` chooses primary or team-position type. Generic slot offsets begin at `$818E` and the caller's team-data pointer is `$38/$39`.
- `$8050..8090` selects ordinary4-byte entries at `$95D6` or alternate12-byte entries at `$9662`. `$8056..8062` handles the variant flag and replacement type.
- For field IDs, `$80D1..80F8` uses the entry's first byte to address the24-byte skill row at `$9FCE`. `$80FC..8112` applies level adjustment, already covered by existing preparation.
- The composed comparison enters the complete `$8224` defender loader and compares selected skill, final skill index, strength, effect, and cost with native output. It also checks that the original selected-player pointer identifies the expected record.

Run locally with the private ROM and py65 available:

```sh
PYTHONPATH=/tmp/captain-majid-python python3 tools/verify_field_player_profiles.py /path/to/reference.nes
```

The harness validates the approved payload hash, reads table values only into memory, compiles the data-free adapter `engine/tests/profile_preparation_driver.cpp` in a temporary directory, and sends content to that process through stdin. No ROM tables are stored in Git.

Evidence: **91,392 matching fixtures**, including all nonzero type bytes, four level boundaries, direct/override alternate paths, generic positions, action/situation/move combinations and4096 seeded cases. Team bytes and records are deliberately constructed. Byte-domain alternate table accesses do not establish which types appear in real matches, and no full-match behavior is claimed.

## Remaining integration

The runtime ROM/content importer and team-selection caller still need to populate these content objects. Automatic team selection, stamina consequences, attack/keeper profiles and ordered match commands remain later checkpoints.

# Nearby field-defender selection

The native selector mirrors the verified Japanese ROM routine at `$E4D7`. It fills the ordered field-defender list used by the action caller; it does not calculate defender strength or contact thresholds.

## Selection rule

- When the current possession side (`$05FB`) is 0, scan opposing field players 12–21 in ascending ID order and select at most five.
- When the side is 11, scan players 1–10 in ascending order and select at most four.
- Skip a player when their record's status byte at offset `+10` is nonzero.
- Compare player coordinates from record offsets `+6` and `+8` with ball coordinates `$0635` and `$0637`.
- Select the player only when both absolute coordinate differences are less than the radius. The area is a square, with its outer edge excluded.

The routine accepts the radius from its caller. The traced callers use radius 10 at `$8B75` and radius 7 at `$E4AB`. The native helper accepts any byte value, including zero.

## Native inputs and evidence

The helper takes the side, ball coordinates, radius, and 20 explicit player records ordered as IDs 1–10 followed by 12–21. This mirrors the selector's inputs while keeping player attributes and ROM data tables outside the selection helper.

`tools/verify_defender_selection.py` executes the original `$E4D7` routine and compares its ordered IDs with the native batch result. It checked 1,024 boundary and seeded cases against the verified Japanese reference ROM. This confirms the isolated selector; it is not a full gameplay trace. Arabic ROM selector differences have not yet been compared.

The development command `--defender-selection-batch` accepts 64 unsigned fields per line: side, ball X, ball Y, radius, then 20 triples of player X, player Y, and nonzero-status flag (0/1). It returns the selected count followed by that many player IDs. This research command is not an online protocol.

## Remaining work

The caller still needs to load player attributes, action-specific defense values, random inputs, and contact-threshold rows for each selected ID. Once those content lookups are reconstructed, the ordered IDs can supply the existing field-contact sequence directly. Animation and later action routes remain separate.

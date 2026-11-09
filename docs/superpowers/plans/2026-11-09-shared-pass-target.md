# Shared pass-target cell resolution

Objective: calculate the destination cell from an explicitly supplied recipient's position (or preserve the supplied cell for recipient255), without selecting the recipient, adding a score-specific encounter, or changing old snapshots.

Original evidence: controlled `$E059` execution matched the existing coordinate-to-cell formula for all65,536 byte coordinate pairs with recipient7, plus2,000 boundary fixtures spanning all20 field-player IDs; five recipient255 cases preserved the previous byte. These observations do not establish which recipient an AI chooses, keeper recipients or live timing.

1. [x] Implement a small pure resolver accepting field player IDs1..10/12..21 or255, a prior cell and explicit high coordinate bytes. Return255 for an out-of-field recipient position, preserve any prior byte for recipient255, and reject unsupported IDs.
2. [x] Compare the native resolver directly against original `$E059` execution over all65,536 coordinate pairs, all supported player IDs at boundaries and all256 preserved sentinel values. Keep ROM and observations private.
3. [x] Add an optional phase-session operation combining the resolver with the existing bounded pass-distance calculation and guarded target capture. Do not change the explicit target/distance operations or snapshot formats. Exercise it in both existing phase317 replays; keep recipient and previous cell supplied.
4. [x] Test invalid IDs, out-of-field coordinates, sentinel behavior, malformed table and atomic failure; run all registered tests and document coverage/limitations before pushing to work.

Verification:67,792 original/native resolver fixtures matched; the two existing phase317 replays and24 earlier sentinel-pass checkpoints matched; all58 registered Windows CTest checks passed. No additional score-specific scenario or snapshot format was added.

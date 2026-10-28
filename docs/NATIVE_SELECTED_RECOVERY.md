# Selected blocked-ball recovery

Phase28 can now recover a blocked ball while retaining the selected defender
prefix. The caller supplies the complete ordered actions. IDs alone do not tell
us whether an unvisited defender was assigned the action5 recovery preference.

## Commands

`--recover-selected-file ACTIONS.txt` (interactive `recover-selected-file`) reads
one decimal action0..6 per saved selected ID. Count/order must match. Visited
choices must use supported0/2, and the last must agree with saved shot progress.
The original first selected field action5 gets the ball when randomE2<64, even
when that player is in the unvisited suffix. Otherwise nearest-player recovery
runs. Missing/extra/invalid actions and repeated recovery reject atomically.

The implementation shares numeric recovery with the older zero-prefix entry:
owner cleanup, one jitter, field advancement, selection and coordinate writes.
It retains coordinate fractions and does not repeat clock, energy or rewards.

- Phase31 is original8C31: recovered ownership, before presentation tail. It keeps
  selected IDs even after possession changes to side11.
- `--continue-selected-recovery` (same interactive name) acknowledges the prepared
  tail and rejoins player_control1 for ordinary interior cases. Control setup
  clears the prefix, matching its existing original behavior.
- Keeper-area/story cases reject continuation with their saved phase31 intact.
- Phase32 retains an out-of-field jitter result before restart execution; selected
  IDs and progress remain available. Boundary coordinates serialize correctly.

Session11 marker B keeps330bytes and readers1–10. Older tags cannot carry31/32.
The format remains a represented development snapshot, not a complete game save.

## Verification

An early regression reached the new recovered-owner state but failed against the
missing phase validation; validation/codec integration then made the focused
checks pass. They cover nearest recovery, action5 suffix, input/clock/reward and
fraction preservation, return to control, boundary/save/load, malformed actions,
replay and story refusal. Full native/CLI/adapter/loopback/reference checks and
production Make build passed using the same-source temporary core archive
`CORE=/tmp/cm-recovery-core.a`. No compiler warnings/errors. CMake is unavailable.
Independent review reported no material correctness findings.

The original observer now supports `--selected-recovery`, preserving the explicit
prefix. Four constructed CPU observations match all324 prior represented bytes
against native CLI output after phase/version normalization. The six appended
progress bytes are native metadata and excluded from the original-memory claim:

| Case | Native330-byte SHA256 |
| --- | --- |
|Ordinary nearest recipient|446da1b0c5f963d23b5367ea2b90cd20e9d7823d3da3260d30538654cccd3ddf|
|Unvisited action5 suffix|3f965f0cf5a98aeb3c10a0cee1926670670ab3afde490d58bec90e848cc30a40|
|Action5 cutoff at random64|e7b38ab7ba0780f63b34d0aadd7b641a0339fbbae61248800d6f0467697b97a9|
|Outside-field jitter|810d2e596b44f7e8585c6bd9bd2c8a853462e87c18debc818b43dbf7c5d6da30|

Private evidence: `/workspace/Captain-Majid-analysis/selected-recovery-nq7x8zdi`.
These are explicit CPU/task observations with host frame/display acknowledgements,
not NMI/audio/PPU or full-match equivalence. Both ordinary and suffix capture
native CLI cases subsequently reached player-control setup. Presentation-facing
and later frame effects are not inferred from that prepared continuation.

Next: defense animation, selected keeper-area/story and boundary/restart handling,
then the nonterminal shot followup into approach/flight/keeper outcomes. Online
play remains dependent on a coherent complete match simulation.

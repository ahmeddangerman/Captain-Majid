# Ordinary pass candidate snapshot

Goal: preserve the original target candidate scan from the most recent cursor movement, with old snapshots remaining readable and unknown candidate state explicit.

1. Verify the pure side-zero candidate scan against two original target seeds and moved full-state observations.
2. Add a versioned optional candidate context, distinguishing newly initialized/unscanned from scanned; preserve version146 reads without inventing candidate state.
3. Update phase331 initialization and movement to store the observed scan only when the cursor changes; validate the stored prefix without recomputing it later.
4. Compare complete snapshots and the captured target replay, run all registered tests and possession regressions, then document and push. Confirmation and candidate chooser remain guarded until separately traced.

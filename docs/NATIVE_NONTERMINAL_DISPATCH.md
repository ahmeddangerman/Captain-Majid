# Nonterminal selected-shot dispatch

The phase27 dispatcher now accepts results0/1 as well as2/3/4. Both nonterminal
results execute the original83F5 field movement once, then stop at8408 before
script8. Native phase43 retains the complete selected prefix and progress, with
no reward, extra clock/energy debit, ownership transfer or presentation execution.
Zero-defender shots are supported with the original all-zero progress context.

Phase43 validates result0/1 and cursor=count. Cursor0 still requires result0 and
zero unused defender/action/subtype fields. The new phase is stored by session17
markerH,339bytes. Readers1..16 are retained; olderG rejects phase43. Existing
restart phases38..42 continue requiring result4 and their original cursor rules.
The same development operation `--resolve-shot-result` exposes this route.
Repeated dispatch rejects atomically. This boundary is an attacking presentation
request, not running ball flight or a complete shot.

## Repeatable original comparison

With py65 installed, invoke from the checkout using private inputs:

```
PYTHONPATH=tools python3 tools/check_nonterminal_dispatch_original.py \
  /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
  /private/selected-field-entry.bin /private/new-comparison-directory
```

RAM must be an explicit supported side0 ordinary selected-shot83F5 checkpoint.
The fresh output directory must be outside this checkout because it contains raw
ROM-derived data. The tool verifies the approved ROM payload hash, constructs
result0, result1, unopposed and longer elapsed movement cases, runs original CPU
instructions through8408, invokes the native CLI, compares all represented values
and checks replay rejection. It records each case and verification.json; any
mismatch fails with nonzero status. This is controlled input evidence, not live
match or presentation/frame equivalence.

Focused regression failed against the old terminal-only dispatcher, then passed.
Full native/CLI/adapter/loopback/reference suite and production Make build passed
with clean diagnostics using the current52-source archive. CMake remains absent.
Fresh review found no material issue; added its positiveG→H restart-save check and
corrected the older dispatcher documentation. The added save check also passed.
Four original/native cases match324prior represented bytes, excluding native
progress/presence metadata from ROM claims. Reports show no skipped calls or
frames before8408. Evidence: ../Captain-Majid-analysis/nonterminal-dispatch-yozb7frs
and the repeatable command's run at nonterminal-dispatch-repeatable.

Next original boundary: script8, then8410 SEC/9095 applies another defender-status/
clock step,847F applies active selected keeper status when present, then ordinary
shot action dispatches throughC618/DCFD. These are separate from the field advance
just connected; do not jump straight to flight or silently omit that extra timing.

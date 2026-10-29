# Saved shot distance and destination

Phase45 now executes native originalDCFD..DD08 setup: reset original061A counter
to255, compute062B distance and0638 destination, preserve the original carry/loose
route and consume exactly one random bit only for keeper-bound shots. Ball x/y,
current ball cell5FE, time, rewards, stamina and possession remain unchanged.
Destination638 is distinct from current cell5FE; target x/y can be derived from it.

`--prepare-shot-approach` (interactive name without dashes) requires phase45 and
versioned approach content. New phase46 stops before flight scheduling; it does
not run flight, contact scan, script, rendering or another clock step. Replay
rejects. Session19J appends presence/distance/destination/counter/loose (5bytes),
344total. Readers1..18 remain supported; I rejects phase46. Malformed/truncated
context, invalid flags, nonzero absent data and invalid initial counters reject.

Focused test failed with an unimplemented stub then passed. Default Make suite
and production build pass with clean diagnostics. Fresh review found no material
issue; added its requested malformed-tail and complete-state-preservation checks,
which also pass. CMake definitions updated but executor lacks CMake.

Six original/native cases match324prior represented bytes plus independent061A,
062B,0638 and carry: loose,127/128threshold, keeper even/odd and unopposed.
Native context presence is excluded from ROM-memory claims. The bounded observer
executes all original calls beforeDD08, with no skipped calls or frame/display
acknowledgements. Reproduce using private files:

```
PYTHONPATH=tools python3 tools/check_shot_approach_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/phase45-entry.bin /private/new-output-directory
```

Evidence: ../Captain-Majid-analysis/shot-approach-repeatable. Previous post-script
attack timing is documented in NATIVE_ATTACK_CLOCK.md. Approach checks do not
establish equivalence for unimplemented earlier scripts or later live flight.

Next observed E73E flight clears selected count/elapsed, uses fractional ball
coordinates634/636, calculates direction and signed velocity, advances the ball
until a cell change, then invokes cell/arrival callbacks. Fractional coordinates
are not yet in the saved flight context; do not silently initialize them to zero
or reuse player boundary clamping for ball flight. The native full frame/contact/
keeper pipeline remains unfinished.

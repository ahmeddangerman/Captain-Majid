# Saved fractional shot flight to first callback

Phase46 now begins prepared flight with explicitly supplied low X/Y coordinate
bytes, existing direction geometry and private CMJBALL1 velocity content. No
fractional zeros are guessed. Original selected count/prefix and elapsed clear,
transient selected progress is removed,062C direction is stored and the numeric
trajectory retains16-bit position/velocity, direction and061B flags0.

| Phase | Boundary | Meaning |
| --- | --- | --- |
|47|E773|Initialization complete; explicit frame acknowledgement still required|
|48|E7B4 before800F|New cell; defender/cell callback pending|
|49|E7CC before800C|Normal arrival callback pending; high bytes snapped, fractions kept|
|50|E7CF early return|Already at destination; no arrival callback or position snap|

`--begin-ball-flight FILE LOWX LOWY` starts from46. Fractions are decimal0..255.
`--advance-ball-cell` explicitly continues prepared numeric movement from47, with
no live frame/task sampling. Phase48 resolves through `--resolve-ball-cell`, appending eligible defenders in
original scan order up to capacity and resuming47. Existing entries and duplicates
are retained; random sample preserves the original extra borrow. Repeating the
callback at47 rejects. Pending49/50 still cannot advance as a new flight step. Interactive paths
can be quoted. CMJBALL1 must contain exactly64 unsigned decimal words0..65535;
negative tokens, wrapping/overflow tokens, truncated or trailing content reject.

```
PYTHONPATH=tools python3 tools/export_ball_velocity.py /private/Japanese-base.nes \
 /private/ball.cmjball
```

Original E73E early return leaves old velocity slots untouched. Those unused
slots are not captured before phase46; native50 normalizes its unused trajectory
velocity to0. It does not claim those bytes match original stale memory. The
oracle explicitly supplies nonzero stale original values and excludes that unused
metadata from ROM equivalence. If a future operation needs original stale velocity,
capture it before flight rather than treating native50's normalized value as data.

Session20K appends11bytes toJ344 (presence, four little-endian16-bit words,
direction, flags),355total. Readers1..19 remain supported. Shot context persists
46..50, trajectory47..50; selected shot progress is present only through46.
Validation checks phase/context presence, coordinate high bytes/current cell,
direction alias, zero elapsed, prefix capacity/unused slots, boundary destination and flags.
Only setup/50 require an empty prefix; candidates accumulate across47/48/49. Initial
ready velocity cannot be stationary. Malformed/truncated/unused payload rejects.

Real setup/step regression failed before implementation. An additional already-
arrived regression failed until50 was split from normal arrival49. Full Make
suite/build pass cleanly; content parser's negative-wrap bug was reproduced RED,
fixed with complete-token from_chars parsing, and checked with malformed content,
replay, interactive quoted path/save and the post-review suite. CMake unavailable.
Fresh review found no remaining material issues.

Six constructed original/native scenarios cover selected/unopposed, keeper-bound,
next-cell arrival, fractional arrival and already-arrived. Eleven setup/body
boundaries match prior represented state and newly represented trajectory fields,
subject to the unused stale-velocity normalization above. Original initialization
executes E73E toE773 without skipped calls. Numeric body explicitly starts after
the wait atE778 and stops before callbacks; it does not run a live frame, callback,
script8, renderer, audio or complete match.

```
PYTHONPATH=tools python3 tools/check_session_ball_flight_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/phase46-entry.bin /private/new-output-directory
```

Private evidence: ../Captain-Majid-analysis/saved-ball-flight-reviewed. Prior attack
clock/approach checks also pass with finalK snapshots. Next: native800F cell/contact
and800C arrival behavior, then real frame/keeper/goal flow. Do not execute arrival
callback for50, or skip pending callbacks to keep the ball moving artificially.


Seven full original87E1 callback comparisons cover prior counts0/3/4/5, sample255
rejection/sample254 inclusion and stunned candidates. Two complete prepared
numeric flights match original at every saved cell/callback through arrival with
queued defenders. This includes actual native direction, projection, movement and
prefix accumulation; it still freezes frame inputs and stops before arrival
encounters. Evidence: ball-cell-repeatable and full-prepared-flight-repeatable.

```
PYTHONPATH=tools python3 tools/check_ball_cell_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/phase48-entry.bin /private/new-callback-output
PYTHONPATH=tools python3 tools/check_prepared_full_flight_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/phase46-entry.bin /private/ball.cmjball /private/new-flight-output
```

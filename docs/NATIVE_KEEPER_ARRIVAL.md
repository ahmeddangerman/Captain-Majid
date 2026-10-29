# Saved keeper entry and stunned decision

Completed keeper-bound flight52 now dispatches through original DD1C into a saved
normal or stunned keeper entry. Current attacking session remains side0/action0.

| Phase | Original boundary | Represented behavior |
| --- | --- | --- |
|53|8509, before first presentation|Count1, selected keeper11, opposing player11|
|54|8900, before stunned random scan|Action0 and opposing player11, empty queue|
|55|8917, before presentation12|Saved result0/1 and cursor0|

`--prepare-keeper-entry` requires52; it reads the opposing keeper's stun, not the
scoring-side keeper. Normal53 is the sole exception that permits a keeper ID in
the selected prefix. Ordinary field/flight prefixes still reject keeper slots.
`--resolve-stunned-keeper T0 T1` requires54 and explicit two-byte threshold content,
with final threshold0. Original scan8928 uses16,0: random>=16 gives0, otherwise1.
Neither operation advances RNG, clock, energy, score, movement or presentation.
Interactive commands use the same names without dashes. Replay/wrong phase and
multiple initial CLI operations reject; errors preserve the source state.

Snapshot22M appends presence/cursor/result (3bytes),358total. Keeper decision context
is required exactly at55; result0..1, cursor0. Absent payload must be zero. Existing
readers1..L remain; L355-byte phase52 loads, but old versions reject new keeper
phases. Shot/trajectory contexts persist through55, with coherent arrived ball,
zero elapsed and saved keeper/loose route checks. Normal53 requires exactly its
opposing keeper;54/55 require nonzero opposing stun, action0 and empty prefix.

The real native test failed with the unimplemented adapter, then passed. The first
original comparison rejected a speculative three-entry threshold table. Inspection
showed scan8928 has only two reachable entries16,0, followed by opcodeA0; tests and
API were corrected before accepting original parity. Four status cases (0,1,127,255,
with the other keeper independently stunned/ready) and all256random values now match
all represented save bytes. Original DD1C..8509/8900 and8900..8917 execute every
instruction with no skipped calls. No display/frame acknowledgement is needed for
these bounded numeric comparisons. Evidence: keeper-entry-final/verification.json.

Five complete prepared flights match each saved cell/callback, including free ready
and stunned keeper routes; the latter also matches its decision. Frame inputs stay
frozen and waits are explicitly acknowledged. Normal keeper menu/power comparison,
animations, arrival encounters, goals/rewards/restarts and live match scheduling
remain unfinished. The old standalone keeper/outcome APIs are not proof that those
paths are connected to this persistent session yet.

```
PYTHONPATH=tools python3 tools/check_keeper_entry_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/keeper-flight-entry.bin /private/new-keeper-output
PYTHONPATH=tools python3 tools/check_prepared_full_flight_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/phase46-entry.bin /private/ball.cmjball /private/new-flight-output
```

Default Make suite and production build pass with clean diagnostics. Fresh review
found no material defect and independently probed forged prefixes and CLI guards.
CMake definitions updated; CMake is unavailable in this executor.

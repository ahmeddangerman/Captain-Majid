# Persistent non-goal keeper save handoffs

`--resolve-keeper-save` accepts saved normal keeper56 with final result2/3, after
an explicitly acknowledged presentation9. It applies the original keeper reward
(selector3 using raw type), timer/penalty changes and ball/possession handoff once.
It does not handle goals/special turnovers or guess scores, presentation or frames.

| Result | Saved handoff | Numeric behavior |
| --- | --- | --- |
|2 rebound|2 before8BDF|One keeper reward; penalty/timer; empty selected prefix; next ordinary recovery owns jitter|
|3 catch (action other than1)|57 beforeC633|One reward/timer; side11 keeper11 owns ball; original selected keeper prefix1 retained|
|3 punch (action1)|58 before8BE5|One reward/timer/penalty; punch placement; elapsed1; empty prefix; next recovery skips jitter|

`--recover-keeper-punch` continues58 through prepared8BE5 numeric recovery into
ordinary recovered-owner6 for either side. It advances the supplied elapsed field
state, without another clock/reward/stamina step or special-owner cleanup/jitter.
Ball cell remains the earlier stored cell through recovery; later player-control
setup refreshes it from the new carrier. Do not silently recalculate it earlier.
Catch57 is an explicit keeper-control boundary, not a field-player control state.
Interactive commands use the same names without dashes.

The adapter reuses the existing guarded result2/3 outcome calculations; its score/
kickoff arguments are unused and not committed. Story classification is already
saved at56, so it is not reapplied. Every supported source has original side0;
catch creates typed side11 keeper ownership. The role exception permits only
side11/owner11/count1/prefix11/opposing11/zero elapsed/stun in57; other field states
still require a field owner. Flight/decision contexts clear at the handoff and
replay rejects. Partial/invalid input leaves source unchanged.

Snapshot24O remains361bytes; readers1..N remain supported. N can read its existing
phase56 decision but rejects new57/58. Phase58 requires side0, empty prefix,
elapsed1 and opposing keeper11. Old flight validation must stop at56 before
reading a flight context:57/58 intentionally have none.

Real outcome and no-jitter recovery tests each failed with their unimplemented
stub, then passed.128 original handlers match all represented bytes, including
rewards/saturation, raw types1/33/64, timer wrap, penalty gates, all punch directions
and vertical signs. Original861C/8646 runs through8640/865B/86AE with every actual
reward/status/ownership/sound helper instruction executed; no calls are skipped.
Eleven punch cases separately compare prepared8BE5 recovery. Original presentation9
was acknowledged earlier; audio playback, rendering and live keeper control are not
claimed. Evidence: keeper-saves-final/verification.json.

Seven whole prepared numeric flights match every saved cell/callback, including
normal comparison and rebound/catch/punch handoffs, plus punch recovery:
full-flight-keeper-saves-final. Explicit frozen frames and prepared presentation/choice
boundaries remain necessary. This is not a playable complete match.

```
PYTHONPATH=tools python3 tools/check_keeper_saves_original.py \
 /private/Japanese-base.nes /private/japanese-cycle-v9.cmjpack \
 /private/normal-keeper-decision.bin /private/new-keeper-saves-output
```

Next: goal/special-turnover rewards, persistent scores and restarts, keeper control,
queued arrival encounters and live frame/presentation scheduling.


Review found a read of a cleared optional in an older route guard: its phase>=52
condition also reached new57/58. A fresh assertion-enabled full-core build aborted
on the disengaged ShotFlightContext (RED), while the default optimized build hid
it. Restricting that arm to52..56 makes the same assertion build/test pass (GREEN),
and final default Make check/all plus original128handlers/7joined flights pass.
A cleared-context fixture remains in keeper_save_session_tests. The assertion
build used a fresh external source copy and -D_GLIBCXX_ASSERTIONS atO1; it emitted
three existing maybe-uninitialized diagnostics in finish's optional-vector move,
also present before this fix. DefaultO2 build/suite diagnostics are clean. CMake
is unavailable. No further material review findings; source stays private.

# Prepared later-flight contacts

`resolve_prepared_flight_contacts` calculates one explicit burst of action0 contacts from the later original caller at `$8835/$888D/$88A8`. It loads each visited opposing field player and move, rolls/classifies the contact, and applies candidate loss on result1. This is different from the initial selected-defender caller, which preserves power.

## Contract

- Caller supplies side0/11, incoming power, selected opposing field IDs, defense actions and staged random bytes. Per-burst caps are5 for side0 and4 for side11. Position selection, repeated flight frames and random scheduling are not inferred.
- Original situation/subtype are0. Profile/move loading reuses existing content. The stamina cost is reported as preparation data but **not charged** in this caller; weakening is false. The input roster is read-only.
- Replaceable content contains two supported defense actions, each with an eligibility flag, variation pair and threshold row. The reference actions are0/2. Only their reachable action0 rows are exported. Adjacent instruction bytes are not treated as hypothetical tables for attacking actions1..4.
- Result1 applies the existing isolated loss arithmetic. Results2/3 stop the burst before later choices are inspected. Immediate terminal-handler effects remain a separate boundary.

## Complete-shot connection

`CompletePreparedShotInput::flight` is optional. Initial terminal field results and post-loop active-defender diversions skip it. Otherwise the wrapper calculates approach, runs the explicit burst, then evaluates loose-ball and keeper-status routing. Original `$DD0E` runs flight before `$DD11` checks the saved loose-ball carry; a loose endpoint can still meet players.

Nonterminal flight power feeds keeper comparison. Initial field contacts/power remain available in `result.flow.field`; later contacts/power are in `result.flight`. They are distinct phases. With no flight context, existing behavior is retained.

If `result.flight->stopped`, the wrapper returns no keeper/outcome and a continuation marked `other_continuation`. The explicit flight result/last defender/action describe the pending handoff. Initial `flow.field_outcome` continues to describe the initial phase; it must not be interpreted as a resolved later-flight terminal outcome. The CLI labels this pending state. No terminal-handler possession, recovery, rewards or restart writes are invented.

## Private data version3

The exporter writes `CMJSHOT3`. After the unchanged version2 geometry it appends16bytes: two actions, two eligibility flags, two two-byte variation pairs, and two four-byte threshold rows. The reference pack is11,833bytes. The reader still accepts versions1/2 and rejects unsupported versions, truncation, oversized variable sections and trailing data. Packs and tables remain private.

The reference export derives eligibility at `$88EB+action`, variation through selector5 of the `$8D93` pointer table with offset`action*2`, and thresholds at `$828C+action*4`. Source control includes these locator rules, not the original values. Eligibility uses X from before the original ASL; variation uses the doubled accumulator saved at `$3B`.

## Native laboratory

```bash
./build/captain-shot-lab /path/to/private-v3.cmjpack \
  --side 11 --ball-x 95 --defenders 0 --flight-defenders 4 \
  --keeper-level 80 --keeper-action 0
```

The scene supplies opposing slots1..4, action0, staged random0/100. Normal usage produces initial power33, four flight contacts and remaining power21, keeper strength177/ratio22, catch with possession/owner0 and ball(52,124), timer7/penalty0/duration0/stun0, count1/active0/kickoff0, attacker stamina1000 and keeper stamina980. These inspected values agree with the crowded continuous original observation documented in [mirrored shots](MIRRORED_SHOT_SCENE.md).

An ordinary level1 development shot stops after its first flight result3 and reports a pending handoff without preparing/debiting a keeper. This illustrates the supported boundary; that terminal native branch has not received a full original handler comparison.

`--flight-defenders N` requires version3 data when N is nonzero. Interactive `flight-defenders N` selects the same explicit burst; zero preserves the controlled scene. Side0 chooses opposing slots12..16, side11 slots1..4. These lists are development inputs, not live player selection.

## Evidence limits

Production Make compilation and normal CLI usage provide native evidence; fresh static review is recorded in the work log/plan. No automated tests or native differential suite were added/run. Automatic flight selection/movement, multiple timed bursts, other attacking actions, terminal handler state, general random scheduling and persistent match phases remain incomplete. The optional window retains its existing default controls and has not been rebuilt for this increment.

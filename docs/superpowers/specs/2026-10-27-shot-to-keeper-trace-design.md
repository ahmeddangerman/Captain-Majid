# Shot-to-keeper original trace

## Goal

Connect original attacking preparation, pre-contact scripts, selected defenders, post-loop movement and goalkeeper preparation/comparison in one bounded CPU/scheduler trace.

## Approach

Extend the existing loop observer with an explicit keeper endpoint. Start at `$829D`, after choices are set and before `$8F72`; retain the field-only endpoint unchanged. Observe inputs/outputs at real attacker and keeper checkpoints. Return terminal field/diverted routes explicitly. Stop at `$856B` before story/outcome scripts. Retain disclosed host frame/display acknowledgements and hardware guards.

## Constraints

- Shot mode accepts action0,0..5 selected defenders; field choices restricted to the observed pre-loop actions2/3. Other legal actions still need study.
- A field-only constructed comparison using action0 does not prove that action is legal in the complete shot setup. The pre-loop script's field-action selector rejects that assumption by indexing outside its supported branches.
- Original post-loop movement can consume random bytes; record stage-specific bytes instead of assuming a single unchanged pair.
- Track actual output boundary labels and CPU addresses. Reaching keeper comparison is not a complete shot outcome or playable game.
- No original content committed; no tests added/run; preserve field-only observer behavior.

## Next boundary

Goal/save scenario execution, native full-stage comparison and persistent match commit remain further work. A goal-script attempt that fails to reach a later handler must remain documented as incomplete.

# Selected recovery plan

Execute inline under continued authorization; spec: ../specs/2026-10-28-selected-recovery-design.md.

- [x] Regression first: block→recovery→prepared control, direct suffix action5,
  ordinary nearest selection, retained prefix, fractions/clock/reward preservation,
  boundary pause, malformed actions, replay and snapshot/version handling.
- [x] Share numeric recovery and add selected entry/continuation, validation and
  session11 codec. Preserve zero-prefix entry behavior.
- [x] Add strict recover-selected-file/continue-selected-recovery lab operations.
- [x] Compare original retained-prefix recovery states; run full suite/build,
  independent review and document exact boundaries; checkpoint source locally.

Review focus: no premature prefix clearing, side11 recipient validation, no
assumed action5 or repeated jitter/debit/reward, outside-field snapshot validity,
keeper/story gate refusal, backward snapshot handling.

Evidence and limits: ../../NATIVE_SELECTED_RECOVERY.md.

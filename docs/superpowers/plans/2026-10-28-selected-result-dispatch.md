# Selected result dispatch implementation plan

Inline under continuing authorization. Spec: ../specs/2026-10-28-selected-result-dispatch-design.md.

- [x] Write failing regression against stub: result2 retains ball/prefix and grants
  once, result3/action2 transfers owner, action0 awaits animation, result4 no reward,
  replay/invalid errors atomic, new snapshots roundtrip and reject old tags.
- [x] Implement field advance/reward/routes, validation and session10 codec.
- [x] Expose CLI resolve-shot-result and document original boundaries.
- [x] Compare four original dispatcher snapshots; run full suite/production build,
  review, log results and save source locally. Update daily Arabic note separately.

Review focus: avoid later-flight ball placement/extra clock/double reward; keep
selected prefix; movement precedes handoff; animation retains ownership; new phase
and file version validation reject malformed state.

Evidence: ../../NATIVE_SELECTED_RESULT_DISPATCH.md. All planned native checks, original dispatcher comparisons, review and local checkpoint steps completed.

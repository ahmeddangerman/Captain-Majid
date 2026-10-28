# Saved-session shot defenses

The phase-session lab now continues a phase20 ordinary shot through the saved
selected defender prefix. It uses the saved post-cost roster, power and subtype.
No second attacker preparation or energy debit occurs. Choices and random samples
are explicit development inputs; comparison tables come from the content pack.

## Command

`captain-phase-session PACK PREPARED.cmjsession --shot-defenders-file CHOICES.txt --capture NEXT.cmjsession`

The interactive equivalent is `shot-defenders-file CHOICES.txt`. Each selected
defender needs four decimal bytes: `action subtype random1 random2`. Player IDs
come from the saved prefix in order. For no defenders, use an empty file. Supported
ordinary choices are action0/2, situation0, subtype0/128. Missing/extra rows,
duplicate selected IDs, unsupported choices and repeated invocations reject.
Unused suffix rows are structurally validated but never prepared or charged after
a terminal contact. Every command computes a copied state before replacing it.

## Boundaries

- Result2/3 joins existing rewarded contact handling, clock/keeper recovery and
  loose-ball/possession handoff. Added time retains the pending contact; resume
  does not repeat defense calculation or rewards.
- Result0/1 retains power/roster and pauses at phase21, `shot_field_complete`.
  Approach, flight, keeper and their effects still need session integration.
- Result4 commits all22 status clears, keeper penalties and situation0, then
  pauses at phase22, `shot_restart_pending`. The existing interruption helper
  reports the restart route; its execution is still pending.
- Random bytes from the last reached contact persist for clock/recovery. With
  no contacts, saved random bytes are retained. No live random schedule is inferred.

Session8 keeps the324-byte layout; the version tag makes new phase21/22 explicit.
Versions1–7 still load; an older tag carrying a new phase is rejected. The private
RAM exporter still emits version7 up to original phase20; it is not evidence for
the new joined native boundaries.

This integration composes supported component calculations. It has no continuous
original-match comparison, live frame/presentation timing, per-nonterminal-contact
presentation/clock schedule, defender AI, restart execution or complete match.
Synthetic integration checks establish native state composition, not full original
shot-loop equivalence. See WORK_LOG for commands and observed results.

The follow-on [timing audit](SELECTED_CONTACT_TIMING_AUDIT.md) records the original nonterminal and final-dispatch steps still needed, including failed contacts preceding a later terminal result. Phase21/22 are calculation checkpoints, not original dispatch equivalents.

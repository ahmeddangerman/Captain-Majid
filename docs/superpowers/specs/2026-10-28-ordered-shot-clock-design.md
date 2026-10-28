# Ordered selected-shot clock progression

Continue the authorized timing work with a staged API alongside the historical
batch-calculation command. Each step accepts one explicit next defense choice,
reads current saved roster/power/status, resolves comparison/interruption, then
runs9095 with carry chosen by effective result (<2 affects defender; >=2 carrier).
Save cursor/result/last choice across phases23(next contact),24(added-time after
contact),25(final clock pending),26(added-time after final clock),27(dispatch ready).
Period-end retains the progress but cannot resume. No later defense is calculated
before a paused clock resumes; attacker preparation is never repeated.

Final elapsed is explicitly caller-supplied: original action0 table values are1
for no defenders (90DD) and2 after exhausting the prefix (9085). It advances elapsed,
keeper/shared recovery and clock directly, without another status assignment.
Terminal/interruption contacts skip that final step. Dispatch is a saved boundary;
rewards, possession handlers, scripts/frames and flight remain downstream work.
The original617 active count is zero for supported field-only defenders; reset the
represented control.last_carrier_cell alias on entry, matching original82F9.

Session9 appends a presence byte plus five progress bytes to session8 (330bytes).
Older readers1–8 remain supported; new phases/context require version9. Progress
is present only in these staged phases or at period-end. Old batch21/22 remain
explicit calculation checkpoints and cannot be upgraded without original context.

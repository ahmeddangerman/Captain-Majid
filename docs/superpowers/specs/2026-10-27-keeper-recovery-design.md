# Elapsed goalkeeper recovery

Continue the native reconstruction by implementing the original fixed-bank `$D235/$D263` state update, derived from the private verified ROM. Preserve original byte arithmetic rather than replacing it with a general real-time countdown. No original code or content is copied into the engine.

## Contract

Represent each keeper's stored timer(+5), penalty duration(+6), penalty(+7), and stun(+10), plus shared flags/timer at `$0449/$044A`. A pure `advance_keeper_recovery(snapshot, elapsed)` returns updated state for keepers0/11 in that order. Timer+5 remains unchanged. Original subtract/underflow decisions use wrapped byte sign bits, including unusual explicit inputs. A nonzero stun branch never also reduces the penalty, even if the stun expires. Penalty duration zero is eligible for underflow on the next positive step; an expired duration adds3 with byte wrap and reduces penalty by25 once. There is no catch-up loop.

## Connection

Add keeper runtime state and an explicit ordered list of elapsed recovery steps to the shared development shot scene. Recover before assembling its shot input, feed timer/penalty into comparison, and feed the same duration/stun into outcome inputs and the keeper-status gate. Expose these as development CLI options, with repeated `--recover N` appending steps and interactive `clear-recovery` resetting the list. Default state remains zero and no recovery runs unless explicitly requested. Each shot starts from the supplied state and replays the list. This is a constructed pre-shot checkpoint, not an assertion about when recovery belongs in a complete match.

Extend the existing original-loop tracer with before/after recovery observations at its real caller, preserving all existing guards and host disclosures. Native elapsed scheduling and persistent match updates remain outside this increment.

## Evidence and limits

Record bounded original routine observations and original continuous caller observations privately. Build production targets and exercise the normal shot laboratory. Do not add or run automated tests under the active developer constraint. Record incomplete independent review separately from author review. A correct recovery helper does not establish full simulation timing or original goal/save-script equivalence.

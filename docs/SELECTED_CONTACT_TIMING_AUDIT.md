# Selected contact timing audit — 28 October 2026

Added `contact_preludes` observations to `tools/trace_field_loop.py`. At original
bank26:$9095 entry they record defender/action, carry-selected status target,
clock/extra/elapsed, result, owner/side and current random bytes. Instrumentation
only observes; original calls and the existing host policy are unchanged.

Three constructed CPU observations start from the private prepared-shot RAM:
side0, owner2, selected defender12/action0, clock451, elapsed1. All original
code runs with the documented cooperative-task/frame/display acknowledgements;
there is no NMI, audio, PPU or hardware timing emulation.

| Contact | Explicit random | $9095 carry / status target | Dispatch clock | Dispatch elapsed |
| --- | --- | --- | --- | --- |
| Power45, result0 |100,100|set / defender|448|4|
| Power1, result3 |100,100|clear / carrier|449|3|
| Power45, interruption4 |0,100|clear / carrier|449|3|

The nonterminal observation enters9095 with carry set; it consumes one elapsed
unit there, then another two before field dispatch. The terminal and interruption
observations use the carrier branch and consume two. Historical two/five-defender
observations also show per-contact clock steps before their final dispatch step.

The new saved-session defense calculation is consequently a bounded integration:
its terminal prelude covers the existing result2/3 handoff, but nonterminal and
interruption timing, including nonterminal contacts before a later terminal one,
is not yet a complete ordered caller. Saved phase21/22 must not be interpreted as
an original field-dispatch-equivalent checkpoint. No full original/native match
agreement is claimed.

Next implementation: retain ordered contact progress and current random samples,
apply each failed-defender prelude before the next choice, preserve added-time or
period-end pauses, then apply the final dispatch timing once. Reuse the existing
contact prelude and keep terminal rewards from repeating across resume. Only after
this is connected should phase21 advance into approach/flight.

Raw observations remain outside Git under `/workspace/Captain-Majid-analysis`:
`session-shot-contact-clock-audit.json`, `session-shot-contact-clock-audit-pass.json`
and `session-shot-contact-clock-audit-capture.json`. They are research observations,
not automated native differential tests.

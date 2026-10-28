# Selected terminal-result dispatch

From timed phase27 result2/3/4, run the original83F5 field advance once, then
result-specific handling. Grant one selector2 defender reward for2/3; no reward
for4. Preserve clock and attacker stamina. Result3/action2 transfers possession
and requests its sound before player-control entry; result3/action0 retains the
old owner until its defense animation. Result2 retains ball position and selected
prefix at recovery entry. Result4 retains state at9366 restart entry.

Use new phases28(selected recovery),29(defense animation),30(restart entry), with
existing progress metadata. Result3/action2 clears progress and enters existing
player_control phase1. Repeated resolve is rejected. Nonterminal0/1 is rejected
before any mutation; its followup handler is a later task. Preserve original
selected counts at pending entries instead of invoking the later-flight handler.

Session10 uses marker A and the same330-byte layout; readers1–9 retained. New
phases require marker A. CLI resolve-shot-result is atomic. No scripts, animation,
recovery scan, extra clock, or downstream reward is inferred here.

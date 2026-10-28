# Selected terminal shot result dispatch

`resolve_session_selected_result` continues a timed phase27 result2/3/4. It first
runs the original83F5/E074 field advance, then handles the selected-field result.
The later-flight terminal helper is deliberately separate: its ball placement,
clock and reward count do not describe this initial selected-field caller.

## Routes

| Result | Native handoff | Original boundary | Immediate state |
| --- | --- | --- | --- |
|2|phase28 selected recovery|bank26:8BDF|One defender reward; retain old owner, ball and selected prefix|
|3, defense action2|phase1 player control|bank30:C60F|One reward, transfer owner/side, request possession sound|
|3, defense action0|phase29 defense animation|bank30:C630|One reward; owner/side remain unchanged until animation|
|4|phase30 restart entry|bank26:9366|No reward; restart setup still pending|

Clock and attacker stamina are preserved. Field advancement consumes existing
elapsed once, before reward and transfer. Result2 does not move the ball to the
blocking defender or clear selected players early. Field-only supported choices
have active-selected-keeper count0, so the original847F keeper status helper is
inactive. No downstream script, reward, recovery scan, animation or restart is
implied at these pending boundaries.

`--resolve-shot-result` and interactive `resolve-shot-result` expose this step.
Repeated invocation and result0/1 reject atomically. New phase28/29/30 snapshots
retain progress; a direct transfer clears progress and can use `--setup-control`.

Session10 uses marker `A` at the existing version byte and remains330bytes. Readers
1–9 remain supported; older tags reject the new handoff phases. This is a private
represented-state format, not a complete game save or network protocol.

## Evidence

The focused regression failed against the throwing stub, then passed for all
routes, reward count, movement ordering, no extra clock/debit, replay rejection,
invalid reward content, format roundtrip and legacy version9 phase27 loading.
The previously missing terminal-contact added-time save/resume route now also has
a passing direct staged-shot regression.

Four constructed original CPU dispatcher runs and normal native CLI operations
match all324 prior-format represented bytes after phase/version normalization.
The six appended progress bytes are native metadata and excluded from the
original-memory equivalence claim:

| Case | Native330-byte SHA256 |
| --- | --- |
|Block|acaa7ed37ee049113a1db1132dc893eb668673f1a5443bc7f303698b52dfcc95|
|Direct capture|d2d01e3be81332c2b7f8307a5e11378a9fbbaa1bc7b21a749697027a2f12efef|
|Capture animation entry|35ec2c061d3944a3efa00eedb8ce22816c91600d9494dc2b98ce342ae358bb14|
|Restart entry|d1b18dbd98ab5edb006ded66a7cc531e89f3cc03b291ad5d3252c46bd136cd15|

Private reports/RAM/snapshots are in
`/workspace/Captain-Majid-analysis/selected-dispatch-fcso5uke`. The observer starts
at83F5 with the same explicit task/frame/display acknowledgements as earlier
traces; it does not emulate NMI, audio, PPU or hardware timing. These dispatcher
comparisons begin from original observed entry states, including their existing
presentation-facing byte. They do not fix the earlier script-facing difference
or establish a full attacker-to-control match trace.

Full native/CLI/adapter/loopback/reference suite and production builds passed with
Make using a temporary archive of the same core sources
(`CORE=/tmp/cm-selected-core.a`); compiler diagnostics were clean. CMake is absent.
Independent static review reported no material issues. Manual direct-capture CLI
usage returned to control-ready state and selected side0 defender1. All four
handoffs rejected replay without altering source snapshots.

Next: selected recovery with its retained prefix and explicit action information,
then defense animation/restart and nonterminal shot followup. Selected action5
recovery preference must not be guessed from a saved ID-only prefix. Full live
presentation/frame/random scheduling, keeper/goal integration and a playable
match remain pending.

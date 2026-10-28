# Prepared restart options

The selected field/goal-area restart phases38/39 now enter the shared options
logic at original9880..98D5 with an explicitly prepared post-script state. The
caller must supply state after opening scripts43/31 and55 are acknowledged.
Those scripts are not executed here and their side effects are not inferred.

Opening clears ball mode062D and initializes cursor0624=4. Each frozen input
poll toggles bit64 once for PRESSED&12 before checking HELD&128. Pressed A alone
does not confirm. Confirming cursor4 returns to the original caller9381/94D7;
confirming cursor68 clears the cursor and stops at tactical UI98D5. No time,
movement, stamina, rewards, random generation, possession or shot values change.
Overlay/display/sprite requests remain presentation requests, not rendered output.
Original workspace11/12 and UI87 are outside the represented simulation snapshot.

## Saved state and controls

- Phase40: prepared restart options, cursor4 or68.
- Phase41: tactical UI pending, cursor0; further tactical input is not implemented.
- Phase42: options complete, cursor4; restart action/player choice remains pending.
- Native context retains the originating field/goal-area route across all three.
- Session16 marker G appends three bytes: context presence, route, cursor. Writer
  emits339bytes; readers1..15 remain supported. New phases require version16.
  Missing/unexpected contexts, invalid cursors/flags, unused bytes and replay fail.

Development CLI: `--open-prepared-restart-options`, then
`--restart-options-input HELD PRESSED` using decimal bytes. Interactive commands
omit the double dash. Example: input0/12 toggles once;128/0 confirms current row;
128/4 toggles before confirming. The normal capture/save commands preserve state.
CLI states label the pending tactics/caller boundaries explicitly.

## Evidence

Focused regression failed with `Restart options unimplemented`, then passed real
state, held/pressed ordering, both route identities, save/resume, atomic/replay,
legacyF and malformed-context cases. Full native/CLI/adapter/loopback/reference
suite and production Make build returned0 with clean compiler diagnostics using
`CORE=/tmp/cm-options-core.a`, an archive of the52current core sources. Current
executor has no CMake; its updated definitions were not executed.

Private `../Captain-Majid-analysis/restart-options-tedfarbr` contains12bounded
original/native observations (open/wait/toggle/confirm/tactics/toggle-back for both
routes), plus strict CLI input, replay and interactive save checks. All324prior
represented bytes match after phase/version normalization; cursor0624 matches
independently. Native context presence/origin route metadata is not claimed to
exist in the original RAM. Reports disclose all acknowledged wait/overlay/display/
sprite calls. The observer starts after the opening scripts, not at9379/94CF;
there is no claim of complete restart, frame, renderer or full-match equivalence.

Fresh independent review found no material issues. Its minor command-error
wording finding was corrected; the affected CLI was rebuilt and checked.

Next: trace and implement the tactical submenu and caller restart player/action
choices; then join remaining keeper/story and nonterminal shot/goal flows.

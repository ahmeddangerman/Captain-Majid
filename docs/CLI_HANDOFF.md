# Codex CLI handoff

The migration package contains full Git history/source and separate private
research. Public GitHub main holds the website; restore the engine from the
included Git bundle on branch work. CLI does not automatically inherit this chat;
AGENTS.md and this document carry the project context.

## Latest checkpoint - 2026-11-05: defended-pass outcomes and control

The saved defended-pass path now restores the stacked ball variant, handles
sampled defender results and subsequent defenders, and connects sampled capture
and blocked-ball recovery to prepared control, ordinary field rounds and action
menu entry. These are constructed original CPU comparisons with captured inputs
and cooperative presentation acknowledgements, not a live match. The original
contact context is now retained correctly; earlier comparisons omitted that byte.
The Windows build and 45 compiled test executables passed on this checkpoint.

The blocked-recovery menu and basic shot preparation also matched two constructed
cases. A subsequent capture-to-shot trace found a real mismatch: original flight
flag $061B stays 1 at new shot setup, while native initializes it to 0. Do not
build further on this path until its saved-state representation and consequences
are traced and corrected. Field-contact/story/special interruptions, boundary recovery,
active-control added time and some terminal clock routes remain unverified or
explicitly rejected; see the latest WORK_LOG entry for exact coverage and failed
fixture attempts. Preserve the existing uncommitted work.

## Earlier checkpoint ? 2026-11-05: first area-pass defender contact

Saved phase116 prepares the first queued area-pass defender and preserves the
original temporarily stacked ball variant and captured presentation cost in a
version66/marker132 snapshot. Phase117 compares the first defender and saves
its result before any outcome handler. Twelve preparation cases and 352 contact
cases matched sampled original fields/results; together the latter covered all
four result codes. Defender power, calculation context and preparation cost are
explicit captured inputs; presentation and their live production are not yet
connected. The Windows build and all 45 compiled test executables passed.

Next implement the original result handler and restoration of the saved ball
variant, then process subsequent defenders. See the latest WORK_LOG entry for
coverage, private comparison data and limitations.

## Latest checkpoint ? 2026-11-05: defender announcement boundary

A queued area-pass defender can now acknowledge the original announcement and
save phase115 immediately before the original pushes the ball variant. Twelve
constructed original cases matched owner, selected prefix, cursor and variant;
field and experience stayed unchanged independently in both implementations.
The Windows build, focused session check and all 45 test executables passed.
This is a prepared acknowledgement, not live presentation. Defender preparation,
contact, stack-value restoration and outcomes remain pending; see WORK_LOG.

## Latest checkpoint ? 2026-11-04: defended pass entry

A normal area pass arrival with queued defenders now saves phase114 before the
first defender's presentation, with possession and selection preserved and the
cursor reset. Twelve constructed original observations matched owner, ordered
prefix and cursor; remaining native snapshot bytes were preserved. Windows build
and all 45 compiled test executables passed. Actual defender presentation,
contact, outcome and continuation remain unimplemented. See WORK_LOG for limits.
Next trace the first defender's presentation and comparison, including the
original's temporarily stacked flag, before implementing their saved boundary.

## Latest checkpoint ? 2026-11-04: area continuations

Area action 1 now advances saved flag-1 ball flight through numeric steps and
cell callbacks, and unopposed arrivals transfer to the recipient and reach a
prepared keeper interaction. Action 4 saves the filtered encounter/control
boundary. Action 6 saves the destination, race, ordinary winner or timeout
recovery, and prepared ordinary control. These are bounded, constructed original
comparisons, not a live playable loop. The carrier experience award was missing
from earlier comparisons; it is now applied before actions 1/4/6 and those
boundaries were rechecked with experience bytes included. Version65/marker131
also preserves choices for action4/6 continuations and the new phases; older
snapshots remain readable but cannot recreate missing choices.

Windows CMake build and all 45 compiled test executables passed. Exact sample
counts and failed attempts are in the latest WORK_LOG entry. Next trace defended
pass arrival, action4 encounters, action6 keeper/story routes, and live
control/keeper-menu transitions before joining them to restart or full-match
control. Prepared presentation acknowledgements are not live frames.

## State at transition

Read the latest docs/WORK_LOG.md entry and `git log -5 --oneline` for the precise
checkpoint. Overall completion is unmeasured. There is no complete playable match.
Native Linux prototypes cover movement, control/menu input, prepared shots,
selected defender timing/dispatch through final area-result possession routing,
ordinary blocked recovery, capture clearance setup/races, and ordinary
winner/control and timeout/recovery handoffs. Pure keeper and goal/save
calculations exist separately. Restart/keeper/story, nonterminal shot/goal
integration, live frame/RNG/presentation and online matches remain work. A native
CMake build and its compiled C++ test executables passed on 2026-11-03; a complete
graphical match remains unverified.

Latest detailed references:
- NATIVE_AREA_ATTACK_CHOICE.md for opponent area choice/preparation and ordered saved defender sequence
- NATIVE_KEEPER_AREA.md for saved keeper-area interaction setup before its menu
- NATIVE_KICKOFF_FORMATION.md for verified kickoff formation placement
- NATIVE_KICKOFF_SELECTION.md for original kickoff team/receiver calculations
- NATIVE_SCORED_GOAL.md for persistent score/scorer reward before goal presentation
- NATIVE_KEEPER_SAVES.md for non-goal keeper handoffs and punch recovery
- NATIVE_KEEPER_ARRIVAL.md for saved keeper setup and stunned decision
- NATIVE_SAVED_BALL_FLIGHT.md for saved fractional flight, callback boundaries and empty-arrival loose possession
- NATIVE_BALL_STEP.md and NATIVE_BALL_VELOCITY.md for verified numeric flight pieces
- NATIVE_SESSION_APPROACH.md for saved shot destination before flight
- NATIVE_ATTACK_CLOCK.md for selected/unopposed post-presentation timing
- NATIVE_NONTERMINAL_DISPATCH.md for passed-defense/unopposed attacking entry
- NATIVE_RESTART_OPTIONS.md for prepared shared restart options and saved UI state
- NATIVE_SELECTED_RESTART_ROUTE.md for restart classification
- NATIVE_CLEARANCE_HANDOFFS.md and NATIVE_CLEARANCE_RACE.md
- NATIVE_SELECTED_RECOVERY.md and NATIVE_SELECTED_RESULT_DISPATCH.md
- PORT_STATUS.md for capability checkpoints rather than percentage guesses

## 2026-11-03 — final area sequence timing and possession

Latest checkpoint (2026-11-04): area action1 now has saved flight-setup phase96
and already-at-target phase97, using explicit prepared presentation/distance and
fraction inputs. Twelve numeric and 24 saved original comparisons passed; all 45
compiled C++ tests and the native build passed. Version65 marker131 is used only
for these new phases. Area flight movement/callbacks remain pending. See the
latest work log and `NATIVE_AREA_ATTACK_CHOICE.md` for coverage and limits.

Latest checkpoint (2026-11-04): the prepared area recovered-owner story-gate
entry preserves the selected defenders and captured facing. Ten constructed
original comparisons matched represented field; the native build and all 45
compiled C++ test executables passed. Story execution remains pending; the next
match-roadmap task is the area attack continuation. See `WORK_LOG.md`.

Latest checkpoint (2026-11-04): the area recovered-owner ordinary-field tail now
accepts an explicitly captured post-presentation facing and reaches player
control. Eleven constructed original comparisons matched represented field,
selection and facing; 45 compiled C++ test executables passed. The area story
route remains pending. See the latest work log and match roadmap.

Later checkpoint (2026-11-04): `continue-recovery` now connects the saved area
recovered-owner state to the keeper-interaction gate while preserving the
selected defenders. Twelve constructed original comparisons matched represented
field state; all 45 compiled C++ test executables and the native CMake build
passed. Ordinary-field and story area continuations remain pending. See the
latest `WORK_LOG.md` entry and `ROADMAP.md` for scope and next task.

Completed the saved final-result boundary after ordered defender sequences.
Phases 93–95 record and dispatch the final result: results 0/1 continue the
attack, result 3 with defense action 2 awards selector-2 experience once and
transfers possession directly, and result 2 or result 3 with action 4 enters
selected loose-ball recovery. Recovery commits the selected recipient; the
interior recovered-owner state retains the selected IDs, while the boundary
restart route clears selection and prior kickoff metadata. Added CLI dispatch
and recovery operations and snapshot version 64 (marker 130, 414 bytes). Older
snapshots remain readable; a version-63 sequence has no saved final result, so
it loads as unknown and cannot be dispatched by guessing.

Verification completed: all 720 complete defender sequences matched the original
ROM (696 contacts, 748 skipped entries, all four result codes and selected slots
covered). The separate original comparison matched 100 final dispatch states
and 60 selected recovery continuations. Focused keeper/session tests passed, the
native CMake build passed, and all compiled C++ test executables passed. The first
720-sequence rerun exposed a fixture that failed to preserve the prior result
byte across skips; the fixture and comparison script were corrected, and the
final rerun passed all 720 sequences.

No blocker remains in the tested sequence, dispatch, or selected recovery paths.
Live rendering/presentation, an active host frame loop and unfrozen runtime
randomness remain unverified; the ROM comparisons use explicit frame
acknowledgements and frozen randomness. The complete playable match is still
unfinished. Next: trace and connect the gameplay step after these possession /
recovery boundaries against the original, then continue the remaining
restart/keeper/story and shot/goal integration. Keep presentation and live
timing tracked as pending. Overall completion remains unmeasured.

## Restore layout

Recommended Windows setup: Ubuntu under WSL2. Store the project in Linux home:

```
~/projects/Captain-Majid
~/projects/Captain-Majid-analysis
~/projects/Captain-Majid-website   # optional separate public main checkout
```

Historical documentation references /workspace/Captain-Majid and
/workspace/Captain-Majid-analysis; map those to the first two local paths.
Private observations at /tmp were scratch only; required packs/ROMs/RAM/reports are
in the transferred analysis folder. Reinstall research dependencies in .venv.
Cloud credentials and binaries are not part of the migration.

## First local session

1. Read AGENTS.md and the current work log; inspect status/branch before modifying.
2. Confirm the restored private analysis directory exists.
3. Build `make -j2 all`; run `make -j2 check` for local verification. These do not
   require a ROM. Python CPU research needs py65 from tools/requirements.txt.
4. Inspect current snapshot readers and the latest implemented boundary before
   continuing. Do not repeat completed checkpoints or invent full-match parity.
5. Continue original restart/keeper/story and remaining shot flow toward one
   complete playable match. Update the work log and follow current AGENTS.md publishing rules.

The public website may be cloned separately from
https://github.com/ahmeddangerman/Captain-Majid.git. The user subsequently authorized engine source on its public `work`
branch; `main` is the website. The transferred bundle may initially have a local
origin; check the current remote before pushing. Never publish ROMs, patches or
derived private research. Repository privacy changes need explicit authorization.

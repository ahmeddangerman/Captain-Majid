# Port status and completion checkpoints

**Latest - defended-pass continuation (2026-11-05):** Sampled defender
outcomes, subsequent defenders, capture and ordinary blocked recovery reach
prepared control and ordinary field rounds. Sampled capture reaches action-menu
entry and ordinary shot preparation. Two blocked-recovery cases also match menu
and basic-shot preparation. A capture-to-new-shot flight setup comparison found
the original flag $061B is 1 while native initializes it to 0; that continuation
is not faithful yet. These are bounded original CPU comparisons,
not live play. Several interruptions and added-time routes remain unverified or
rejected; see the latest WORK_LOG entry. The Windows build and all 45 compiled
test executables passed. Overall match completion remains unmeasured.

**Latest ? first defended-pass contact:** The first queued defender can be
prepared and compared at saved boundaries; 12 preparation and 352 contact cases
matched the stated original values. The comparison requires explicit captured
scratch inputs. Restoring the ball variant, result handling, later defenders,
and live presentation remain pending. See WORK_LOG.

**Latest ? defender announcement:** Queued area-pass defense can now save the
acknowledged announcement boundary before the original temporarily pushes the
ball variant. Twelve constructed cases matched the stated state fields; live
presentation and the defender contact/outcomes remain pending. See WORK_LOG.

**Latest ? defended pass entry:** A normal arrival with queued defenders now
saves the first-defender boundary; 12 constructed observations matched owner,
ordered prefix and reset cursor. Defender contact and outcomes remain pending.
See WORK_LOG for the precise scope.

**2026-11-04 area continuation checkpoint:** Saved pass flight, cell callbacks,
unopposed recipient/keeper preparation, action4 filtering, and action6 race,
ordinary recovery and prepared-control boundaries are connected and compared
with constructed original observations. Experience was added to the action
continuations and rechecked. Defended pass arrivals, action4 encounter outcomes,
action6 keeper/story routes, live control/presentation, and complete keeper or
restart flow remain unfinished. See the latest WORK_LOG entry for exact checks.

**Overall completion is not currently measured.** The previously repeated7% was an unsupported planning guess and is retired. The checkpoints below describe observable capabilities; they have different sizes and must not be averaged into a game-completion percentage.

## Latest verified step — 2026-11-03

The supported opponent area sequence now connects action/recipient selection,
attack preparation, prepared defender announcements, ordered supported defenders'
saved comparison/stamina, post-contact recovery/status/clock and prepared result
display. It handles skipped entries and terminal early exits while preserving
added-time and period-end boundaries. Final sequence timing now selects its elapsed value from private original game
data and has a saved continuation. Supported result dispatch and selected
recovery are connected. Other defensive choices and a complete playable match
remain unfinished. See [NATIVE_AREA_ATTACK_CHOICE.md](NATIVE_AREA_ATTACK_CHOICE.md)
and the later evidence in [WORK_LOG.md](WORK_LOG.md).

On 2026-11-04, 12 sampled original area recovered-owner continuations reached
the keeper-area interaction with the selected count preserved. Two separately
constructed ordinary-field and story-gate continuations reached the expected
control and story boundaries while retaining one selected defender. The native
keeper-area continuation is connected and matches 12 sampled original fields.
The prepared ordinary-field area continuation matches 11 constructed original
fields with captured facing. The prepared story-gate entry matches 10 constructed
original fields, with selected defenders and facing retained. Story execution,
broader coverage and live timing remain unverified.

The next area-attack dispatch was observed from original `$8A6F` through
`$8A80` for six constructed action/result combinations with zero elapsed time.
Six further constructed traces pass through the common `$8BBA` call and
presentation into action-table targets `$C612`, `$C627` and `$C62D`. The action
handlers and native continuation remain unimplemented.

Four constructed action 1 observations continued through `$C612`/`$DE52` to
`$DE5E`; byte `$061A` was 255 or 38 according to bit 7 of `$0628`.

Eight constructed action 1 traces now continue through the shared `$E059` call
to `$DE61`: selected recipients refresh the target cell, while recipient 255
leaves it unchanged. Eight constructed traces additionally reached `$DE64` after the
distance-related `$DF8B` call; native integration and full distance coverage
remain pending.
Ten constructed action 1 traces continued through `$E73E` to flight setup:
eight reached `$E773` and two already-at-target cases reached `$E7CF`, with
selected count cleared and flag `$061B` equal to 1. This area flight path has not
been connected to separate saved area-flight setup boundaries. Twenty-four
constructed original comparisons match represented field, trajectory, recipient,
counter and captured presentation inputs; actual area flight movement and
callbacks are still pending. See [NATIVE_AREA_ATTACK_CHOICE.md](NATIVE_AREA_ATTACK_CHOICE.md).
See the playable-match checkpoints in [ROADMAP.md](ROADMAP.md).

## Current capabilities

| Area | Current state | Completion checkpoint |
| --- | --- | --- |
| Original behavior research | Private CPU observations and many historical component comparisons; full frame/audio behavior remains outside the harness. | Supported gameplay phases have documented original inputs, outputs and timing boundaries. |
| Native shot calculation | Development lab joins preparation, initial contacts, one later burst, keeper comparison and immediate outcomes in both directions. | Gameplay supplies the inputs from a coherent match rather than a resettable constructed scene. |
| Movement | Native target-direction, speed profiles, formation/tactic targets and ordered field advancement implemented for staged interrupt-free inputs. Live frame/random scheduling and ball-flight progression remain pending. | Player and ball state advance coherently through action and possession transitions. |
| Persistent match state | Phase-session lab supports repeated ordinary walking cycles, selected encounter menus and ordinary shot preparation with clock, stamina, roster and selected defenders. Explicit selected shot calculations have saved boundaries; staged per-defender/final timing now preserves added-time pauses and now enters terminal selected-field dispatch with one reward, direct capture/control and selected blocked-ball recovery, direct/ordinary recovery return to control and capture-clearance arrival race with saved winner/timeout and ordinary winner/control and timeout/recovery handoffs, selected field/goal-area restart routing and prepared shared restart options with saved input/route state; tactical UI, actual restart action and keeper/story paths pending. Nonterminal selected results reach saved attacking-presentation entry and prepared selected/unopposed clock/status continuation into shot approach, including added-time save/resume and saved distance/destination setup and explicit fractional flight with saved per-cell defender accumulation through arrival; empty-arrival routing, prepared loose-ball recovery back to possession, keeper setup, saved stunned decision and prepared normal keeper skill/comparison/story decision and persistent rebound/catch/punch rewards, keeper possession, punch recovery and prepared score/scorer-reward registration and post-menu kickoff handoff, plus kickoff team/receiver calculations and saved prepared 22-player formation placement with keeper coordinates, receiver selection, initial ball placement and a prepared post-display possession handoff and connection to saved control setup, first button polling, the ordinary clock/energy handoff, chained directional field rounds, action-menu confirmation, ordinary shot preparation, unopposed timing/destination, numeric kickoff flight, prepared normal keeper comparison, goal registration, prepared post-goal restart and controlled non-goal keeper handoffs and punch and interior rebound recovery, ordinary control resumption and prepared keeper-area interaction setup; queued arrival encounters, defender AI, live flight scheduling, goals/saves, other action routes, live frame loop and full-match state remain unfinished. | Atomic roster/ball/clock/score/phase commits, save/replay snapshots and supported next actions. |
| Playable match | Not implemented. | Two teams can start, finish and restart a complete match with supported rules and controls. |
| Campaign and team AI | Not integrated. | Original team setup, progression, AI decisions and match scheduling work in native play. |
| Game presentation and languages | Private content analysis and placeholder native window; no complete runtime graphics/music/text/cutscene system. Fresh expandable translation content and right-to-left Arabic are planned; the runtime text pipeline is not implemented. | Arabic, Japanese and English game flows render and play correctly, including Arabic shaping and mixed-direction text. |
| Quality of life | Planned after a stable playable match loop; settings, input, save and presentation boundaries should be considered during the port. The exact options remain undecided. | Agreed PC conveniences work in a complete match without silently changing default or online rules. |
| Windows/Linux application | Linux development builds and historical optional-window capture; native Windows CMake build and C++ test executables previously passed. A complete Windows application and match remain unverified. | Installable builds, input, saves/settings and desktop behavior work on both targets. |
| Mods and visual/audio upgrades | Replaceable development data pack exists. User-facing asset/music replacement and shader systems pending. | Community packs can replace supported content and shaders without changing simulation or desynchronizing online play. |
| Online friend match | Ordered fixture and loopback proof exist; no playable online match. | Two peers complete an actual match with identity, invitations, synchronization and recovery. |
| Matchmaking/additional participants | Planned. | Supported rules, sessions, participant roles and random matchmaking operate with the real match simulation. |
| Release readiness | Not reached. | End-to-end compatibility, packaging and recovery evidence supports a public playable release. |

## Current engineering sequence

1. Connect selected keeper/story/boundary restarts and nonterminal shot followup, then approach/flight and keeper outcomes; extend other action/AI decisions, goal-area, restart/keeper/story routes and remaining reward callers.
2. Extend field snapshots into coherent match phases with clock/score, recovery and player actions.
3. Integrate live ball-flight and frame/random scheduling with atomic state commits.
4. Complete playable match, original presentation and desktop integration.
5. Build online match synchronization on that simulation, followed by invitations/matchmaking and broader participant rules.

Online constraints continue to guide state ownership and deterministic scheduling during the earlier steps. No server or networking success is inferred from isolated arithmetic or the loopback fixture.

See [WORK_LOG.md](WORK_LOG.md) for commits, evidence, corrections and resume notes. Production builds, original observations, static reviews and automated test results are recorded separately. A build does not establish original gameplay equivalence.

# First engine foundation progress

**Date:** 2026-10-27

**Scope:** Native skill-index, action-power, field-defender, and goalkeeper comparison calculations, plus a development replay fixture. Stamina, keeper story/state, and a single original random step are verified in isolation. The goalkeeper outcome handoff and an explicit shot sequence are implemented. A playable full shot interaction remains incomplete.

## Implemented

- Portable C++17 core with no rendering, audio, networking-library, or ROM dependency at runtime.
- Original action-power contact-loss behavior, independently implemented from the verified Japanese ROM contract in [ACTION_STRENGTH.md](ACTION_STRENGTH.md).
- Defender strength variation, weakened/special branches, score calculation, threshold classification, and conditional contact loss, specified in [DEFENDER_CONTACT.md](DEFENDER_CONTACT.md). Coefficients and thresholds are inputs; the engine bundles no original ROM tables.
- Standard level/move index adjustment, randomized action power, and the pre-override goalkeeper comparison, specified in [SHOT_POWER_AND_KEEPER.md](SHOT_POWER_AND_KEEPER.md). The shared defender roll is reused for goalkeeper strength.
- Stamina deduction with the original special-player surcharge, raw subtype exemption, wrapped subtraction, weakening, and outgoing carry, specified in [ENERGY_USAGE.md](ENERGY_USAGE.md).
- Keeper story classification override and explicit timer/penalty helpers, specified in [KEEPER_STATE.md](KEEPER_STATE.md). The shot caller still needs to select the appropriate helpers and apply complete consequences.
- One original random-generator step plus a canonical 2 KiB RAM sampler, specified in [RANDOMNESS_NOTES.md](RANDOMNESS_NOTES.md). Its original gameplay schedule is unresolved.
- Immediate goal, turnover, rebound, keeper catch, and keeper punch writes at the goalkeeper handoff, specified in [SHOT_OUTCOME.md](SHOT_OUTCOME.md). A calculation sequence links energy, shot power, goalkeeper comparison, and these outcome writes with explicit caller data.
- Ordered field-contact calculations carry power between caller-selected defenders, initialize each contact with that defender's own subtype, and stop on original result codes 2/3, as documented in [SHOT_FLOW_TRACE.md](SHOT_FLOW_TRACE.md). The supported action-0 continuation now applies its route gates and connects eligible results to keeper and goal/save calculations.
- Field result handoffs now distinguish continued play, ball recovery, keeper control, field-player control, and the other defense animation route. `resolve_field_contact_handoff` joins those results to the ordered contact calculation. The code-3 possession writes match 144 original-ROM cases; the ordinary interior-field code-2 recovery handoff matches 512 cases.
- The original nearby-defender selector at `$E4D7` returns opposing player IDs in ROM order, applying the side-specific limit, nonzero status skip, and strict square-radius test. Its output matches 1,024 original-ROM cases; caller-supplied radius and later contact data remain explicit.
- Standard field-defender preparation now selects a skill by defense action/situation and a move by subtype, then loads strength, move effect, and stamina cost from replaceable content. Its contact wrapper supplies the loaded strength and subtype to the existing comparison. The complete original `$8224` loader matches 16,064 fixtures; see [FIELD_DEFENDER_PREPARATION.md](FIELD_DEFENDER_PREPARATION.md).
- Field-player profile resolution now supplies defender preparation from ordinary types, generic opposing-team positions, and alternate player forms. The composed path matches 91,392 constructed original-ROM fixtures; all tables remain caller-supplied. See [FIELD_PLAYER_PROFILES.md](FIELD_PLAYER_PROFILES.md).
- Prepared defender sequences now load each visited player and move, apply defensive stamina use, and feed weakening into ordered contacts, returning an updated roster. Terminal results stop later energy use; repeated choices use current stamina. See [PREPARED_DEFENDER_SEQUENCE.md](PREPARED_DEFENDER_SEQUENCE.md).
- The prepared field-shot handoff now joins updated defender stamina and contacts to field recovery/possession, then routes eligible action0 attempts through goalkeeper comparison and goal/save outcomes. It derives the last visited defender, remaining power and ownership from preceding stages. Keeper and attacker subtypes remain separate; see [PREPARED_FIELD_SHOT.md](PREPARED_FIELD_SHOT.md).
- Attacker move preparation now resolves profile/skill, move cost and raw flags, applies the side-specific energy rule (including action2/no-defenders cost0), and feeds weakening into shot power. Its action0 subtype selector special case is implemented; see [PREPARED_ATTACK.md](PREPARED_ATTACK.md).
- A fixture session that separates participant identity from team, rejects unauthorized and unordered commands before state changes, and serializes every fixture state field explicitly.
- A native terminal executable with a batch arithmetic mode and strict replica command protocol.
- A Python development adapter that sends commands through loopback TCP to two separate native processes, then compares their complete state after every command.
- Make build/check commands and CMake build definitions for Windows/Linux development.

## Evidence

The initial missing implementations failed linkage as expected. The completed core and terminal checks pass on Linux with GCC. The ROM comparison executes the isolated original 6502 routine and matches the native output in 6,656 boundary and seeded cases. No third-party source, ROM data, graphics, audio, or translated comments are included.

The local TCP proof exercises 15 commands, a deliberately delayed replica, duplicate/future commands, unknown/unauthorized participants, malformed input, depletion of power, and connection cleanup. It requires matching full snapshots and expected replies after every command. Unit checks also require different fixture configuration to produce a different snapshot.

An independent code review found no critical or important issues in the defined scope. Its three minor findings were addressed: initialize sockets before spawning the native child, enforce a deadline for an entire protocol line rather than each arriving byte, and use the virtual-environment interpreter in the ROM comparison instructions. Regression checks reproduced the process leak and the absent deadline before the fixes and now pass.

The expanded defender calculation matches original ROM execution for 19,200 cases across all eight exposed outputs. These include 6,511 special-branch cases, 4,584 weakened cases, 3,093 zero-strength cases, and 1,111 conditional loss cases. The comparison reads 64 nonzero-range coefficient pairs from the user's ROM and checks all three field-defender threshold rows. These are isolated, deliberately constructed fixtures rather than captured full-match traces.

The defender increment received an independent review with no critical or important findings. Its one minor verification finding was addressed by adding explicit incoming-power wrap boundaries (1023, 1024, 1025 and other limits) to the original-ROM corpus, alongside the defender-power sweep.

The TCP experiment also compares ordinary, weak, and special defender calculations and rejection of invalid coefficients. These stateless research requests leave the original ordered fixture state unchanged.

The next original-ROM comparison passes 69,632 skill-index cases, 7,456 randomized action-power cases, and 5,248 goalkeeper cases. Skill checks cover every base-byte/level-byte pair with varying move modifiers plus seeded extra cases. Action checks include carry, critical, weakened, and truncation boundaries. Keeper checks include both team-side pointer paths, threshold equality, numerator wrap, zero strength, and state-adjustment saturation. Every exposed output is compared with original instruction execution.

The loopback proof also checks skill adjustment, action power, and goalkeeper comparison, including using a calculated action power as the next keeper query's input. These remain research queries, not integrated shot commands or complete match state.

Independent review of the skill/action/keeper increment reported no actionable findings and reproduced its full native checks and all three new ROM comparison sets. The earlier defender comparison also remains green after extracting the shared strength-variation code.

### Current increments: original ROM verification

The new stamina, keeper story/state, and random-step APIs are exposed through strict batch modes and stateless replica queries. Research parsing lives separately from the session driver. The new queries do not access or update the ordered session.

The private-ROM harness passed 9,728 stamina cases, 6,812 story cases, 5,632 keeper-state cases, and 3,584 random-step cases: 25,756 total. It uses mirrored RAM for the original random routine and checks the exact reference payload before executing it. The existing native and loopback regression suite also passed. These are isolated comparisons rather than complete match traces.

The next goalkeeper outcome check matched 384 isolated original-ROM cases across goal, turnover, rebound, catch, and punch branches. It checks immediate match-state writes through the handler handoff; animation, experience, and music calls are skipped in that comparison. The field result-three possession handoff matches 144 further ROM cases. Ordinary interior-field result-two recovery matches 512 cases, including recovered-player coordinates. The linked shot calculation has a native unit check, while an end-to-end original gameplay trace remains pending. [SHOT_FLOW_TRACE.md](SHOT_FLOW_TRACE.md) documents the field-defender loop and its current handoff coverage.

The action-0 continuation now connects eligible field results 0/1 to the keeper comparison and its goal/save outcome writes. Its native checks cover a goal, rebound, keeper catch, punch, possession changes, action codes outside this helper, each documented diversion gate, and rejection of field result codes 2/3. It consumes caller-supplied post-contact power/subtype and route values; it does not load player/content data or reconstruct the complete gameplay trace. The existing 384-case original-ROM outcome comparison still verifies the outcome stage, while the new continuation gate has native tests and instruction-trace evidence rather than a separate 6502 differential harness.

The new `resolve_field_contact_handoff` wrapper joins ordered contact results to the existing immediate defender handoff. Native tests cover code-2 recovery, code-3 player possession, the deferred animation route, and code 0/1 continuation. The individual outcome calculations retain their existing 144- and 512-case original-ROM comparisons; the wrapper itself has native integration tests rather than a separate 6502 differential harness. Nearby-defender ID selection now matches 1,024 original-ROM cases, but player stats, defense actions, threshold tables, and persistent match-state updates are still outside the contact handoff.

The field-player profile increment passed the full Make suite and all 20 CMake/CTest checks. The earlier preparation comparison still matches 16,064 cases after extending its private helper, while the composed profile comparison matches 91,392 constructed cases. Independent review reported no actionable findings and reproduced the targeted native and composed ROM checks.

The prepared defender sequence has native integration coverage and passed the full Make suite and all 21 CMake/CTest checks. Additional generic-primary-type and stale-comparison-input tests passed targeted rebuilds afterward. The profile/preparation verifier still matches 91,392 original-ROM fixtures and the energy verifier 9,728; these validate components rather than the complete connected original loop.

The prepared field-shot handoff passed the full Make suite and all 22 CMake/CTest checks. Existing component ROM comparisons remained green: 384 goalkeeper outcomes, 512 recoveries, 5,248 keeper comparisons, 7,456 action-power cases and 69,632 skill-index cases. The joined wrapper has native integration tests; a complete original gameplay trace remains pending. A regression reproduced and corrected the inherited attacker/keeper subtype mix-up in the older shot composition.

Attacker preparation passed the full Make suite and all 23 CMake/CTest checks. Existing skill/action/keeper/energy original-ROM comparisons also remained green. The preparation wrapper itself has native tests; its entire loader/caller has not yet been compared as one original execution.

Attacker preparation passed the full Make suite and all 23 CMake/CTest checks. ROM instruction tracing caught and corrected the side11 cost-override path and energy carry into weakened shot power. Component skill/action/keeper/energy comparisons remain green; the new wrapper itself has native integration tests.

## Limits

- The ordered fixture still advances only a contact-loss stage. The direct shot sequence and gated action-0 continuation accept explicit statistics, route values, and random values; they are not yet ordered match commands or playable interactions.
- The goalkeeper outcome handoff updates score, possession, keeper state, and punch placement. Code-3 field-defender handoffs and ordinary interior-field code-2 recovery are implemented; active-ball updates, boundary restarts, field-defender rewards, and full phase transitions remain to be integrated.
- Standard field-defender preparation loads strength and cost from supplied content, using a decoded player profile. Ordinary/generic/alternate field profile resolution is implemented; runtime content import, team-data selection, attack/keeper move preparation, and attacker/keeper stamina accounting and persistent match-state commits still need integration.
- Shot arithmetic receives explicit random bytes. A single original generator step is implemented, but its inputs and frame schedule still need study before integrating a complete deterministic match.
- Static inspection identifies a frame-interrupt random generator that mixes player and sound-driver memory. [RANDOMNESS_NOTES.md](RANDOMNESS_NOTES.md) records why a gameplay trace is required before choosing simulation scheduling compatible with replacement music and online play.
- The fixture's initial power 300 and loss 37 are developer-selected inputs. They are not claimed to be an actual player's statistics.
- This snapshot is a comparison format; save loading and network state recovery are not implemented.
- Python supplies TCP transport for this experiment. Native transport, protocol/version negotiation, identity authentication, encryption, reconnection, and host validation remain future work.
- A sender supplies a participant ID in this local development protocol; the ID is not authenticated. The adapter binds only to loopback and is unsuitable as a public game server.
- No private-room service, invitations, random matchmaking, ranked modes, or shared-team game rules exist yet.
- Windows compatibility is intended by the project structure but remains unverified. Current build evidence includes Linux/Make/GCC and Linux/CMake/CTest.

## Next checkpoint

Connect prepared attacker power to the joined field-shot handoff; resolve goalkeeper profile/move/stamina next. Then capture a complete original shot trace before introducing an ordered action0 match command. Persistent state commits must include roster, ball and outcome writes together. Add the runtime content importer and team-data selection caller. Trace the remaining comparison-table selection, action 1 and other phases, and gameplay random scheduling. Extend the keeper handoff through boundary restarts in a complete gameplay trace before making the shot an ordered match command.

Early network comparisons continue alongside each simulation increment. Public matchmaking and the rules for additional participants remain part of the engine direction, without committing to an unsupported player count.

# Codex CLI handoff

The migration package contains full Git history/source and separate private
research. Public GitHub main holds the website; restore the engine from the
included Git bundle on branch work. CLI does not automatically inherit this chat;
AGENTS.md and this document carry the project context.

## State at transition

Read the latest docs/WORK_LOG.md entry and `git log -5 --oneline` for the precise
checkpoint. Overall completion is unmeasured. There is no complete playable match.
Native Linux prototypes cover movement, control/menu input, prepared shots,
selected defender timing/dispatch, ordinary blocked recovery, capture clearance
setup/races, and ordinary winner/control and timeout/recovery handoffs. Pure keeper
and goal/save calculations exist separately. Restart/keeper/story, nonterminal
shot/goal integration, live frame/RNG/presentation and online matches remain work.

Latest detailed references:
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
   complete playable match. Keep the engine private and update the work log.

The public website may be cloned separately from
https://github.com/ahmeddangerman/Captain-Majid.git. Engine work must not be pushed
there. The transferred bundle's origin initially points to a local file; leave
that safe local origin until a private source remote is explicitly configured.

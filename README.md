# Captain Majid

A native PC reconstruction project for Captain Tsubasa II, targeting Windows and Linux, with Arabic, English, community graphics/music packs, shaders, and online play as project goals.

The current engine is an early development foundation: verified skill-index, action-power, field-defender, goalkeeper, stamina, story/state, single-step random, and ordinary loose-ball recovery calculations, plus an ordered replay fixture. It calculates immediate goalkeeper outcomes and field-contact results using explicit statistics. A playable match has not been implemented.

See the [development log](docs/WORK_LOG.md) for current milestones, local commits, evidence and remaining gaps.

## Build the prototype

Linux, with a C++17 compiler, Make, and Python 3:

```bash
make
make check
```

The complete check command covers the native core, terminal input handling, and a two-process loopback TCP experiment. No ROM or third-party Python packages are needed for these checks.

A CMake definition is also supplied for Windows/Linux toolchains:

```bash
cmake -S . -B build
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

Linux builds and checks have been exercised through Make and CMake. Windows execution remains unverified.

## Try the development fixture

For a native shot calculation using private game data, see [local shot content and laboratory](docs/LOCAL_SHOT_CONTENT.md): export a `.cmjpack` from your own verified ROM, then run `captain-shot-lab PACK --interactive`. This is a resettable development scene with explicit random/route inputs; it is not a playable match.

An [optional SDL2 native window](docs/NATIVE_SHOT_SCENE.md) displays that same scene with placeholder graphics and keyboard controls. Enable it with CMake's `CAPTAIN_BUILD_SDL_SCENE` option; the default core build does not require SDL2.

The [native field-phase laboratory](docs/NATIVE_FIELD_PHASE.md) adds persistent position/roster state, formation-based targets, player advancement and prepared terminal flight tails. It requires private version4 or newer content and a bounded `.cmjstate` checkpoint. Full match sequencing and live frame/random scheduling remain incomplete.

The [native phase session](docs/NATIVE_CONTACT_CLOCK.md) extends that checkpoint with contact status/time, keeper recovery and match-clock events. Version5 private content and `.cmjsession` state support possession/recovery handoffs, explicit added-time pauses and period-end boundaries.

The [terminal reward integration](docs/NATIVE_CONTACT_REWARDS.md) adds persistent named-player experience and an explicit rewarded contact operation. That checkpoint introduced content version6 and session version2; older supported formats remain readable.

The [persistent recovery session](docs/NATIVE_RECOVERY_SESSION.md) continues terminal loose balls through player advancement and recipient selection, preserving clock/rewards and coordinate fractions. Session version3 adds optional recovery context; unsupported restart/keeper/story routes pause explicitly. Prepared continuation can reach player control, while live input and presentation timing remain unfinished.

The [player-control setup](docs/NATIVE_CONTROL_SETUP.md) now prepares either possession side, refreshes the carrier/ball cell and selects the controlled defender for opposing possession. Current session exports use version4 with optional control context. Setup stops before the live input loop.

```bash
./build/captain-prototype --replica
```

Enter one command per line:

```text
state
contact 7 0
state
contact 7 1
state
quit
```

This advances a developer-defined contact-loss fixture from power 300 to 263 to 226. Participant 7 controls team 0; participant 42 belongs to the other team. These are sample values, not a complete shooting system. Sequence numbers start at zero and duplicates are rejected.

## Compare with the original ROM

Install the optional [analysis dependencies](tools/README.md), then provide your own verified Japanese ROM:

```bash
.venv/bin/python tools/verify_action_strength.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_defender_contact.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_shot_power_keeper.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_shot_state.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_shot_outcome.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_defender_outcome.py /path/to/japanese-game.nes
.venv/bin/python tools/verify_ball_recovery.py /path/to/japanese-game.nes
```

ROMs, patches, extracted game assets, and generated script data stay outside Git. The public engine sources are independently written; the discovered disassembly is a research reference.

The defender comparison covers strength variation, weakening, special branches, classification, and conditional power loss. It reads the original tables from your ROM during verification. The native `--defender-batch` research mode accepts explicit parameters as documented in [DEFENDER_CONTACT.md](docs/DEFENDER_CONTACT.md).

The skill/action/keeper comparison covers level/move index adjustment, randomized action strength, and the keeper's score/classification before story overrides. Research command formats and remaining limits are in [SHOT_POWER_AND_KEEPER.md](docs/SHOT_POWER_AND_KEEPER.md).

The state comparison harness passed 25,756 cases covering stamina deduction, keeper story overrides, keeper timer/penalty updates, and one original random-generator step. The outcome harness checks 384 immediate goalkeeper results, 144 field-defender possession handoffs, and 512 ordinary loose-ball recovery cases. Research interfaces are documented in [ENERGY_USAGE.md](docs/ENERGY_USAGE.md), [KEEPER_STATE.md](docs/KEEPER_STATE.md), [RANDOMNESS_NOTES.md](docs/RANDOMNESS_NOTES.md), [SHOT_OUTCOME.md](docs/SHOT_OUTCOME.md), and [BALL_RECOVERY.md](docs/BALL_RECOVERY.md). A complete playable shot and the original random timing still need integration.

The nearby-defender selector at `$E4D7` matches 1,024 original-ROM cases. It returns nearby opposing player IDs in original order, including the ROM's team-specific limit and status check. See [DEFENDER_SELECTION.md](docs/DEFENDER_SELECTION.md); player attributes and contact tables are still supplied separately.

Standard field-defender preparation matches 16,064 executions of the complete original loader. It selects the skill and move, loads strength/effect/stamina cost from replaceable content, and can supply that strength to the contact calculation. Ordinary, generic opposing-team, and alternate field player profiles now feed that preparation, with 91,392 matching constructed ROM fixtures. The [prepared defender sequence](docs/PREPARED_DEFENDER_SEQUENCE.md) now connects this lookup to energy use and ordered contacts, returning an updated roster for the match caller. The [prepared field-shot handoff](docs/PREPARED_FIELD_SHOT.md) joins these contacts to field possession/recovery and eligible goalkeeper goal/save outcomes. Attacker preparation now connects move selection, stamina and shot power; see [PREPARED_ATTACK.md](docs/PREPARED_ATTACK.md). Keeper profile/move preparation now connects keeper skills, stamina and comparison through [PREPARED_KEEPER.md](docs/PREPARED_KEEPER.md). The [joined prepared shot calculation](docs/COMPLETE_PREPARED_SHOT.md) now applies attacker, defender and keeper stages to one returned roster, with goal/save or field handoff results. Runtime content import and full match integration remain in progress; see [FIELD_PLAYER_PROFILES.md](docs/FIELD_PLAYER_PROFILES.md) and [FIELD_DEFENDER_PREPARATION.md](docs/FIELD_DEFENDER_PREPARATION.md).

## Project reference

- [Working roadmap](docs/ROADMAP.md)
- [Engine direction](docs/ENGINE_DIRECTION.md)
- [Current engine progress and limitations](docs/ENGINE_PROGRESS.md)
- [ROM findings](docs/ROM_ANALYSIS.md)
- [Text and dialogue formats](docs/TEXT_FORMAT.md)

# Engine direction

**Date:** 2026-10-26

**Status:** Design direction agreed; the first native kernel and local replica fixture are implemented. See [ENGINE_PROGRESS.md](ENGINE_PROGRESS.md) for verified scope and limitations.

## Goal

Build a native Windows and Linux reconstruction of Captain Tsubasa II, using the verified Japanese ROM as the behavior reference. Arabic, English, and Japanese content, replacement art, replacement music, shaders, and gameplay additions must fit the foundation. Online play is a core requirement and must be exercised early.

## Source strategy

Write our implementation independently. Use the discovered English-translation disassembly as a research reference and verify its technical findings against our ROM. Do not import its code, translation, comments, graphics, or audio. Record the origin of findings in our research notes. This strategy does not establish permission to distribute the original game's content; distribution decisions remain separate from the engine design.

## Engine boundaries

- **Simulation:** Match rules, player statistics, action outcomes, match clock, and gameplay random state. Given the same starting state and ordered commands, it must produce the same results on Windows and Linux.
- **Presentation:** Graphics, dialogue display, animation, audio, shaders, and input devices. Presentation translates input into commands and displays simulation results.
- **Content:** Language packs, player/team definitions, scripts, images, and sound mappings. Stable asset identifiers allow replacement packs to supply different resolutions, music, and effects.
- **Session:** Participants, teams, control permissions, ordered commands, connection state, and simulation compatibility. Participant identity must be separate from team identity and footballer identity.
- **Online services:** Private room discovery/invitations, public matchmaking, and connection assistance. A private invite and a random match both create a session through this boundary.

Simulation time and gameplay randomness must not depend on rendering speed, network arrival time, animation length, or a graphics pack. Save/replay data must preserve the simulation state, random state, command order, and rules/content version.

## Online scope

The first proof uses two clients. It is a development checkpoint rather than a permanent two-participant limit.

The architecture must allow a list of participants, private invitations, and matching with strangers. The exact participant limit and how several people share control remain undefined. Possible shared-team modes require their own rules for who can submit each decision; the engine must validate that permission before accepting a command.

The first implemented control arrangement will be one participant per team. Do not promise shared-team play, spectators, ranked play, or a specific total player count until their rules and performance have been evaluated.

Private rooms and matchmaking should use the same match/session format. Public matchmaking will need a hosted coordination service. Connection assistance may also require relay hosting; the choice between relayed peer connections and authoritative game servers remains open until a network proof provides evidence.

## Community additions

- Replacement images may exceed NES resolution while preserving the logical placement and timing of game elements.
- Music and sound packs replace assets by identifier without changing match results.
- Language and cosmetic packs may differ between participants if they do not change simulation data.
- Gameplay mods require the same agreed rules and simulation data for everyone in a match. Check their versions before a match begins.
- Matchmaking must group compatible game versions and rules; it must not silently match incompatible gameplay mods.
- Downloading or executing community code is outside the first prototype's scope.

## Next checkpoints

1. Document one original shot interaction: starting state, valid decisions, random inputs, outcome, and presentation events. Use emulator observations to confirm the behavior.
2. Implement that interaction as a small native simulation with a minimal display. Choose the language and platform libraries in its implementation design, with Windows and Linux as requirements.
3. Replay the same commands with the same random state and compare the complete resulting game state, including across platforms when runners are available.
4. Connect two clients and reproduce the interaction. Exercise delayed commands, disconnects, and incompatible rules before expanding gameplay.
5. Expand to one complete match, with early private online play developed alongside the offline version.
6. Grow the campaign, localization, replacement-pack interfaces, shaders, and online services in reviewable increments.

## First proof acceptance

A player can make a decision in one authentic match interaction and see its result. The result matches documented reference behavior. Replaying the same starting state, random state, and commands produces identical simulation state. Two connected clients agree on that state. Presentation choices do not alter the result.

This proof establishes a foundation. It does not demonstrate a complete playable match or settle the multiplayer control design.

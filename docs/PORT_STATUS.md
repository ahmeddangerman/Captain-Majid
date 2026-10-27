# Port status and completion checkpoints

**Overall completion is not currently measured.** The previously repeated7% was an unsupported planning guess and is retired. The checkpoints below describe observable capabilities; they have different sizes and must not be averaged into a game-completion percentage.

## Current capabilities

| Area | Current state | Completion checkpoint |
| --- | --- | --- |
| Original behavior research | Private CPU observations and many historical component comparisons; full frame/audio behavior remains outside the harness. | Supported gameplay phases have documented original inputs, outputs and timing boundaries. |
| Native shot calculation | Development lab joins preparation, initial contacts, one later burst, keeper comparison and immediate outcomes in both directions. | Gameplay supplies the inputs from a coherent match rather than a resettable constructed scene. |
| Movement | Native target-direction, speed profiles, formation/tactic targets and ordered field advancement implemented for staged interrupt-free inputs. Live frame/random scheduling and ball-flight progression remain pending. | Player and ball state advance coherently through action and possession transitions. |
| Persistent match state | Persistent field and phase-session labs include contact status/time, keeper recovery, clock events and supported terminal tails. Added-time/period-end pauses are explicit. No canonical full-match state yet. | Atomic roster/ball/clock/score/phase commits, save/replay snapshots and supported next actions. |
| Playable match | Not implemented. | Two teams can start, finish and restart a complete match with supported rules and controls. |
| Campaign and team AI | Not integrated. | Original team setup, progression, AI decisions and match scheduling work in native play. |
| Game presentation and languages | Private content analysis and placeholder native window; no complete runtime graphics/music/text/cutscene system. | Arabic, Japanese and English game flows render and play correctly. |
| Windows/Linux application | Linux production builds; optional window had historical software capture. Windows execution remains unverified. | Installable builds, input, saves/settings and desktop behavior work on both targets. |
| Mods and visual/audio upgrades | Replaceable development data pack exists. User-facing asset/music replacement and shader systems pending. | Community packs can replace supported content and shaders without changing simulation or desynchronizing online play. |
| Online friend match | Ordered fixture and loopback proof exist; no playable online match. | Two peers complete an actual match with identity, invitations, synchronization and recovery. |
| Matchmaking/additional participants | Planned. | Supported rules, sessions, participant roles and random matchmaking operate with the real match simulation. |
| Release readiness | Not reached. | End-to-end compatibility, packaging and recovery evidence supports a public playable release. |

## Current engineering sequence

1. Derive rewards and recovery/player-control progression around the connected contact-clock/status phase.
2. Extend field snapshots into coherent match phases with clock/score, recovery and player actions.
3. Integrate live ball-flight and frame/random scheduling with atomic state commits.
4. Complete playable match, original presentation and desktop integration.
5. Build online match synchronization on that simulation, followed by invitations/matchmaking and broader participant rules.

Online constraints continue to guide state ownership and deterministic scheduling during the earlier steps. No server or networking success is inferred from isolated arithmetic or the loopback fixture.

See [WORK_LOG.md](WORK_LOG.md) for commits, evidence, corrections and resume notes. Production builds, original observations, static reviews and automated test results are recorded separately. A build does not establish original gameplay equivalence.

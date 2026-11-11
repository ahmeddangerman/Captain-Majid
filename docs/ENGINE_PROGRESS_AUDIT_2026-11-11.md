# Engine progress audit — 2026-11-11

## Verdict

Real reusable numerical work exists, but recent integration has disproportionately advanced a captured scenario rather than a general match runtime. The recent 20 commits inspected mostly concern one opponent pass and its surrounding control/waits. These are distinct changes, not repeated identical implementations; they are also not 20 independent playable features. Overall progress is unmeasured. A complete graphical match remains unverified.

Scope: local work branch, current source and recent history. This does not measure another agent's background runtime or compute spending.

## Evidence

| Area | Inspected evidence | Finding |
|---|---|---|
| Gameplay kernels | match_clock.hpp, contact_prelude.cpp, ball flight, attack/defense, keeper and recovery modules | Useful reusable foundations; presence does not prove complete routing. |
| AI | ai_action_candidate.hpp | Candidate, area and recipient helpers exist; header explicitly excludes branches and generic team resolution. Full autonomous match AI is not established. |
| Integration | phase_session.hpp/cpp, control_input.cpp, ball_flight_session.cpp | Exact player, score, position and random guards recur. Phase identifiers reach 466; that is not a feature count. |
| Runtime | main.cpp, phase_session_lab.cpp | Research protocol and manually invoked transitions, not a demonstrated end-to-end match runner. |
| Graphics | CMakeLists.txt, shot_scene_view.cpp | Optional SDL shot viewer, disabled by default, not a complete match client. |
| Timing | control_tick.cpp and private replay verifiers | Captured random memory, timing and presentation samples reproduce observations; they do not constitute an autonomous scheduler. |
| Tests | registered tests and WORK_LOG.md | Last code checkpoint reports 62 passing checks. Repeating them is regression prevention, not new feature delivery. Final-whistle acceptance remains missing. |
| Content | content loaders/private packs | Local research content exists; a distributable clean playable asset package is not established. |

Integration file sizes at audit: phase_session.cpp 1,467 lines; phase_session_lab.cpp 2,859; control_tick.cpp 1,110; control_input.cpp 1,282. Avoid a wholesale rewrite merely to reduce these counts.

## What to keep and what to stop

Keep kernels, original evidence, byte arithmetic/order, atomic errors and legacy snapshot readers. Pass flight, defender outcomes and possession are essential capabilities. Exact presentation waits and more instances of an already understood mechanic are lower priority unless they change an outcome needed by the playable milestone.

The primary mistake was allowing the next trace boundary to dictate the next task. This accumulated scenario-specific integration without establishing the general runtime. Very small commits, repeated full-suite runs and repetitive status entries added overhead. No reliable time accounting was audited, so no percentage of wasted time is claimed.

## Playable milestone specification

Build a local human-versus-computer exhibition match: kickoff, movement, action selection, passing, defending, shooting, goals/restarts, clock and final whistle. First deliver a reusable possession cycle, then an interactive cycle, then the complete match. Use original placeholder graphics. Local private packs may be selected explicitly; never bundle or commit derived data.

Acceptance requires normal user input and computed opponent decisions. No per-action CLI operation, captured outcome injection, snapshot editing or exact score/player/random checkpoint may be needed during play. A shortened-clock fixture is useful for testing but is not evidence of a faithful full-length match.

Classic-compatible rules remain the target. Compare each new gameplay behavior with controlled original observations. Missing rules produce a named unsupported capability and a targeted investigation; they must not silently become invented rules. Visual presentation can be simplified, but acknowledging it must not silently supply gameplay outcomes or randomness. Simplified AI/timing requires an explicit product decision, not an invisible workaround.

Preserve historical replay paths. Keep engine work on work, private data outside Git, and leave Custom Tools from Ahmed and build-demo untouched. Online play, expanded languages, shaders and replaceable media remain long-term requirements; they are not first-local-match prerequisites.

Report playable capabilities, evidence and blockers instead of phase/commit counts or percentages. Suggested model: GPT-6.1 Terra High for bounded implementation tasks.

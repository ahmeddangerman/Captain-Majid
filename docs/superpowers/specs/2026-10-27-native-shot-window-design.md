# Optional native shot window

## Goal

Display and operate the existing resettable development scene in a native window, sharing its calculation with the command-line laboratory. Keep rendering/animation independent of simulation.

## Approach

Extract the explicit scene constructor/calculation into a small shared core API. Add an optional SDL2 executable with placeholder field/player shapes, original simple bitmap lettering and keyboard controls. Run a complete scene calculation only when the user shoots; use time only for drawing the ball's presentation path. Default builds keep SDL2 optional. Support saving a BMP development view through a software renderer without a display server.

The user authorized autonomous implementation and had already agreed to a minimal native display milestone. This is an implementation choice for the development scene, not a new public webpage or final game interface.

## Constraints

- C++17 and SDL2; no downloaded source/art/fonts or original game assets committed.
- Shared scene keeps all documented fixture assumptions and per-shot reset; no match/AI/netcode claims.
- Space fires; Up/Down adjust attacker level, Left/Right keeper level, D cycles defender count, R clears, Escape exits. Changing settings clears the prior result/animation.
- Logical640×480 canvas, resizable/letterboxed, placeholder graphics and basic pixel lettering. Presentation timing never advances the native calculation or random state.
- Optional CMake flag `CAPTAIN_BUILD_SDL_SCENE=OFF` by default. Core/CLI builds work without SDL2.
- Capture option writes only a new `.bmp` output, never an existing file. It renders once then exits; normal window remains interactive.
- SDK is built locally under `/tmp` for this headless environment; no system install or source vendoring. No automated tests added/run; production build and normal capture usage provide limited evidence.

## Next boundary

Real asset rendering, animation scripts, Arabic UI, shaders, audio, full matches and online session integration remain separate work. No claim that placeholder positions/animation reproduce original presentation.

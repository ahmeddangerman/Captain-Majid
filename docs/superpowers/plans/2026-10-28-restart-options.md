# Prepared restart options implementation plan

> Use superpowers:executing-plans. The user authorized continuous implementation.

Goal: connect selected field/goal restarts to the original shared options input
loop at9880..98D5, preserving saves and stopping at an unimplemented tactical UI.
Architecture: a dedicated restart context stores the originating route and0624
cursor. Phase40 is polling,41 stops before tactical UI98D5,42 returns to caller
9381/94D7. Session16 marker G appends three bytes (presence, route, cursor),339total.
C++17 and the current Make/CMake definitions remain the toolchain.

## Spec and constraints

Input38/39 is explicitly prepared after opening scripts43/31 and55 have been
acknowledged externally. Do not emulate those scripts or infer their side effects.
Open clears ball_mode062D, cursor0624=4. Presentation requests are clear overlay,
display0/1/51 and sprite4. Frozen polling toggles cursor bit64 once if PRESSED&12,
then tests HELD&128 (not PRESSED). Confirm on cursor4 returns to caller; cursor68
writes cursor0 and stops before tactical UI. Requests after confirmation include
clear overlay and display0/1; a toggle emits its sprite first. Waits and sprite
renderer calls are acknowledged, with no live frame/RNG simulation. No movement,
time, reward, energy or owner writes. Both routes retain their identity.

## Approaches

Chosen by implementer under continuous authorization: small saved native state
machine corresponding to observed original boundaries. A monolithic restart
simulation would require guessing the tactical UI, restart action and scripts.
Pausing without saving options would lose the selection and route on resume.

## Task1: complete saved options loop

- [x] Trace original9880 and98AC with explicit disclosed display/wait acknowledgements.
- [x] Add failing real C++ regression for open, held/pressed priority, both routes,
  save/resume, legacyF reading, invalid/replay/context inputs and untouched source.
  Run against throwing stub; expected failure is unimplemented behavior.
- [x] Implement restart_options.hpp/.cpp, session enum/context/validation/I/O,
  CLI operations and Make/CMake targets. Read versions1..15; new phases requireG.
- [x] Compare bounded original/native observations including independently stored
  cursor0624; exclude native route metadata from original claims.
- [x] Run focused tests, full make check and production make all; expected exit0.
- [x] Fresh final review, document exact scope and evidence, commit locally.

Review focus: numeric phase range coupling; corrupted unused context bytes;
old snapshot reads/new writer compatibility; one-shot open/confirm/replay;
held versus pressed behavior and simultaneous toggle/confirmation.

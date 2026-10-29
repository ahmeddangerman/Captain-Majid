# Saved numeric ball flight plan

Connect phase46 through prepared DD08/E73E setup to E773, then explicit numeric
E778 step to cell/arrival callback. Caller supplies captured low X/Y bytes; never
assume zero. Native direction geometry and private ball projection content are
explicit. Clear selected count/prefix and elapsed at setup, retain destination,
set recovery062C direction, reset flight061B flags0. Save full16bit position,
velocity, direction and flags. Already-at-destination skips velocity init and
returns without arrival callback or snapping coordinates. Live frame wait and cell/
arrival callbacks remain explicit pending work, never silently acknowledged.

New47 ready,48 cell callback,49 arrival callback,50 already-arrived return. Transient selected progress is
cleared after count is cleared. Shot context persists46..50; trajectory47..50.
Snapshot20K appends11bytes toJ344=355total, retaining readers1..19. Guards reject
stale/missing contexts and replay. Private CMJBALL1 text file carries64uint16words.

- [x] Test real setup/save/step/replay/arrival/fractions and content parsing RED.
- [x] Implement state validation/I/O, CLI and Make/CMake integration.
- [x] Compare original setup and first-cell step, both selected/unopposed/arrival.
- [x] Implement/verify original87E1 cell callback accumulation and resume47.
- [x] Full suite/build and prior ROM regressions; fresh review/log/commit.

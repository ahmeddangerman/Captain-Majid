# Development log

This is the running reference for work between chat updates. Entries distinguish implemented code, observed original behavior, and remaining gaps. Source commits stay local unless publication is requested; ROMs and derived content remain private.

## Current position

- **Full port: roughly 7%.** This is a subjective planning estimate, not a measured percentage of recovered code. There is no playable match yet.
- **Available:** native shot calculations, private content loading, a resettable shot laboratory, and an optional SDL2 scene with placeholder graphics.
- **Current milestone:** replace fixed shot route inputs, then implement elapsed goalkeeper recovery.
- **Next major milestone:** coherent persistent match state and supported phase transitions, with explicit simulation randomness for future online play.
- **Suggested model:** Sol 6.1 High for original-code tracing and C++ integration. This is a judgment about the task, not a benchmark result.

## 2026-10-27 — completed local checkpoints

| Commit | Change | Evidence and limits |
| --- | --- | --- |
| `e6a6f6d` | Corrected prepared defender contacts to preserve power in the selected-defender caller. | Seven original loop observations; production compilation. Automated native regressions were not run for this increment. |
| `a1cbbf0` | Added continuous original attacker-to-keeper tracing. | Four side0 observations reach the keeper comparison boundary. Host frame/display acknowledgements are explicit; full outcome scripts and side11 remain unresolved. |
| `9a7c423` | Added private data pack loading and native shot laboratory. | Linux production builds and normal CLI usage. Data stays outside Git; scene resets between shots. |
| `004434a` | Added optional SDL2 shot scene using the same calculation. | Linux production compilation and software image capture. Real window/input, Windows, artwork, audio and a full match remain unverified or unfinished. |

## 2026-10-27 20:25 UTC — resumed work

- Resumed the uncommitted shot approach increment: original distance adjustment, destination cell, final target position, and loose-ball routing.
- Eight isolated original observations and four continuous shot observations are recorded privately. These are research observations, not a native differential suite.
- Version2 private packs add geometry tables; the reader retains version1 support.
- The independent reviewer stopped at a model usage limit. Its partial review reported no finding, but **the review did not finish**. Author review and production builds are being recorded separately.
- The temporary build tools/SDK from the prior runtime are absent after resumption. The repository and private analysis files remain available; rebuilding the default native targets needs only the installed C++ compiler and Make.
- Next: finish the approach checkpoint, record its commit, and inspect the original goalkeeper recovery caller before connecting elapsed time.

### Shot approach checkpoint

- `make -j2 all` completed successfully after resumption, with no compiler warnings in the captured output.
- Normal CLI usage loaded both pack versions. Version2 produced a distance45 goal aimed at(204,124), and a distance90 loose-ball diversion aimed at(196,124). Version1 retained its explicit scene behavior.
- Author review covered route gates, subtype handling, separate random stages and pack layout. Independent review remains incomplete due to its usage limit. No automated tests were added or run.
- Keeper recovery research is next; this does not change the rough full-port estimate yet.

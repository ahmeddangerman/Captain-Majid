# Captain Majid working instructions

Read docs/CLI_HANDOFF.md, docs/WORK_LOG.md (current position and latest entries),
and docs/PORT_STATUS.md before continuing. This is an early native reconstruction
of Captain Tsubasa II, not a complete playable PC port.

## User priorities

- Faithful classic gameplay, alongside optional enhancements; expandable fresh
  Arabic/English/Japanese text, correct Arabic right-to-left layout, quality of
  life, replaceable graphics/music and shaders. Online invitations, random matches
  and potentially additional participants are central long-term goals.
- Continue routine authorized engineering without repeated approval requests.
  Verify meaningful changes and keep an honest dated work log.
- After each update report overall progress, next steps and suggested model.
  Overall percentage is unmeasured; the old7% estimate was unsupported and retired.
- Explain findings plainly. Prefer a complete playable match milestone over
  interpreting component counts as percentage progress.

## Source and private data

- Engine work is on local branch work. Keep it private. Public GitHub main at
  ahmeddangerman/Captain-Majid hosts the promotional website. Do not push engine
  work there or change repository visibility without explicit authorization.
- ROMs, patches, derived packs, RAM, observations and session files belong in
  ../Captain-Majid-analysis, outside this Git checkout. Never commit them.
- The external English translation/disassembly repo was inspected as background;
  do not import its code. Its reuse permission was not obtained here.
- Recreate compiler/Python environments locally; do not transfer cloud credentials.

## Website

The user authorized direct publication of daily Arabic progress notes. Use a
separate website checkout on public main. Edit only progress notes for routine
updates. Roadmap copy, NES menu design and geometry were explicitly approved;
do not rewrite them without the user's request. Date navigation is manual, and
the bottom box must not move automatically. Verify publication separately from
engine work; do not claim a browser render from source-only verification.

## Build and evidence

- Linux C++17 + Make are verified. `make -j2 all` builds development programs;
  `make -j2 check` runs the core/CLI/adapter/loopback checks without ROMs.
- CMake and optional SDL2 definitions exist; current cloud increments used Make.
  Native Windows builds and a complete graphical match remain unverified.
- Research tools use tools/requirements.txt in a private Python environment.
  Trace private original behavior before inventing missing gameplay rules.
- Compare every new gameplay behavior with controlled original ROM observations;
  record matches, differences and partial coverage before building further on it.
- Preserve atomic errors, original byte arithmetic/order and snapshot readers.
  Keep live frame/random/audio/presentation limitations explicit. Prepared
  acknowledgements are not evidence of running the original presentation engine.

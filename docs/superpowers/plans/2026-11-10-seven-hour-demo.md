# Seven-Hour Playable Demo Plan

Superseded by [the revised eight-hour plan](2026-11-10-eight-hour-demo.md).

**Goal:** Identify and implement the shortest original-backed path from the current captured replay to a small interactive match demo, with an honest checkpoint at the end of seven hours.

**Starting point:** Engine branch `work`, commit `b6984e8`; captured original and native states match through the second post-return field round at $E145, clock 154, player 18 in possession, random index 144. Original timing, presentation samples and AI choices are supplied. The full match and native Windows graphical match are unverified.

**Approach:** First audit the existing executable and graphical frontend for what actually runs. Continue the controlled original comparison where it unlocks a live interaction; connect existing verified kernels into a narrow continuous loop. If live integration proves larger than the available time, deliver a clearly identified limited replay or interaction milestone and a precise blocker rather than label it a playable match.

**Constraints:** Follow `AGENTS.md`, `docs/CLI_HANDOFF.md`, `docs/PORT_STATUS.md` and `docs/WORK_LOG.md`. Never commit private ROMs, observations, traces or derived packs. Do not copy external disassembly code. Preserve atomic errors, byte arithmetic and snapshot readers. Use a separate website checkout for authorized progress notes, and push engine work only to `work`. Overall progress is unmeasured.

## Hour 1 — Establish the demo baseline

- [ ] Review the current prototype entry points, graphical/SDL support, input and match loop; run the existing executable where practical and record what is visible and interactive.
- [ ] Inspect the private original trace following the current $E145 boundary through the next decision. Identify the first missing transition and its evidence.
- [ ] Define the smallest honest demo scenario using existing assets and controls; identify what can be original-backed within this timebox.

## Hours 2–3 — Extend the verified match path

- [ ] Replay and compare the next control cycle and opponent decision from the saved native checkpoint. Preserve the ordering of random, movement, clock and presentation samples.
- [ ] For each new behavior, capture controlled original observations; reuse existing kernels where evidence supports them, and implement the smallest guarded transition with meaningful rejection and snapshot checks.
- [ ] Record matching fields, differences and unobserved paths in the work log. If the next branch cannot be verified, stop extending that branch and work on the demo loop using already-supported behavior.

## Hours 4–5 — Connect a narrow interactive loop

- [ ] Connect the existing frontend, input handling, match state and verified actions for the smallest feasible scenario. Prefer a short start-to-play sequence with a visible field, player input and one verified action/outcome if the current frontend supports it.
- [ ] Keep live timing, rendering, audio, random behavior and AI limitations explicit. Any temporary scripted input or presentation acknowledgement must be labeled as such.
- [ ] Exercise normal flow, an unsupported choice and a reload/continuation boundary; reject unsupported actions cleanly and retain the saved state on failure.

## Hour 6 — Verify and repair

- [ ] Build with the available Windows CMake toolchain and run all registered checks. If Linux Make is available, run `make -j2 all` and `make -j2 check`; otherwise report it as not run.
- [ ] Run the actual demo path end to end where possible, including input, state changes and output. Confirm any graphical behavior in a real window; do not claim a visual result from source inspection.
- [ ] Fix failures revealed by verification and compare any changed gameplay behavior against the original evidence. Preserve older snapshot readers and check invalid or incomplete inputs.

## Hour 7 — Package the checkpoint

- [ ] Update `docs/WORK_LOG.md`, `docs/CLI_HANDOFF.md` and `docs/PORT_STATUS.md` with dated evidence, exact commands, what works and what remains unsupported.
- [ ] Run final targeted checks, inspect the diff and Git object health, commit only authorized public source/docs, and push `work`. Keep private evidence in `../Captain-Majid-analysis`.
- [ ] Publish the authorized Arabic progress note on the separate website checkout if warranted; verify the live response and distinguish source checks from an actual browser render.
- [ ] Report whether the result is an interactive demo, a limited scripted demonstration, or an engineering checkpoint; list the next blockers and recommend GPT-6.1 Terra High.

**Timebox rule:** The order is a priority guide, not a promise that each boundary takes exactly an hour. Reserve the last hour for verification and handoff. Do not claim a complete match or estimate a completion percentage without evidence.

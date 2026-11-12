# Six-hour contested-pass integration plan

> For agentic workers: use superpowers:executing-plans to execute this plan sequentially. Preserve the existing uncommitted work. Do not restart implementation from scratch.

Goal: one supported ordinary human pass with exactly one opponent defender progresses in the native development window through original-compared contact to explicit possession and a subsequent control step, or produces a precise reproducible first-divergence report.

Architecture: reuse existing session/contact/flight/recovery kernels, and connect them with guarded runtime transitions. Keep original timing evidence distinct from provisional runtime timing. Do not hard-code the captured winner or remove guards to make the demonstration continue.

Tech stack: native C++17, Windows CMake build, private Python original-CPU comparison tools, existing development window.

Spec: AGENTS.md, docs/CLI_HANDOFF.md, docs/WORK_LOG.md and docs/PORT_STATUS.md. This plan is a bounded integration investigation, not a promise of a complete match in six hours.

## Why this is the best next milestone

Recent work has established useful isolated comparisons, but repeated checkpoints have stopped before a playable vertical slice. The latest final-choice survey matched 2,224 supported constructed cases, with 1,264 unsupported and 96 original out-of-scope cases. It does not prove live contact or possession. The old player2/defender21 replay reaches opponent20 control; it is a different scenario from the live-window defender17 blocker. Never substitute that recorded outcome for the live scenario.

Two concrete gaps are already known: the appended special-option data breaks older U packs under the same signature, and the ordinary-pass presentation handler rejects selected defenders. Spend this block fixing these dependencies and proving one end-to-end path. Defer broad AI, graphics, networking, fresh startup implementation and full-match promises.

## Global constraints

- Engine branch is work; public main is the website. Preserve unrelated files, including Custom Tools from Ahmed and build-demo.
- Keep ROMs, derived packs, RAM, trace data and saved sessions outside Git in ../Captain-Majid-analysis. Never import external disassembly code.
- Original byte arithmetic/order, snapshot compatibility and atomic errors are mandatory.
- Unsupported outcomes stay explicit; no guessed special moves, forced result0, fixed winner or presentation acknowledgement described as original execution.
- Runtime parity and recorded-event replay parity are separate claims. A GUI demonstration with demo random timing is not proof of original live timing.
- Budget six hours of active work. Timeboxes guide effort; do not wait to fill them. Continue authorized engineering without routine permission requests. Reserve the final 30 minutes for verification and handoff.

## Review focus

Old versus extended U packs; special/conditional defender rules; malformed or repeated transitions; save/reload with pending contact; original-versus-provisional random and presentation order. Test each where its implementation is changed.

## 0:00-0:30 — Recover a trustworthy baseline

Files: the four instruction/status documents, current Git diff, CMakeLists.txt, existing build configuration.

- [ ] Read current handoff/status and inspect the entire outstanding diff. Record branch and changed files; preserve rather than reset them.
- [ ] Inspect the unexpectedly broad WORK_LOG.md diff for accidental encoding/line-ending changes from recent PowerShell rewrites. Repair only demonstrably accidental changes while preserving new entries, using explicit UTF-8.
- [ ] Locate the real Windows compiler/CMake/test runner. The cached ctest path was stale; 52 executables passing was not the full 65-test registered suite. Do not repeat guessed paths indefinitely.
- [ ] Build current sources. Enumerate registered tests and run them using a working runner; if unavailable, document exact coverage of a fallback. Separate pre-existing failures from new failures.
- [ ] Create a private evidence directory and a concise manifest of commands, source revision/diff identity, pack hashes and scenario provenance.

Gate: reproducible baseline and preserved work. Do not spend more than 30 minutes on incidental toolchain cleanup; report limitations and use a viable focused build path.

## 0:30-1:15 — Fix content-format compatibility

Files: engine/src/shot_content.cpp, tools/export_shot_content.py, tools/check_pass_defense_pack.py, existing content-loading tests and engine/tests/ai_action_candidate_tests.cpp.

- [ ] Reproduce old-U rejection with the pre-extension private U pack; retain it unchanged as a compatibility fixture outside Git.
- [ ] Write a failing synthetic regression: original U loads candidate tables but has no final-choice special-rule knowledge; final selection requiring that missing knowledge rejects atomically.
- [ ] Prefer a distinct new format signature for the extended payload. Check the repository for the next available version before choosing it. Preserve old U and T readers; avoid accepting arbitrary truncation as an older format. Account explicitly for experimental extended-U files already generated locally rather than silently reinterpreting them.
- [ ] Test new-format roundtrip/loading, old-format loading, truncated and trailing data, invalid rule/row flags and missing special-rule content. Observe failure before implementation, then pass.
- [ ] Regenerate private content and rerun check_pass_defense_pack.py and check_ai_pass_defense_final_original.py, updating version expectations as needed. Preserve supported/unsupported/out-of-scope counts separately. Review the survey's broad exception skipping before treating it as evidence.

Gate: no pack ambiguity or regression; rules match original evidence. Commit only the reviewed source/test changes on work, without private assets.

## 1:15-2:30 — Trace the actual live scenario through first contact

Files: tools/check_ai_pass_defense_final_original.py, existing original trace tools, tools/verify_captured_pass_defenses.py, engine/src/action_menu.cpp, engine/src/match_runtime.cpp.

Private starting evidence: pass-defense-runtime-input-20261112/{entry,target,confirmed}.cmjsession; locate the corresponding original capture and provenance. The older player2 trace is a reusable reference, not the same scenario.

- [ ] Reproduce the runtime stop with exactly one selected defender and ordinary action. Record carrier, defender ID/type, target/recipient, ball coordinates/fractions, stamina, power and random inputs.
- [ ] Trace the corresponding original from the earliest defensible boundary through defender choice, attacker preparation, presentation effects, defender preparation, comparison and contact. Label constructed interventions explicitly.
- [ ] Derive comparison tables and selection from the actual original caller. Never tune RANGE/OFFSET/thresholds until a replay happens to pass; the previous contact replay tried unrelated parameter sets before finding a match, so matching alone is insufficient provenance.
- [ ] Capture outcome, possession writes and the next control boundary if reachable. Record the first missing input or divergent field, rather than silently normalizing it away.
- [ ] Produce a boundary table: original address/event, native phase, compared fields, explicitly supplied inputs, expected owner.

Gate: an evidence-backed contract for the one-defender route. If it cannot be established within this block, use the remaining engineering time to resolve this specific trace gap; do not invent a runtime outcome.

## 2:30-4:15 — Integrate the supported contact and possession path

Files: engine/src/match_runtime.cpp, engine/tests/match_runtime_tests.cpp; only if necessary, engine/include/captain/match_runtime.hpp and its existing save/load implementation. Reuse existing session kernels in engine/src/action_menu.cpp and existing flight/recovery helpers.

- [ ] Write a failing runtime regression from the selected evidence: confirmed ordinary pass with one defender reaches the first missing boundary, preserving unrelated state.
- [ ] Implement the smallest guarded transition. The current awaiting_presentation branch explicitly rejects selected defenders; do not simply delete the guard or route contested passes through the uncontested branch.
- [ ] Extend sequentially through evidence-backed preparation, comparison, contact clock/dispatch, pass continuation/recovery and ownership. Feed each native output into the next step; do not splice in later captured native states.
- [ ] Compare each new gameplay boundary against the original before implementing the next one. Allow only named, justified representation differences.
- [ ] Test missing content, unsupported special/result branches, wrong/repeated input and invalid defender IDs for atomic rejection. Save/reload at every new pending boundary and verify continuation equality.
- [ ] Confirm ball owner, controlling side, location and subsequent control-ready/input behavior. A changed owner field alone is insufficient.

Gate: one supported integrated route, not a generalized unverified contact system. If a different outcome is encountered, preserve the stop and trace it rather than force the old opponent20 result.

## 4:15-5:15 — Live-window acceptance and focused repair

Read the computer-use skill before interacting with the window. Use the actual built binary and current compatible private content.

- [ ] Launch the development window from the documented scenario. Record that a developer snapshot is used; this is not fresh match startup.
- [ ] Perform ordinary pass input with exactly one defender. Observe contact and possession. Capture screenshots or UI evidence and retain the detailed trace privately.
- [ ] Show who controls the ball, then demonstrate one valid subsequent control step. Compare to the corresponding original evidence; call out provisional timing/random differences explicitly.
- [ ] Repeat once from a saved boundary to check determinism/reload. Run focused regression tests after any repair.
- [ ] If the live test stops, capture the exact phase/capability and first mismatch. Do not substitute the historical recorded replay or source inspection as a successful live test.

## 5:15-5:30 — Triage fresh startup, only if pass acceptance succeeds

- [ ] Identify the smallest missing dependency from the Sao Paulo versus Fluminense pre-match meeting to first control. Use existing startup evidence; separate scoreboard/stage/team initialization from roster levels and first-control evidence.
- [ ] Write a short dependency list and a runnable next experiment. Do not start a broad new startup implementation in the remaining minutes.
- [ ] If pass acceptance is unfinished, spend this time isolating its first remaining blocker instead.

## 5:30-6:00 — Verify, checkpoint and hand off

- [ ] Run a fresh build and the applicable full registered suite; state exact omissions if the runner remains unavailable. Rerun changed original comparisons once after final code changes.
- [ ] Review diff for private data, unrelated edits, encoding churn and unsafe compatibility changes. Commit verified increments to work. Push only reviewed engine commits under existing authorization; do not blindly push the 17 pre-existing ahead commits without reviewing what they contain.
- [ ] Update CLI_HANDOFF.md, WORK_LOG.md and PORT_STATUS.md with one concise dated checkpoint and evidence paths. Avoid duplicative bulk rewrites.
- [ ] Website progress notes are secondary: if updating, use the separate main checkout, edit notes only and verify publication separately. Do not let this displace the engine acceptance work.
- [ ] Final report: what actually ran; original/native matches and differences; exact final owner/control state; live-window evidence or explicit failure; tests; remaining blocker; next experiment. Overall progress is unmeasured. No complete-match claim without a full match through the final whistle.

## Success hierarchy

Best: compatible content plus an original-compared single-defender pass to possession and usable control in the window.
Acceptable partial: compatible content plus an integrated, compared contact boundary and a precisely captured downstream blocker.
Unacceptable: another broad promise, a historical replay presented as a new live game, forced possession, or removing guards without original evidence.
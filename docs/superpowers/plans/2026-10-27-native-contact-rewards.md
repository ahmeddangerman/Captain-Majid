# Native Contact Rewards Implementation Plan

> Required execution: superpowers:executing-plans, inline continuous implementation with one fresh final review.

**Goal:** Persist named-player experience and apply the terminal field-contact reward before the connected clock/tail.
**Spec:** `docs/superpowers/specs/2026-10-27-native-contact-rewards-design.md`
**Stack:** C++17/Python, private original CPU observations, production Make/CMake.

## Constraints and review focus

No original code/data committed; private exporter only. No tests added/run. Preserve named-type gate, packed group/slot mapping, actual low-byte scaling, saturation, reward-before-clock ordering, no duplicate reward on resume, old pack/snapshot support and bounded presence/padding codec. Full presentation/NMI and campaign level progression remain outside scope.

## Tasks

- [x] Derive reward routine and coherent terminal reward-entry observations; implement pure replaceable-content reward helper.
- [x] Add pack6 reward data and optional experience session context with backward-readable bounded snapshot2/exporter.
- [x] Connect explicit rewarded terminal commands, production builds and normal persistent usage; inspect original/native represented states.
- [x] Update logs/docs, request one fresh whole-plan review, address material findings and commit locally for handoff.

## Ledger

Ruling: Inline work continues under standing user authorization. Developer prohibits tests, overriding skill TDD/test commands; record production builds, research observations and normal usage separately.

Pre-flight: Existing terminal operation receives post-reward state and session1 has no experience context. Add a distinct rewarded operation and optional context rather than silently inventing campaign/global experience fields for old snapshots. Snapshot1 remains readable; writer2 has explicit presence plus zero absent padding.

Research: Nine direct original C4C8 observations return normally without skipped calls. Types0/35 bypass; selectors0/3 on type1/stage0 add15/2, alternate campaign mapping changes destination slot,65530 saturates at65535. Stage28/base266 uses only the low-byte scaling and yields1 in observed type3/selector0, not the amount an idealized full-word formula would produce.

Ruling: Preserve original low-byte scaling — original byte operations and direct observations agree. Cost of substituting a seemingly corrected word formula would be nonfaithful campaign rewards.

Research: Continuous original terminal traces call8BC8 once for result3 and twice for result2:88BB calls it, then81BC calls it again after ball placement. Captured represented pre-reward/pre-clock state differs only by that placement for result2; no represented difference for result3. Preserve two distinct grants and saturation steps. The input example's type1 global total goes0→1 for possession and0→1→2 for loose ball.

Ruling: Both result2 grants are deliberate compatibility behavior even if surprising. Their location around ball placement does not affect reward arithmetic in the supported frozen-input path; no original wait/NMI timing is inferred. Cost if collapsed is incorrect experience totals.

Tasks1–3 complete. Make/CMake production builds completed. Pack6 is18,804bytes, session2 is289bytes. Two represented native/original final snapshots have matching inspected hashes (see NATIVE_CONTACT_REWARDS.md). Added-time resume retained total1. Old pack5/session1 loads with experience absent. No tests added/run. Initial exporter command lacked the private py65 PYTHONPATH; rerun with configured path succeeded without a source change.

Ruling: Latest user instruction says finish the task — complete/review/commit this active increment and provide the resume point, rather than starting a further implementation plan.

Task4 complete: One fresh independent Astra High static review found no material findings after reviewing all increment files, original ROM reward arithmetic, private observations and build wiring. Reviewer executed no code/builds/tests. Final Make/CMake production commands returned0 with no diagnostic lines. Native final snapshots match the represented original states as recorded in NATIVE_CONTACT_REWARDS.md. Save source locally, leave private data outside Git, hand off recovery/player-control progression.

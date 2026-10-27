# Energy, Keeper State, and Random Step Implementation Plan

> **For agentic workers:** Execute each bounded task with tests before implementation and one independent final review.

**Goal:** Reconstruct original energy usage, post-comparison keeper state rules, and one original random-generator step.

**Architecture:** Independent pure C++17 functions, strict stateless research protocols, original-ROM comparisons, and regression of the existing math/replica fixture. Research on independent subsystems runs in parallel; integration stays in the primary workspace.

**Specs:** `docs/ENERGY_USAGE.md`, `docs/KEEPER_STATE.md`, `docs/RANDOMNESS_NOTES.md`, under `docs/ENGINE_DIRECTION.md`.

## Constraints

- No external source, translated comments, or bundled game assets copied.
- Preserve wrapped byte/word arithmetic and the actual sign/carry behavior.
- The random-step sampler must honor 2 KiB NES RAM mirroring.
- Implement the random algorithm without claiming its full frame scheduling is resolved.
- Keeper helper invocation is explicit until the shot flow is integrated; do not guess consequences for every comparison code.
- Stateless research queries must leave the ordered fixture untouched.

## Tasks and review focus

- [ ] Energy: independently confirm core/wrapper semantics; write failing tests, implement stamina/cost/weak/carry outputs, and expose strict inputs.
- [ ] Keeper: independently confirm story override and timer/penalty helpers; write failing tests, preserve raw subtype checks, byte timer wrap, penalty cap, and disabled-helper behavior.
- [ ] Random: write failing tests and implement a faithful sampled-byte step plus mirrored RAM sampler; compare both sides of the RAM mirror boundary to real original instructions.
- [ ] Integrate all research modes into terminal/batch and the local TCP proof, checking invalid flags/ranges and fixture-state preservation.
- [ ] Run private-ROM comparisons and the full existing suite, obtain independent review, record limits, and commit locally.

## Integration decision

These are prerequisites for the complete shot. Player/content loading, frame scheduling, scenario/animation calls, possession transitions and scoring are not invented to fill gaps. Their integration follows original gameplay traces.

## Resume ledger — 2026-10-27

- Energy implementation and literal fixtures are present. Earlier standalone execution recorded 36 passing cases; those results were not rerun during this continuation.
- Keeper story/state implementation and literal fixtures are present. Earlier standalone execution passed; the native differential comparison is pending.
- The random step and mirrored RAM sampler are present with literal fixtures. Earlier standalone execution passed; the native differential comparison is pending.
- All four new batch/replica interfaces and Make/CMake definitions are integrated. Existing stateless parsers were extracted into `research_protocol.cpp`, leaving session handling separate.
- The private-ROM harness is prepared with 25,756 cases. No completed native comparison result is recorded for this increment.
- Ruling: preserve the already prepared tests, but do not add or run tests in this continuation because the current developer instruction requires an explicit user request. Full regression, new TCP proof cases, and native ROM comparisons remain pending. Build compilation and static code review may proceed; they cannot establish gameplay fidelity.
- CMake/Windows execution, complete shots, random scheduling, and public networking remain outside this increment's evidence.
- `make -B all` completed on Linux/GCC with no compiler warnings. Independent static review reported no actionable findings and agreed with the original ROM instructions; no tests/verifiers were executed by the reviewer.
- Preserve the work on branch `work` as a local implementation checkpoint. The task checkboxes above remain open where their verification contract is not yet satisfied.

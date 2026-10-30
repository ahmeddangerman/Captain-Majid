# Area Defense Special Availability Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Match the original defensive special-option availability and register ordinary choices when no option exists.

**Architecture:** Private content supplies per-raw-player-type action2 rules. A pure session query checks the selected player and first ten home field records; preparation reuses it and rejects available special moves until their chooser exists. Area variants are nonzero, so original actions0/1 bypass the special loader. No new saved phase or bytes are needed.

**Tech Stack:** C++17, Make, private Python/py65 observations.

**Spec:** `docs/NATIVE_KEEPER_AREA.md`; original bank28:8D58–8E1A.

## Global Constraints

- Source/public branches and private data boundaries remain as AGENTS.md requires.
- Actual special selection and stamina handling remain pending; never silently choose an option.
- Existing snapshot51p383-byte readers/writer remain; private content12C is additive to B.
- Raw player type controls this lookup, not skill-profile remapping or stamina flags.

## Review Focus

- Required teammate absent/present, including actor matching the required raw type.
- Unconditional options and a required raw type0 or255 must remain distinguishable.
- Missing old-pack section and out-of-range player type reject action2 atomically.
- Partial-menu A+direction and final A+B+direction keep the same sample priority.
- Legacy o partial70 positive input must continue to load.

### Task 1: Private rules, native query and ordinary fallback

**Files:** action_menu.hpp/cpp, shot_content.hpp/cpp, export_shot_content.py,
phase_session_lab.cpp, keeper_save_session_tests.cpp.

**Interfaces:** `std::optional<std::uint8_t> keeper_area_defense_special_option(const PhaseSessionState&, const AreaDefenseMenuContent&)` consumes pending71. `prepare_keeper_area_defense` uses the same rule gate; CLI `inspect-keeper-area-defense` reports availability without changing state.

- [ ] Write failing synthetic conditional/unconditional/absent rule tests and compatibility/sample-priority cases; run focused test and observe failure.
- [ ] Implement private C rules and strict decoding, query and preparation fallback; retain older pack readers.
- [ ] Run focused tests; export private pack and verify stripping new section restores exact B.

### Task 2: Original availability and registration proof

**Files:** trace_area_defense_poll.py, new check_area_defense_options_original.py,
NATIVE_KEEPER_AREA.md, WORK_LOG.md, tools/README.md.

**Interfaces:** Observer preparation mode can stop D3E6 immediately after original options loader; count and option compare with native query. Existing registration observer proves ordinary fallback paths.

- [ ] Compare every exported raw type under absent/present team conditions and multiple area variants/actions; inspect actual original0430/0431.
- [ ] Compare ordinary registration for supported no-option branches; available-special preparation must reject without capture.
- [ ] Run Make check/all, fresh review, document exact coverage, commit and push work.

## Execution ledger

Pre-flight: query and preparation share AreaDefenseMenuContent rules; observer stops
before presentation so no special-menu state is invented. Existing authorized work
branch is isolated from website main. Prior plan's minor coverage suggestions are
included as this task's compatibility and sample-priority requirements.

Task1: focused regression failed against the stub, then passed. C content adds
132 bytes; stripping it and restoring B matches the previous private pack.
Task2: original proof passed1,416 option checks and1,404 ordinary registrations;
12 available specials correctly pause. Existing input/registration comparisons
passed with C. Full Make check/all passed. Independent reviewer hit its usage limit;
author review found and corrected a prompt-tool C guard. Special chooser remains.

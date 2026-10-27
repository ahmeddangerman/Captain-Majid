# Prepared Flight Contact Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Connect an explicit later flight-contact burst to native keeper power.
**Architecture:** Pure prepared burst with no stamina debit, optional joined-shot context, backwards-compatible private pack version3.
**Tech Stack:** C++17, Python/py65, Make.
**Spec:** `docs/superpowers/specs/2026-10-27-flight-contact-design.md`

## Global Constraints

One caller-supplied burst; no movement or timing inference. Private data only. Old pack/context behavior retained. No tests added/run.

## Review Focus

Later contacts apply loss while the initial caller preserves power; no defender stamina debit/weakening in this caller; action/pair index and eligibility flags; lazy terminal/gate handling; version3 section layout and old reader compatibility.

## Task 1: Native burst

- [x] Implement `FlightContactContent`, choice/context/result types and `resolve_prepared_flight_contacts` in focused header/source; integrate build targets.
- [x] Connect optional burst after approach, feed keeper power and expose terminal pending handoff separately.

## Task 2: Content and scene

- [x] Append16 flight bytes in exporter/reader version3, retaining versions1/2.
- [x] Expose explicit scene flight count and CLI results, requiring new content when used.
- [x] Build production targets, export private pack and inspect normal crowded/controlled native usage.

## Task 3: Checkpoint

- [x] Document exact evidence/limits, request fresh review, update log and commit locally.

## Ledger

Ruling: Execute inline under standing user authorization. Developer test constraint overrides skill test steps; no automated tests added/run.

Pre-flight: Original stat loader returns a stamina cost but this later caller does not spend it. Reusing the initial prepared-sequence helper would incorrectly debit stamina and omit loss. Use the shared loader and isolated contact arithmetic without that wrapper. Version3 data adds2+2+4+8bytes after version2 geometry; old data sections stay unchanged.

Ruling: Terminal later-flight branches return a pending handoff with explicit result/last defender/action and no keeper, rather than borrowing initial-loop handler semantics without proving them. The cost is unresolved terminal flight state until the later handlers are reconstructed; caller can distinguish this supported boundary.

Ruling: Static inspection shows adjacent instruction bytes beyond the reachable action0 eligibility/threshold rows. Narrow content to the two reachable defense variants, each with its own imported eligibility/coefficient/threshold row. Do not export invented20-row layouts or generalize this helper to attacking actions1..4. The cost is deliberately restricted action0 support until other callers are traced.

Ruling: Static original DD08..DD23 shows flight executes before the saved loose-ball carry and keeper-status checks. Evaluate optional flight even when the approach chose a loose endpoint; only initial-terminal/active-defender gates bypass it. The cost of getting this order wrong would be silently dropping reachable contacts on a loose-ball route.

Build note: GCC14's optimized build diagnosed the existing optional comparison-content construction as possibly reading an uninitialized vector after the input aggregate grew. Direct `emplace` alone retained a warning about destruction of the old value. The input is value-initialized, so set its three context scalars directly instead of assigning a second context aggregate, then construct the still-empty optional with `emplace`. This removes the redundant aggregate/optional reset path without suppressing diagnostics. Fresh production rebuilds record the result; debug CMake compilation had no diagnostic.

Final review: Fresh independent static review found no runtime correctness blocker and one factual locator issue: eligibility reads X at`88EB+action`, while variation reads the doubled index. Both supported reference eligibility bytes are identical at the mistaken/correct offsets, so existing private pack bytes and observed behavior are unchanged.

Ruling: Correct this low-priority factual locator and its docs in the same local checkpoint instead of retaining known inaccurate original-code extraction instructions. Static original register flow and both source byte values were inspected directly. This is a small accuracy correction, not a claim of a new behavioral test; no tests were added/run. The remaining cost is lack of broad differential/terminal coverage.

Evidence: Final Make and CMake production builds completed successfully with no warning/error lines; BUILD_TESTING=OFF. Normal Make/CMake laboratory use showed the crowded power33→21/ratio22/catch and stamina1000/980; older pack defaults and controlled catch/goal remain available. A level1 scene stops after first result3 with a pending handoff and untouched keeper stamina. Corrected exporter produced a private11,833-byte version3 pack. No automated tests added/run. Optional SDL target was not rebuilt in this increment.

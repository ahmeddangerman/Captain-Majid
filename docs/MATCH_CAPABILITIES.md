# Match runtime capability map — 2026-11-11

This is an engineering map, not a claim of a complete match. `captain-match-lab CONTENT_PACK DEVELOPMENT_SESSION` loads explicit local inputs, prepares existing player-control handoffs, or identifies the first unsupported runtime operation. The snapshot is a development fixture; the executable does not initialize a match or loop autonomously. A control-ready snapshot currently stops at live timing and random scheduling.

| Stage | Existing entry point | Classification | Next evidence or integration needed |
|---|---|---|---|
| Fresh initialization | `prepare_session_kickoff_formation`, `prepare_session_kickoff_receiver`, `prepare_session_kickoff_ball` | Partial; start from captured goal/kickoff context | Original match startup and content/formation initialization; synthetic fixtures are not proof. |
| Control setup | `prepare_session_control` | Reusable for validated player-control states; wired into runtime | Broader owner/side coverage and control-presentation handling. |
| Control polls | `poll_session_control`, `advance_observed_nmi_control_poll` | Shared kernels, but observed-NMI version requires supplied random/travel inputs | Determine live scheduling, random memory and input ordering without captured events. |
| Opponent choice | `plan_ai_area_action`, `choose_ai_area_candidate` | Partial; unsupported branches/team resolution | Controlled original cases covering action selection and missing recipient/subtype paths. |
| Pass target and flight | `resolve_pass_target_cell`, `advance_ball_to_next_cell` and session flight functions | Numeric kernels reusable; several session routes scenario-bound | Connect general state/content adapters and compare defended/undefended passes for varied players. |
| Contact | `compare_defender_contact`, `apply_contact_prelude` | Shared arithmetic; selection and dispatch partially scenario-bound | Original comparisons for alternate actions, stamina and outcome routing. |
| Possession | `prepare_session_control`, recovery and pass session functions | Partial, much guarded by captured state | General reception and turnover transitions, both teams. |
| Shot/keeper/goal | prepared-shot, keeper and goal session modules | Numeric and sampled flows exist, no unified match routing | Original comparisons and generalized restart/goal paths. |
| Clock/period | `advance_match_clock` | Numeric clock exists; live event scheduling missing | Controlled clock progression including half/full time and timing effects. |
| Presentation | shot scene viewer and presentation request structures | Development viewer, no match client | SDL runtime client; decouple cosmetic waits from game effects. |

First implementation task remains incomplete: full initialization, safe live timing, action payloads and possession acceptance are not implemented. `MatchConfig` accepts a validated development state only; nonzero seed/clock options explicitly reject until their semantics exist. Synthetic test content does not demonstrate a playable match. `MatchStepResult` preserves the input state and sequence for unsupported operations.

Ruling: use an explicitly provided existing `PhaseSessionState` as the initial development fixture until original startup is established. This keeps replay snapshots reusable but does not satisfy fresh-match initialization. MatchInput sequence numbers prevent duplicate accepted operations; unsupported requests do not consume them. Neither mechanism claims original gameplay parity.

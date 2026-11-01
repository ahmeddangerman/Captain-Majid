# Opponent action after area defense selection

The completed defensive menu returns to bank26:8B8F. Route2 continues at8978,
whose first call enters the stats bank's8006→8609 dispatcher. With opponent
possession it runs875D to choose the attacking action before returning at897D.
Only then does8F72 prepare the attack, followed by presentation and defensive
action processing. Reusing a fixed ordinary-shot preparation here would skip
the opponent's decision.

`tools/trace_area_attack_choice.py` executes the original CPU from8B8F to897D
with bounded instructions. It rejects scheduler or unexpected return boundaries;
no scheduler or display calls are skipped within the observed choice. Its
constructed completed prefix comes from an existing captured area-menu fixture.
ROM, generated RAM and observations remain outside source control.

The first observation covers768 random pairs on that single fixture: every first
random byte and second bytes0/127/255. It produced action1 in192 cases and action6
in576 cases. Evidence: `area-ai-choice-first/observations.json`. These numeric
codes are recorded without assigning unverified action names. Other rosters,
formations, areas and live random scheduling remain unverified.

This is original-game research, not native AI implementation or an executed
match action. Next: trace875D's route/category and weighted choice data, reproduce
its supported decisions natively, then connect attack preparation and the saved
defensive prefix to their actual outcomes.

## Native packed candidate sampler

`choose_ai_action_candidate` accepts four bytes containing eight action codes and
a random byte. Original bank28:8B0B uses the random byte's low three bits: even
indices read the upper nibble and odd indices read the lower nibble. Duplicate
codes preserve their original probability. The function accepts supplied data;
no original action tables are embedded in the distributed source.

`check_ai_action_candidate_original.py` compared8,192 cases against the original
routine using32 synthetic replacement rows and every random byte. Native focused
checks and original comparisons passed. This covers sampling only; selecting the
row from player profile/field category and later subtype adjustments remain
unimplemented and the sampler is not yet connected to the saved match session.

## Native route2 profile and field category

`choose_ai_area_candidate` now selects the route2 candidate data from a direct
opponent player type and its raw alternate flags, folds the field position into
the original cell/category map, and calls the packed sampler. Generic team-type
resolution and types outside the captured169 profile rows reject explicitly.
The private G pack adds493bytes (profile row IDs, cell categories and six action
rows); the canonical pack is20,717bytes. Its earlier F payload is unchanged.

`check_ai_area_candidate_original.py` matched21,632 original comparisons across
169 direct types, ordinary/alternate flags, eight positions and eight random
indices, stopping at87B0 before final action/subtype adjustment. Five unsupported
input cases reject, as do malformed AI content sections. Evidence:
`ai-area-original.log`. Full Make check/all pass. This is not yet the final AI
choice or a saved-match continuation; those must preserve the later original
adjustment branches before attack preparation is connected.

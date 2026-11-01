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

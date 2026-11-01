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

## Supported final action adjustments

`choose_ai_area_action` applies original route2 handlers for candidate0,1 and3,
returning the final action and subtype. Candidate2 enters the separate8933
recipient-selection flow and explicitly rejects in this operation. It is not
silently replaced by another candidate. Session integration remains pending.

`check_ai_area_action_original.py` compared21,632 controlled inputs including
mirrored positions. The18,832 supported final action/subtype pairs matched the
original return at897D. The2,800 recipient-choice cases were identified using
the original candidate and rejected by the native operation. Full Make check/all
passed; evidence: `ai-final-original.log`. No complete opponent turn is claimed.

## Recipient branch research

`trace_ai_area_recipient.py` observes8933 until8A3F (before special-move lookup)
or its caller return. The first private report covers12,168 constructed cases:
169 direct profiles, three teammate position patterns, eight first-random values
and three second-random values. It reached move lookup in5,911 cases and fallback
in6,257. Evidence: `ai-recipient-first.json`. These are original observations,
not native recipient-choice parity.

The branch can try a preferred teammate, scan field players12–21 for a teammate
on the relevant side, or choose another target using random bytes. Some strategies
rotate the first random byte, so native integration must preserve the changed
random state as well as the chosen player. The fallback action is4 in this route.
After a recipient is found the original writes its ID and field cell before
looking up the move subtype. Those steps must be represented together; action
and subtype alone cannot describe this branch.

## Native recipient selection and combined plan

The native recipient routine now reproduces the preferred target, teammate scan,
fallback action, target cell and random-byte rotation. The original retry path
computes another random target but does not store it before retrying the old
target; the native code preserves this behavior. The original comparison matched
15,210 cases across169 profiles, three position patterns, all ten carrier IDs,
multiple ball positions and random pairs. Evidence: `ai-recipient-profile-original.log`.

Private content17H adds557bytes of recipient-profile and recipient-choice rows;
the canonical pack is21,274bytes and the previous G payload remains unchanged.
`plan_ai_area_action` combines candidate, final adjustment and recipient choice
for supported route2 inputs. Nonzero area variants skip action1 special subtype
options in the original. The native plan requires variants1/2 and rejects raw
option-group types outside the represented range when entering the recipient
branch. Generic teams are still unsupported.

`check_ai_area_plan_original.py` matched5,976 complete choices through897D,
including action, subtype, target, field cell and final random state. This is a
pure planning component; the saved match phase and attack preparation are next.

## Saved area action choice

`choose-keeper-area-action` connects completed defensive choices (phase72) to
phase78, before attack preparation. Action, subtype and random state are committed
with the optional recipient and cell. The target cell must agree with the saved
recipient position. Snapshot56u adds three bytes (396 total); prior formats remain
readable and cannot encode phase78. Empty defensive prefixes are not connected
by this operation yet.

`check_area_ai_session_original.py` compared480 complete saved choices to original
execution through897D, across four profiles, two variants, three teammate position
patterns and random inputs. Complete represented state matches; evidence:
`ai-session-final`. Roundtrip, replay, invalid target cell and older version checks
are covered. Full Make check/all pass. Live scheduling, generic team resolution,
attack preparation and resulting defense execution remain unfinished.

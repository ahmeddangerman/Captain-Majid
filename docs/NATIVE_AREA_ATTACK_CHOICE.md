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

## Opponent area attack preparation

`prepare_opponent_area_action` now joins profile-based move preparation and
original power adjustment for supported actions1/4/6 and area variants1/2.
Action6 needs additional private selectors and two moves; content18I adds12bytes
to the H pack (21,286bytes total). Its earlier H payload is unchanged. The shared
numeric loader is reused with action6's private data; it does not substitute an
ordinary-shot move. Unsupported action/variant/subtype or missing data reject.

Original CPU comparisons through8980 matched640 cost, effect, flags, power,
subtype and stamina outputs. Evidence: `area-attack-original.log`. The opponent's
stamina is preserved by this stage. This preparation function is not yet committed
to the saved phase78 session; that connection and presentation/defense execution
remain the next task.

## Saved prepared opponent attack

`prepare-opponent-area-attack` now continues saved phase78 to phase79 at8980,
before presentation. It commits power, adjusted subtype, effect, flags and cost;
the roster stamina and match clock remain unchanged. Snapshot57v adds two cost
bytes (398 total); old saved choice inputs remain readable. Repeated preparation
rejects. `check_area_ai_session_original.py --prepare` matched480 saved choices
and their480 prepared continuations against complete represented original state.
Evidence: `area-attack-session-original.log`. Full Make check/all pass. Action
presentation and defensive resolution are still next.

## Prepared action presentation and defender entry

`begin-prepared-area-defense` advances prepared phase79 to phase80 at8998,
after script14 and before the defender loop. This is an explicit prepared
presentation acknowledgement, not a native renderer. Original EC3E clears the
motion request; the script's variant selector at bank16:8259 chooses the branch.
Variant1 subsequently requests motion2 (ED32 and bank24:8701), while variant2
leaves it0. The native state reflects this distinction and emits script14.

Snapshot58w stays398bytes and adds the new phase. `--present` on the saved-choice
checker compared480 original choices, preparations and presentation continuations
to8998, including full represented state and script request. Evidence:
`area-presentation-final`. Host frame acknowledgements remain explicit. Full Make
check/all pass. The defender loop and its gameplay outcomes remain next.

## Prepared defender announcements

`announce-prepared-area-defenders FACING` advances phase80 to81 at89D5.
For supported area choices2/4/5 it emits script15 and a20-frame wait per defender;
action6 skips that announcement. Script4 and a final1-frame wait complete the
prefix. The last announced player becomes the saved opposing player; the final
motion request is16. Final facing0/64 is supplied explicitly from acknowledged
presentation and is not claimed as independently calculated.

Snapshot59x stays398bytes. Original comparisons matched32 cases across one/four
defenders, ordinary/special/skip patterns and both area variants, including full
represented state and ordered script/wait requests. Evidence:
`area-announcement-first`. Full Make check/all pass. Host acknowledgements remain
explicit; no native live frame advancement or defender calculation is inferred.

## Defender preparation and energy

`prepare_area_defender` reproduces the numeric stage89DA..8A05 for supported
human defenders1..10, actions2/4 and area variants1/2. It prepares skill/effect
from private profiles and applies the original stamina cost, including raw
type32's cost adjustment and the weakened flag after an exhausted subtraction.
This pure calculation does not yet commit a defender into the saved session.

The original CPU checker matched2,430 strength/effect/cost/stamina/weakened/carry
outputs across three raw types, three levels, both variants and ordinary/special
defense choices. Seven unsupported inputs reject. Evidence:
`area-defender-original.log`. Full Make check/all pass. Next: area-specific
comparison tables and classification, then saved defender execution.

## Area-specific contact comparison

`compare_area_defender` combines preparation/energy with the original area
comparison at8A05..8A28. Its action groups, variation pairs and thresholds are
separate from ordinary shot comparisons. Private J adds36bytes (21,322 total);
the earlier I payload is unchanged. The shared numeric roll/classifier is reused,
including special defense and exhausted weakening. Incoming attack power is not
reduced by this caller. Unsupported defense actions and missing data reject.

`check_area_contact_original.py` matched9,720 complete results, covering all four
classification codes, random boundaries, exhausted stamina, both variants and
supported attacks1/4/6. Four malformed packs reject. Evidence:
`area-contact-original.log`. Synthetic selection tests and full Make check/all
pass. Saved session integration and result dispatch/presentation remain next.

## Saved first defender contact

`resolve-area-defender` now continues announced phase81 to phase82 before the
original result timing at8A2B. It commits the first selected defender's stamina,
identity, prepared effect/cost, rolled strength, subtype and comparison result.
The attack power and clock remain unchanged. Snapshot60y adds14bytes (412 total);
older announcement snapshots load with no contact context. Replayed calculations
reject before charging stamina again. Unsupported first choices5/6 still reject
atomically; skipping them and advancing subsequent defenders remain pending.

Original comparisons matched1,296 complete saved states across both variants,
one/four selected defenders, supported attacks, special/ordinary defenses, stamina
and random boundaries. All four results are covered. Interactive capture, replay,
malformed metadata, legacy announcement reading and full Make check/all pass.
Evidence: `area-contact-session-original.log`. Earlier32 announcements and480
choice/preparation/presentation chains also pass with the new format. A test input
initially used incorrect field-cell geometry; corrected to the existing8-pixel,
12-row coordinate mapping before the final run. Result timing/dispatch is next.

## Saved contact recovery and clock

`advance-area-contact-clock` continues phase82 through the original8A2B..8A3F
status/recovery/clock stage. Results0/1 affect the defender; results2/3 affect
the ball carrier, following the original carry selection. Stamina is retained
from the earlier calculation. It saves normal continuation83, added-time pause84
or period-end85; the period transition itself is not implemented here.
`resume-area-contact-clock` acknowledges prepared added-time presentation and
changes84→83 only, without repeating time, recovery or status effects.

Snapshot61z remains412bytes. Older y compared-defender snapshots load, while
y cannot encode the new clock boundaries. Roundtrip, malformed boundaries,
replay and legacy reading are covered. `check_area_contact_session_original.py
--clock` matched1,296 complete saved contacts and156 full post-contact snapshots
across all four contact results and normal/added-time/period-end routes. Original
clock display enters its scheduler; the observer executes those instructions with
explicit frame acknowledgements and frozen random input, rather than skipping
the display routine. This does not establish native rendering or live scheduling.
Evidence: `area-contact-clock-original.log`. Result presentation and advancing
subsequent defenders remain next.

## Ordered defender sequence and prepared result display

`present-area-contact-result` continues saved clock phase83 to86 at8A48. It
requests script16 for terminal results2/3 or script17 for results0/1, then
script18. The terminal presentation clears facing and motion; nonterminal
presentation preserves them. These are explicit prepared display acknowledgements,
not a native renderer. Repeating the presentation operation rejects.

`advance-area-defender` routes phase86 to the next selected defender87, the
exhausted prefix88 before original9085 final timing, or terminal exit89 before
possession dispatch. Nonterminal advance increments the cursor once, discards
the old comparison and requests the original one-frame wait when another player
remains. Terminal results keep their context and do not process later players.
`resolve-area-defender` now uses the saved cursor in phases81/87, so subsequent
supported defenders reuse the existing calculation/clock/display pipeline.

`skip-area-defender` handles choices5/6 in phases81/87. It records the selected
identity and advances without charging stamina, applying contact timing or
inheriting an old result. Both all-skip and mixed prefixes stop at the correct
next/end boundary. Other unsupported choices continue to reject atomically.

Snapshot62 uses explicit byte128 after the earlier ASCII1..z versions. It adds
one loop-cursor byte at412 (413 total), preserving every previous offset and
reader. Old y/z contact and clock snapshots with cursor0 still load. A later
contact cannot be downgraded to an old format by removing its new cursor; the
existing contact cursor would disagree and the reader rejects it.

`check_area_defender_sequence_original.py` compares complete represented
contact, clock, presentation and next/end snapshots plus ordered requests. The
original observer executes actual instructions with frozen random bytes and
explicit host frame acknowledgements. It stops before final prefix timing or
terminal possession dispatch; neither is implemented by these new operations.

Final original comparisons matched720 complete sequences:696 contacts and748
skipped entries across one/four selected players, both area variants, supported
attacks1/4/6, all four results, ordinary/special defenses, exhausted input and
both saved facing states. Every selected slot0..3 is exercised. Interactive
chains and replay guards match. Evidence: `area-sequence-work/original-final.log`.
Earlier1,296 saved contacts,156 clock states,32 announcements and480 opponent
choice/preparation/presentation chains pass with the new format. Full Make
check/all and independent review pass; focused legacy tests preserve cursor0
y/z compatibility and reject downgraded later-slot contacts.

## Final prefix clock calculation

`calculate_area_prefix_clock` previews the original final elapsed-time and keeper
recovery calculation after the saved defender prefix is exhausted. The caller
supplies the elapsed value from the original action table. The function leaves
the input and saved phase unchanged, and returns the resulting state, clock
continuation and elapsed step. It rejects other phases. This is a pure numeric
piece; it is not yet connected to a saved phase, the action table or possession.

`check_area_prefix_clock_original.py` matched 90 controlled original CPU cases
for supported attacks, normal time, added time and period end, including elapsed
wrapping and keeper recovery. The original observer uses explicit host frame
acknowledgements; no native rendering or live scheduling is inferred. Evidence:
`area-prefix-original.log`.

## Saved final prefix clock

`advance-area-prefix-clock ELAPSED` commits the previously verified calculation
from phase88 to normal phase90, added-time pause91 or period-end boundary92. The
elapsed byte must currently be supplied explicitly from the original action
table; automatic content-table selection remains pending. The operation rejects
a second advance, and `resume-area-prefix-clock` changes91 to90 without repeating
the clock or recovery. Period transition and possession dispatch remain pending.

Snapshot63 uses byte129 and remains413 bytes. Earlier byte128 saves load, but
cannot encode the new phases. Ninety complete original saved states match through
the final prefix clock, covering normal time, added time and period end. The
original observer uses explicit frame acknowledgements and frozen randomness.
Evidence: area-prefix-saved-original.log. Full Make check/all pass.

## Automatic final prefix timing

`advance-profiled-area-prefix-clock` selects the elapsed byte from the saved
attack action using original-derived private timing content, then commits the
existing saved clock continuation. Private pack K adds seven timing bytes;
the preceding J payload is unchanged. Older packs remain readable, but this
operation rejects them when timing content is absent. The explicit elapsed
operation remains available for development comparisons.

Ninety complete saved cases match original execution, including actions1/4/6,
normal time, added time, period end and keeper recovery. Missing/truncated timing
content and repeated advancement reject. Evidence: `area-auto-original.log`.
The following possession dispatch and a complete playable match remain pending.

## Area result dispatch movement preview

`preview_area_result_dispatch` previews original8A6F through8A72, where C606
advances the field before the result is dispatched. It requires a saved normal
prefix clock or terminal result and private movement content. It returns the
advanced field and a route:0 for attack continuation (original results0/1),
2 for loose-ball recovery, or3 for the original terminal handler. It does not
change the input session, commit the movement, award experience, or resolve
possession. The exact last nonterminal result is not retained by the existing
prefix snapshot; both nonterminal results share this initial route.

Sixty original CPU comparisons match the complete represented field across
20 cases per route, elapsed and random boundary inputs. The observer executes
C606 with frozen input randomness and explicit host frame acknowledgements.
Evidence: `area-dispatch-original.log`. Full Make check/all pass. Saved movement
and actual downstream result/possession handling remain next.

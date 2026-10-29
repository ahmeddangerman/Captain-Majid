# Prepared keeper-area interaction setup

The saved recovery flow can now enter original8B4A..8B73 numeric setup before
its interaction presentation/menu. This follows phase7 and ends at new phase65,
`keeper_area_prepared`. It does not execute the subsequent menu or player actions.

The native `prepare_session_keeper_area` function requires a captured ordinary
(action0) recovered-ball state in an original keeper area: Y96..159 with X<80 or
X>=176. It preserves an existing ball variant, or chooses random.bit0+1 when the
variant is0. The selected prefix is cleared, the interaction route is2 when the
area side matches possession and1 otherwise, and the original counter is set255.
It then uses the existing exact-cell field-player selection, status exclusion,
wrapped random subtraction, scan order and side-dependent5/4 limits.

Both keeper Y bytes, score, raw positions, fractional coordinates and other saved
state survive. Neither movement nor a match clock is advanced. Invalid inputs and
replay reject without mutating their source. The development CLI accepts
`--prepare-keeper-area`, and interactive mode accepts `prepare-keeper-area`.

## Save format

Snapshot45 (`j`) is372bytes. It appends keeper-area presence and counter to the
previous370-byte layout. Presence is required exactly at phase65, with counter255;
unused counter bytes must be zero. The route must agree with the captured area
and possession side. Earlier snapshots through i remain readable and cannot
encode phase65. Truncation, malformed presence, missing context and bad route/counter
states reject. These checkpoints are not a complete match save or network protocol.

## Original comparisons

- 64 recovered-ball entries from the controlled rebound suite match original
  setup before8B73. The preceding outcome/recovery/control comparisons remain
  included. Evidence: `keeper-area-original-first`.
- 24 constructed cases check both sides and areas, generated/preserved variants,
  random subtraction, status skipping, scan order and selection limits. They cover
  both routes and selected counts0/4/5. Evidence: `keeper-area-routes-first`, via
  `tools/check_keeper_area_original.py`.
- All instructions in original8B4A..8B73 execute, including DCDF and87E1 calls.
  The routine ends before its presentation. The recovered-ball fixtures originate
  from explicitly supplied non-goal keeper results and do not establish naturally
  occurring save classifications or a complete playable match.

Focused regression failed on the initial stub, then passed. Save roundtrip, prior
i input, forbidden old output, malformed/truncated context, invalid area and replay
checks pass. Full Make check/build pass. ROM and generated evidence remain private.

Next: trace8B73 presentation/story/menu boundaries and the following action route.

## Prepared route2 menu entry

`open_prepared_keeper_area_menu` continues phase65 with own-area route2 only.
It recomputes nearby opposing field players using radius10, retains the variant
and route, and captures completed menu presentation effects. Side0 reaches the
existing human menu phase18; side11 reaches new phase66 before human defensive choices.
The human cursor resets to0; the defensive-entry cursor is preserved.

The original final facing depends on animation state that the native renderer
has not implemented. The API therefore requires an explicit observed facing0/64.
CLI: `--open-prepared-keeper-area-menu FACING`; interactive mode uses the same
command without the leading dashes. Typed requests preserve original script63,
keeper warning priority, overlay clear and human route2 title/background order.
These are requests and prepared acknowledgements, not executed native graphics.

Snapshot46 (`k`,372bytes) permits KeeperAreaContext in phase18/66 and reads the
prior j phase65. Prior j cannot encode the new attack/defense menu combinations. Context,
area, possession, route and presence validation remain strict. Route1, original
story/presentation effects beyond this prepared scope, defensive choices, and subsequent
action execution remain unfinished.

`check_keeper_area_menu_original.py` matches24 controlled cases across both sides,
near/far players, random values, normal/penalized/stunned warning conditions and
stale menu/control fields. Complete represented bytes and typed presentation
request order match original8B73 to D5E5 (human) or D36E (defensive entry). The final
facing value is explicitly supplied, not independently reproduced. Evidence:
`keeper-area-menu-first`. Focused regression failed with the stub before passing;
roundtrip, legacy j setup, forbidden j menu, facing/replay/route checks pass.

## Human own-area selection and confirmation

Snapshot47 (`l`,372bytes) retains area context at confirmed human choice phase19.
Existing `--menu-input` supports route2 directions, inactive confirmation, blinking,
combined input and confirmation without discarding keeper/area metadata. Prior k
menus remain readable; prior k cannot claim the new area-context phase19.

`check_keeper_area_menu_original.py --polls` matched all256 pressed bytes in both
inactive and active selection states:512 complete snapshots and sprite requests.
There are280 waiting/menu states and232 confirmations; confirmed original action
IDs are1/4/6. Evidence: `keeper-area-choices-first`. The24 menu-opening comparisons
are included. These choices stop before execution; ordinary-shot preparation stays
rejected for route2.

Correction to earlier terminology: D36E is the human defensive-choice handler
when the opposing team has possession (it reads controller input and selected
human defenders). Phase66 is named `keeper_area_defense_pending`. Earlier notes
calling it AI entry were a label error; serialized phase66 and its stop address
are unchanged. This checkpoint does not implement defensive inputs or AI actions.

## Defensive initialization and empty-prefix return

Snapshot48 (`m`,382bytes) adds optional defensive state: four action slots, four
subtype slots and a captured blink counter. Phase67 represents initialization
before D390 draws the defensive menu. Nonempty prefixes reset cursor0 and all
four actions toFF; subtypes behind FF actions are canonical zeros because original
code cannot read them before a new direction clears them. The prior0620 blink
counter is supplied explicitly. Its previous value is not inferred from menu flags.

With no selected defenders, original D37D returns immediately. Phase68 preserves
cursor/menu fields and has no defensive context. It stops at8B8F before the area
action route. CLI/interactive: `begin-keeper-area-defense BLINK` (leading `--`
for the flag). Legacy l phase66 is readable; l cannot encode67/68. Missing,
truncated and malformed context and replay reject.

`check_keeper_area_defense_original.py` compared27 controlled cases: counts0/1/4,
blink0/7/255 and prior flags0/128/211. Full represented state matches original
menu execution to D390 initialization or8B8F empty return. Unread stale subtype
bytes are explicitly normalized, not claimed as preserved raw RAM. Evidence:
`keeper-area-defense-init-first`. Menu drawing, defensive input and area actions
remain the next work. Host acknowledgements and hardware limits still apply.

## Initial defensive prompt and private content

Snapshot49 (`n`,382bytes) saves phase69 after the initial defensive prompt, before
its input wait. The native operation emits a count display, one blank sprite29
per defender with explicit slot0..3, and the selected-player title. Its saved
match/choice state is otherwise retained. `ControlPresentationRequest` now carries
an optional slot index default0 so multiple defensive icons remain identifiable.
CLI/interactive command: `open-keeper-area-defense` (flag form starts `--`).

Private content10 (`CMJSHOTA`) appends15bytes to the previous pack: three defensive
subject labels and three four-direction rows. The native reader still accepts
versions1..9; prompt opening requires the new private section. The exporter uses
the approved ROM and writes the new format. The canonical pack is20,081bytes;
removing the new section and restoring header9 exactly matches the prior v9 pack.
No table bytes or generated pack are distributed with source.

`check_keeper_area_defense_original.py --prompt` matches27 initialization cases
and18 nonempty prompts, including complete saved state and ordered slot-aware
requests. Original8B73 runs through menu/initialization to D3A6. Evidence:
`keeper-area-defense-prompt-first`. Bad/truncated/trailing menu content and missing
v10 content reject; prompt replay rejects. Save roundtrip and legacy m initialization
are covered. This is prepared drawing requests, not native rendering or defensive
input execution. Earlier hardware/acknowledgement limits continue to apply.

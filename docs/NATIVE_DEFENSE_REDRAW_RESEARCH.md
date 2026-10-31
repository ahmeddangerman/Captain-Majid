# Defensive choice redraw research

This describes a traced original-game boundary, not implemented match rendering.
The trace uses the approved Japanese reference privately. No ROM bytes or derived
content are included here.

## Original flow after the special chooser

At D3F4 the original records whether the chooser canceled. A non-canceled choice
loads the selected subtype's cost and checks stamina without deducting it. An
accepted choice stores the subtype at `$0606 + cursor` and advances `$061E`;
a denied or canceled choice leaves both unchanged. At D40C all three paths clear
the overlay and call D4EA. That redraw emits the count display followed by one
sprite request per selected defender, in slot order. Afterward a nonfinal cursor
can request a subject display. The saved cancellation flag and current cursor
determine whether execution reaches D438 or D430, so the full sampled chooser
input must survive until the subsequent control-flow path is completed.

## Field icons and their bounds

D4EA calls D4E4/D504 for each slot. Action FF selects the blank icon. For a field
player, D504 uses action as an index into the seven little-endian pointers at
`$D52B`, then uses the slot's subtype as an offset into that action's icon list.
The list starts are `$D539`, `$D53D`, `$D542`, `$D544`, `$D545`, `$D546`, and
`$D547`; the next table starts at `$D548`. Their lengths for actions0–6 are
therefore **4, 5, 2, 1, 1, 1, 1**. The code performs no subtype bounds check;
an invalid index reads adjacent original data. A native implementation should
reject unsupported combinations rather than silently treating adjacent data as
valid icon content.

For the currently represented route2 human field-player choices, action2 is the
only action that reaches the special-option loader. The four nonempty approved
reference rules for action2 return subtype1; its icon is the second byte of the
action2 list, at `$D543`. Existing private content through E stores the subtype0
icon for each of the seven actions but not this subtype1 icon. It is sufficient
for the next narrow increment to append the single missing action2/subtype1 byte
to a new private content version. The seven subtype0 values and old readers should
remain usable for previously supported cases. Whenever a subtype1 icon is needed,
missing new content must reject the operation atomically. The original's
goalkeeper icon path is separate and is not covered by the existing selected field
player IDs1–10.

The current chooser can represent synthetic option values such as0 and127 for
testing. Option0 is a valid ordinary choice. The action2 icon list has only two
entries, so a synthetic subtype127 cannot be claimed as a supported registration:
the native continuation must reject it before mutating state. Supporting other
actions' nonzero subtypes later requires exporting their additional list entries
and validating each action/subtype combination against available private content.

## Saved-state and interaction implications

Current session validation requires every stored defensive subtype to be zero.
Before recording an accepted action2/subtype1, introduce a new snapshot version
and permit that pair in every defensive phase that can retain prior choices,
including waiting, confirmation, special chooser, result and redraw boundaries.
Do not relax initialization or FF/inactive slots: their unused subtypes remain
canonical zero. Older snapshot versions must continue rejecting nonzero subtype
data they could not previously encode.

An earlier defender's special subtype can persist while later defenders are
chosen, or when B returns to a previous slot. Consequently both the menu's back
icon and blinking icon lookup must use the stored subtype, not just the selected
action. A direction that repeats the current action preserves the subtype in the
original; a direction that changes action clears it. Canceling or failing the
stamina check leaves the previous slot subtype intact, including an earlier
special choice. Validate every required icon and state before returning a mutated
copy or emitting an incomplete sequence of presentation requests.

The move's loaded effect is original `$0445`. The existing `action.move_effect`
represents `$0444`; they are distinct values and must not be aliased. The original
display/sprite operations still depend on explicit host acknowledgements in the
CPU observer. Comparing those requests does not demonstrate a running renderer,
NMI, audio system or complete match.

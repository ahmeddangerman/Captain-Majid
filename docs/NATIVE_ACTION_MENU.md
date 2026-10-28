# Native human action menu and prepared shot

The phase-session lab now continues the supported side0 field path through its ordinary attacking menu, a confirmed action, and preparation of an ordinary shot. It stops with the shot prepared. It does not choose a special move, ask defenders to act, simulate the shot flight, or decide a goal/save.

## Private development formats

`tools/export_shot_content.py ROM NEW.cmjpack` now writes `CMJSHOT9`. It appends the six original direction rows, ten menu-icon IDs, and the title/background IDs used by this menu. The exporter still checks the verified Japanese reference payload. Keep the pack and ROM outside Git.

`tools/export_phase_session.py RAM NEW.cmjsession --phase N` now writes a324-byte `CMJSESSION7`. It appends an optional action context containing the current action, route, cursor, flash counter, auxiliary byte, move result fields and power. Phases18–20 require this context. Older session versions remain readable, but they cannot enter a new menu phase unless the missing context is captured explicitly.

The supported phase numbers are:

| Phase | Meaning |
| --- | --- |
| 11 | Action menu requested during normal control |
| 15 | Nearby field contact selected |
| 18 | Human attacking menu open |
| 19 | Menu action confirmed |
| 20 | Ordinary shot prepared |

Menu entry supports the ordinary side0 route. It keeps the selected opposing-defender prefix and old action byte, emits the observed entry and keeper-warning requests, clears the overlay, and requests the original title/background. The old action byte is preserved while the cursor is inactive; the menu action range is enforced once a selection is active. A defending-keeper stun warning takes priority over a keeper-penalty warning. These requests describe work for a future renderer; the lab does not render frames or advance NMI timing.

## Menu input and shot preparation

`captain-phase-session PACK STATE --open-action-menu` enters phase18. One `--menu-input PRESSED` supplies the original low four directional bits and A bit128. When several directions are pressed, the first low bit wins. A confirms an active choice, including direction and A in the same poll. Without confirmation, the cursor and flash counter advance once; this is a logical poll, not a rendered frame.

`--prepare-basic-shot` accepts a confirmed ordinary action0 and an explicit ordinary subtype0 choice. It applies the original raw-cost affordability rule, then reuses the existing attacker preparation, energy and power calculations. Other menu actions remain pending for later work.

Example command sequence, using private paths created by the exporters:

```text
captain-phase-session PACK CONTACT.cmjsession --open-action-menu --capture MENU.cmjsession
captain-phase-session PACK MENU.cmjsession --menu-input 129 --capture CONFIRMED.cmjsession
captain-phase-session PACK CONFIRMED.cmjsession --prepare-basic-shot --capture PREPARED.cmjsession
```

`129` combines the lowest direction bit with A. Use a different pressed-byte value to reproduce another input.

## Evidence and limits

Bounded original CPU observations and native session snapshots matched byte-for-byte at the contact-menu opening, free-menu opening, defending-keeper stun and penalty warnings, first direction, repeated direction/blink, direction plus A confirmation, and prepared-shot boundaries. The observed prepared example ends at stamina917, power45, subtype128. A constructed stamina60 input was rejected against raw cost80 by the native affordability check. These are manual observation/CLI comparisons, not an automated test suite.

The original/native observations use a cooperative host policy that acknowledges the bounded display/graphics calls; they do not model PPU, audio, live NMI scheduling or frame timing. Special-move inventory, pass targets, other action outcomes, defender decisions and flight remain future work. The state file is a development checkpoint, not a complete match save or network protocol.

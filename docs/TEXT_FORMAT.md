# Text and Dialogue Format

**Status:** Initial text path mapped from static analysis. Control-code names are provisional until emulator traces confirm runtime state.

## Rendering path

The main dialogue engine is in 8 KiB PRG bank 24, mapped at CPU `$8000`.

1. `$800F` selects a message pointer from the table at `$9220`, indexed by RAM `$05EA`.
2. The scene state machine at `$8053` reads timing and window-layout bytes.
3. `$82F2` builds two PPU transfer packets in RAM `$04A5` for the top and bottom halves of the 8×16 font.
4. `$8346` reads the encoded text stream through pointer `$5F/$60`.
5. Bytes below `$E0` go to glyph emission at `$8629`.
6. `$8629` calls the fixed-bank glyph mapper at `$CBC2`, which returns the top tile in `Y` and bottom tile in `A`.
7. `$E0–$FF` dispatch through a 32-entry control table beginning at `$8364`.
8. The NMI routine at `$C8FB` uploads queued packets from RAM `$04A5` to PPU `$2007`.

The complete Arabic ROM keeps this core path unchanged. The Extraordinary edition keeps the same path and control table, but modifies glyph-remapping data and a few substitution tables.

## Message structure

The pointer table at `$9220` references records beginning around `$9400`. A typical record contains:

```text
[delay] [window/layout] [encoded line] FC [encoded line] FC F0
```

- The first byte controls message timing.
- The second byte selects the window layout and line count.
- `$FC` flushes the current text packet and advances the line/page state.
- After the configured line count is exhausted, execution returns to the scene interpreter.
- `$F0` at that boundary is the scene interpreter's stop command. The same byte has a different substitution meaning if encountered inside the text interpreter.

This layered interpretation explains why control bytes cannot be decoded correctly without tracking the active state machine.

## Glyph encoding

Normal bytes map to 8×16 characters. The renderer stores two tile indices per input byte:

- `Y`: top 8×8 tile
- `A`: bottom 8×8 tile

The Japanese and complete Arabic ROMs use the same mapper code at `$CBC2`; their visible language change comes primarily from replacing CHR font tiles and encoded strings. The Extraordinary version also alters parts of the `$CBC2` mapping for Arabic character ranges.

Generated glyph atlases confirm:

- the Japanese ROM maps the lower byte ranges to kana and Japanese punctuation;
- the complete Arabic ROM uses most of the available byte space for contextual Arabic forms;
- the Extraordinary version uses a smaller Arabic range while retaining more Latin letters and digits.

## Text control table

| Code | Handler | Current interpretation |
| --- | --- | --- |
| `E0–E3` | `$83A4–$8443` | Derive dictionary indices from actor/team match-state fields |
| `E4` | `$8467` | Insert the indexed entity name selected by RAM `$0441` |
| `E5–E6` | `$846D–$8475` | Insert a side/team dictionary string derived from RAM `$05FB`, `$002A`, and `$002B` |
| `E7` | `$848D` | Render the value at RAM `$0600` as a direct numeric glyph (`value + $33`) |
| `E8–EB` | `$8493–$84A5` | Insert indexed entity names from RAM `$0601–$0603` or `$05FC` |
| `EC` | `$84AB` | Insert an optional role/title prefix and a dictionary string selected by match state |
| `ED–EE` | `$84CE–$84D6` | Insert indexed entity names derived from RAM `$05FB` or `$0442` |
| `EF–F0` | `$84DC` | Render a state-dependent numeric glyph derived from RAM `$0616` |
| `F1–F2` | `$84E6–$84EC` | Insert side/team dictionary strings selected by RAM `$002A/$002B` |
| `F3–F4` | `$84FB–$8507` | Insert a pair of entity names with state-dependent grammar/separators |
| `F5` | `$85B1` | Insert dictionary token `$ED` |
| `F6` | `$85B6` | Insert dictionary token `$EE` |
| `F7` | `$85BB` | Read a repeat count and emit tile code `$7C` that many times |
| `F8` | `$85D0` | Insert dictionary token `$EF` |
| `F9–FB` | `$85D5` | No operation / reserved |
| `FC` | `$85D6` | Flush line/page and advance dialogue state |
| `FD` | `$85FD` | No operation / reserved |
| `FE` | `$85FE` | Pause for state/input synchronization |
| `FF` | `$8621` | Terminate the active text state |

## Reusable tools

- `tools/extract_chr.py`: exports each 8 KiB CHR bank as a tile sheet.
- `tools/extract_dialogue.py`: exports all 240 bank-24/25 script records, their raw bytes, aliases, addresses, and control-byte counts as JSON.
- `tools/extract_dictionary.py`: exports the 240-entry fixed-bank substitution dictionary at `$F329` as JSON.
- `tools/extract_script_bank.py`: exports a self-indexed scenario-script bank mapped at `$A000–$BFFF` as JSON.
- `tools/render_text_glyphs.py`: executes the ROM's own `$CBC2` mapper and builds a labeled byte-to-glyph atlas.
- `tools/render_message.py`: reads a pointer-table record and renders its static glyphs, line breaks, repeats, and control placeholders.
- `tools/disassemble_reset.py`: maps MMC3 fixed banks and disassembles the reset path.

## Extracted table dimensions

- The script pointer table contains 240 entries and 236 unique record addresses in all three analyzed ROMs.
- Script records occupy a 16 KiB CPU window spanning consecutive PRG banks 24 and 25 at `$8000–$BFFF`.
- Duplicate pointers are aliases: each alias resolves to the same non-empty record.
- The substitution dictionary contains 240 entries at `$F329`.
- Dictionary entry `$00` points to the runtime buffer at RAM `$05EB`.
- Entries `$01–$EF` point to strings in the fixed bank and stop at the first byte at or above `$E0`; 239 Japanese and Extraordinary entries use `$FC` as that terminator.
- The complete Arabic version intentionally reuses some dictionary pointers: its 240 entries resolve to 198 unique addresses.

## Scenario-script banks

PRG banks 3 through 6 begin with self-indexed pointer tables for larger match and story scripts:

| PRG bank | Entries | Unique records | Japanese vs. complete Arabic pointers | Extraordinary pointers |
| ---: | ---: | ---: | --- | --- |
| 3 | 16 | 16 | Identical | Repacked |
| 4 | 16 | 16 | Identical | Repacked |
| 5 | 64 | 56 | Identical | Repacked |
| 6 | 6 | 6 | Identical | Repacked |

The first pointer gives the table size: `$A020` means 16 entries and `$A080` means 64. Bank 5 contains eight pointer aliases. The complete Arabic release preserves the Japanese record addresses, which indicates that its translated records were constrained to the original bank layout. Extraordinary uses shorter, repacked records and therefore changes its pointer tables.

MMC3 PRG register state is mirrored in zero-page RAM: `$24` selects the bank mapped at `$8000`, and `$25` selects the bank mapped at `$A000`. The fixed-bank routine at `$CE2D` writes both selectors to `$8000/$8001`. The task scheduler stores and restores these values across task switches.

The scenario loader in PRG bank 0 begins at CPU `$8462`. Its range table at `$8AEE` maps scenario IDs to banks:

| Scenario ID | PRG bank | Record index |
| --- | ---: | --- |
| `$00–$0F` | 3 | `ID - $00` |
| `$10–$1F` | 4 | `ID - $10` |
| `$20–$5F` | 5 | `ID - $20` |
| `$60+` | 6 | `ID - $60` for the defined entries |

The loader switches `$25`, dereferences the selected bank's pointer at `$A000 + index × 2`, stores the script pointer in `$4D/$4E`, and records the active script bank in `$56`. An execution trace is still needed to attach human-readable story or match-event names to each scenario ID.

## Next work

1. Identify the dynamic-name tables used by `$E0–$F8` and connect dictionary indices to their callers.
2. Label the scenario IDs with story/match events through an emulator execution trace.
3. Add a Unicode transcription layer for Japanese and both Arabic encodings.
4. Confirm the provisional control meanings with an emulator execution trace.

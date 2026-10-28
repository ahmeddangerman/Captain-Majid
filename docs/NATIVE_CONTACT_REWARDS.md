# Native terminal contact rewards

Player experience now persists in the native phase session. The supported rewarded terminal operation starts before the original field-defender reward caller and continues through the connected clock and possession/recovery tail. It remains a development operation; a playable match, campaign level progression and live presentation timing are unfinished.

## Accounting rules

- Original named types1–34 receive rewards. Type0 and types35+ are ignored by this helper.
- Campaign context chooses a packed mapping to an experience slot and reward-factor group. Global experience has11slots; it is separate from the22field records.
- The explicit reward selector chooses one of eight factors. The observed terminal field defender uses selector2.
- The original arithmetic discards the stage-base high byte before scaling: `((low_byte(base) >> 2) * factor) >> 6`. Native code preserves this observed behavior, including the discontinuity above255.
- Add the byte reward to the selected16-bit total, saturating at65535. No immediate player-level or skill increase is inferred.

For terminal result3, the original calls the reward handler once. For result2, it calls once before ball placement and again through the shared loose-ball handler. The native operation preserves both grants and their individual saturation steps. Added-time resume never repeats them.

## Development usage

```sh
python3 tools/export_phase_session.py /private/reward-entry-ram.bin /private/reward-entry.cmjsession
build/captain-phase-session /private/content-v6.cmjpack /private/reward-entry.cmjsession \
  --rewarded-contact 3 --defender 1 --capture /private/rewarded.cmjsession
```

Interactive equivalent: `rewarded-contact CODE DEFENDER ACTION SUBTYPE`. Existing `contact` remains the post-reward operation and preserves any captured totals. Neither command determines whether an arbitrary contact is a legal game action.

Rewarded operations require private version6 content and captured experience context. Old snapshots do not silently acquire campaign data or zeroed experience. State is calculated before commit; errors leave the current session unchanged. All private snapshots/exports stay outside Git.

## Backward-readable formats

`CMJSHOT6` appends189 private bytes: three campaign offsets,70packed mappings,48factor bytes and34stage-base words. Canonical export size is18,804 bytes. Readers retain versions1–5.

`CMJSESSION2` is exactly289 bytes. Its first264bytes use the same field/clock/pending layout as version1, with the version marker changed. The final25bytes are:

| Section | Bytes |
| --- | ---: |
| Experience presence flag | 1 |
| Campaign and opponent stage | 2 |
| Eleven little-endian experience totals | 22 |

When context is absent, its24bytes must all be zero. Readers reject invalid presence, campaign/stage bounds, truncation, trailing bytes and nonzero unused padding. Version1 remains readable with experience absent; writers at this checkpoint produced version2. `export_phase_session.py --without-experience` explicitly omits that context.

The later [recovery integration](NATIVE_RECOVERY_SESSION.md) advances current writers to session version3, retaining these version1/2 readers and adding optional recovery context. The289-byte evidence below describes this reward checkpoint.

## Evidence and limits

Nine direct original CPU observations returned without skipped calls. They cover ignored types, selectors, campaign mappings, high-base low-byte scaling and saturation. Continuous original terminal traces establish one grant for possession and two for the observed loose-ball branch. Between captured reward-entry and clock-entry states, the represented field/clock difference was only loose-ball placement; original experience changed0→1 or0→1→2.

Normal native rewarded commands produced289-byte saved states identical to the corresponding original final exports:

| Observed path | Shared SHA-256 |
| --- | --- |
| Result3: player control, total1 | `e212159aa2e3cf2ba1259f1e7e2d8621b64234eff84a017b3642db2fce9eeccf` |
| Result2: ball recovery, total2 | `da643858454143804c87fd569f69eb9c362e975a554d136301c8c91a64150216` |

Interactive development usage retained total1 across added-time pause/resume. Old snapshot/content usage remains separate from rewarded operations. Make and CMake production builds completed; no automated tests were added or run for this increment.

These observations use the previously disclosed cooperative CPU/task harness and host frame/display/input acknowledgements. They do not reproduce NMI/audio/controller refresh or a complete match. Match-end rewards, level thresholds, other action reward callers and autonomous recovery/player-control progression remain separate integration work.

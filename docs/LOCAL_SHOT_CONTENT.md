# Local shot data and native laboratory

The native engine now loads shot data from a local, replaceable development pack. Original data is exported only from the user's private verified Japanese ROM. Source control contains the exporter, reader and engine code; no generated packs, ROM tables, graphics, music or text are bundled.

The CLI and [optional native window](NATIVE_SHOT_SCENE.md) share `calculate_shot_scene` and the same explicit scene inputs. The window adds placeholder presentation without changing simulation timing or random state.

## Run

Build with Make (`make`) or CMake (`captain-shot-lab` target). Export a new private pack using the optional analysis dependencies:

```sh
.venv/bin/python tools/export_shot_content.py /path/to/japanese-game.nes \
  /path/to/private-analysis/japanese-shot-v1.cmjpack

./build/captain-shot-lab /path/to/private-analysis/japanese-shot-v1.cmjpack
./build/captain-shot-lab /path/to/private-analysis/japanese-shot-v1.cmjpack --interactive
```

The exporter refuses an existing output path and requires `.cmjpack`. That extension is ignored by Git. Keep packs outside the repository. The native executable needs the pack, not a ROM or Python installation.

Interactive commands are `shoot`, `level N`, `keeper-level N`, `subtype N`, `defenders N`, `keeper-action N`, and `quit`. Levels/subtype are bytes, defender count0..5, keeper action0..3. These are development inputs; legal game-menu move availability is not enforced. Each shot resets the scene rather than pretending to advance a match.

## The scene

- Side0, attacker slot3/type1, keeper slot11/type33, ball(160,100), stamina1000.
- Default levels20, subtype0, two field defenders14/15 with action2 and keeper action1.
- Attacker/field random bytes(100,100), keeper bytes(0,100), distance adjustment45, clear continuation gates.
- Other roster slots use ordinary type1/level20. Generic team selection, AI roster construction and original movement/random scheduling are not imported.
- Field contacts use pack-selected comparison content and preserve power on result1. The keeper and outcome stages receive explicit scene state. Field terminal paths return their current handoff; later field phases are not invented.

These assumptions reproduce useful inputs from the observed original calculation, but changing one scene input does not recalculate the original animation/movement/random schedule. The laboratory is a development interface, not a complete match, graphical PC port or replay of the original outcome scripts.

## Pack format

`CMJSHOT1` is an eight-byte magic/version, followed by32source-payload hash bytes. The source hash records provenance; it is not a hash of the entire pack, an authenticity check, or an online compatibility identifier.

All section counts are unsigned little-endian16-bit except move-list lengths, which are bytes. In order:

1. Shared profile mappings:35ordinary bytes, counted alternate IDs(max221),11generic-slot offsets, counted generic team types(max256).
2. Counted field profiles(1..256),31bytes each; counted keeper profiles(1..256),8bytes each. The original field profile view intentionally uses a24-byte stride while preserving cross-row selectors through30.
3. Attack content:15selectors,5move lists of0..64four-byte records,192curve bytes. Defense content has the same shape.
4. Keeper content:4decoded selectors,4move lists,192curve bytes. Action3's skill selector comes from the original selector index5; move-list selection still uses action3.
5. Field comparison:5attack groups,5defense groups, counted eligibility bytes(1..256), counted variation pairs(1..128),4thresholds.
6. Keeper comparison range/offset,4thresholds,8punch intervals.

The reader bounds every count, rejects missing bytes, unsupported versions and trailing data. Semantically unavailable moves/profiles, zero selected ranges and invalid threshold ordering reject when the existing engine helpers use them. Exported generic team types are empty because runtime team selection is not implemented. Imported field eligibility/variation views include bytes beyond labelled source tables for unusual constructed selections; this does not establish those selections' gameplay legality.

This format is an early development format; incompatible future changes must use a new version. Replacement data can use the same engine types; graphical/music/language pack formats remain separate work.

## Current evidence

Make production targets and a separate CMake production build completed on Linux. A private verified ROM exported an11,767-byte pack. Normal CLI and interactive usage loaded it, calculated two field contacts and keeper strength79/ratio80 with power33, and produced a punch with attacker stamina920. Raising the attacker level to80 with no defenders produced power224 and a goal in a fresh scene. Those preparation/comparison numbers agree with corresponding stored original observations, but no automated differential check or outcome-script equivalence is claimed.

Normal usage exposed an exporter pointer error before completion: keeper selector3 must read from the pointer table beginning `$8D93`, not from its selector1 entry `$8D95`. Direct original instruction inspection identified the cause; the exporter was corrected and its task-generated private pack regenerated. No original content was committed.

No automated tests or suites were added/run for this increment. Windows execution, malformed-pack behavioral coverage, general shot scheduling, persistent saves/replays, rendering and online play remain further work.

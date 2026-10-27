# ROM analysis tools

These scripts inspect user-supplied ROMs without copying ROM data into the repository. Generated PNG and JSON output should stay in a local analysis directory.

## Environment

Python 3.11 or newer is recommended.

```bash
python3 -m venv .venv
.venv/bin/pip install -r tools/requirements.txt
```

## Commands

Replace `game.nes` and output paths with local files.

```bash
# Cartridge reset vectors and startup path
.venv/bin/python tools/disassemble_reset.py game.nes --follow-jumps

# Every 8 KiB CHR bank as a tile sheet
.venv/bin/python tools/extract_chr.py game.nes .analysis/chr

# Byte-to-8×16-glyph mapping used by the game
.venv/bin/python tools/render_text_glyphs.py game.nes .analysis/glyphs.png

# The 240-entry bank-24/25 dialogue table
.venv/bin/python tools/extract_dialogue.py game.nes .analysis/dialogue.json

# The 240-entry fixed-bank substitution dictionary
.venv/bin/python tools/extract_dictionary.py game.nes .analysis/dictionary.json

# One larger scenario-script bank
.venv/bin/python tools/extract_script_bank.py game.nes .analysis/bank-03.json --bank 3

# A rendered static dialogue record
.venv/bin/python tools/render_message.py game.nes .analysis/message.png --index 1
```

The JSON formats include source hashes, original CPU addresses, raw bytes, aliases, and control-byte counts. They are extraction artifacts rather than final translations; dynamic substitutions and runtime state remain explicit.

## Native comparison harnesses

After building the native prototype with `make`, these optional commands execute isolated original CPU routines from a private Japanese ROM and compare every exposed output with the native batch modes:

```bash
.venv/bin/python tools/verify_action_strength.py game.nes
.venv/bin/python tools/verify_defender_contact.py game.nes
.venv/bin/python tools/verify_shot_power_keeper.py game.nes
.venv/bin/python tools/verify_shot_state.py game.nes
.venv/bin/python tools/verify_shot_outcome.py game.nes
.venv/bin/python tools/verify_defender_outcome.py game.nes
.venv/bin/python tools/verify_ball_recovery.py game.nes
```

The state harness matched 25,756 energy, keeper story/state, and random-step cases. Its RAM model mirrors the NES's internal 2 KiB RAM. The goalkeeper outcome harness matched 384 handoff cases; the field defender possession harness matched 144 result-three cases; the ball-recovery harness matched 512 ordinary interior-field cases. The scripts require the verified Japanese payload and do not copy its tables or instructions into the native engine. See [engine progress](../docs/ENGINE_PROGRESS.md) for the current scope and remaining work.

## Banked attacking-stage comparison

`verify_banked_attack.py /path/to/reference.nes` executes original attacker preparation, energy and power including real PRG switching, comparing3,882 constructed cases with the native wrapper. See [BANKED_ATTACK_TRACE.md](../docs/BANKED_ATTACK_TRACE.md). The memory adapter rejects unmodeled hardware; this is not a complete gameplay trace.

`verify_field_comparison_selection.py /path/to/reference.nes` compares original field caller selection and classification in550 constructed fixtures. See [FIELD_COMPARISON_SELECTION.md](../docs/FIELD_COMPARISON_SELECTION.md) for the later result4 boundary.

`verify_field_interruption.py /path/to/reference.nes` checks result4 override, real status reset and initial restart routing in27,904 private-ROM fixtures, with no calls skipped. See [FIELD_INTERRUPTION.md](../docs/FIELD_INTERRUPTION.md).

## Field script parser observation

`trace_field_script.py` executes the original banks16/17 parser from an explicit private RAM snapshot. It emits JSON presentation requests and writes to watched gameplay addresses (shot power is always watched). This is original-code research, not a native comparison or gameplay replay.

```sh
.venv/bin/python tools/trace_field_script.py /path/to/reference.nes \
  --ram /path/to/private-internal-ram.bin --script 7 --watch 0x612 \
  > /path/to/private-analysis/script7-trace.json
```

Internal RAM must be2048bytes. If a path accesses cartridge RAM, supply its8192-byte snapshot with `--cartridge-ram`. Unsupported hardware, absent cartridge data and execution limits stop the trace. Scripts5/6/7 are supported. Host script setup, synthetic stack/registers and acknowledgement of presentation requests are listed explicitly in the report. Rendering, audio, input callbacks, the cooperative scheduler and frame interrupts are omitted. Keep snapshots and generated JSON outside Git. See [FIELD_LOOP_SCHEDULING.md](../docs/FIELD_LOOP_SCHEDULING.md).

The snapshot's mapper shadow `$22` must have PRG mode bit6 clear. Other modes are rejected before execution rather than traced with a different bank arrangement. Host setup also initializes mapper select/shadow `$23` consistently with the original bank helper.

## Selected-defender loop observation

```sh
.venv/bin/python tools/trace_field_loop.py /path/to/reference.nes \
  --ram /path/to/private-field-loop-ram.bin \
  > /path/to/private-analysis/field-loop-trace.json
```

This entry requires an explicit2048-byte snapshot with side0/11, attack action0..4,1..5 selected defenders in `$0601..$060F`, prepared attacking power at `$061C/$061D`, valid player records, clock and random state. Entry `$82F4` is after attacker/pre-loop setup; exit `$83F5` is before result handoff/player movement. Optional cartridge RAM and mapper-mode limits are the same as for the parser tool.

The tool constructs foreground task5 and background task1, then executes real original subroutines/scheduling. It explicitly acknowledges host frames/display queues and supplies repeated A input. It records comparisons, power/status/clock writes and all host acknowledgements. No NMI, PPU/APU timing, rendering, sound or real controller sampling is modeled, and unsupported hardware accesses stop execution. It is original-code research, not a native differential check or a full-game emulator. See [FIELD_LOOP_SCHEDULING.md](../docs/FIELD_LOOP_SCHEDULING.md) for the seven observations that motivated separating prepared comparison from isolated power loss.

Add `--until keeper` to start at `$829D` before attacking preparation and stop at keeper comparison `$856B`. This mode supports action0 and0..5 selected field defenders with actions2/3. It returns named field-terminal/loose-ball/other-continuation boundaries when no keeper is reached. Its stage records expose actual attacker and keeper preparation, stamina, power, randomness and comparison outputs. It stops before keeper story/outcome scripts. See the continuous trace observations in the same document for constructed input details and unsuccessful attempts.

## Private native shot content

`export_shot_content.py /path/to/reference.nes /path/to/private-analysis/japanese.cmjpack` writes an original-data development pack after verifying the Japanese payload. Existing outputs are refused. Packs are ignored by Git and should remain outside the repository. Run the native `captain-shot-lab` with that pack; it requires no runtime ROM/Python dependency. See [LOCAL_SHOT_CONTENT.md](../docs/LOCAL_SHOT_CONTENT.md) for format, scene assumptions and remaining work. This imports statistics/moves/comparison content, not graphics, music, dialogue or a team/campaign.

The exporter now writes `CMJSHOT2`, appending original approach-distance arrays. The native reader also accepts old version1 packs. Continuous keeper traces include approach checkpoints and final ball coordinates; stage random inputs remain explicit. See [SHOT_APPROACH.md](../docs/SHOT_APPROACH.md).

The loop report now also includes `keeper_recovery`: elapsed input and before/after keeper/shared state at each actual `$D235` call and `$D1A0` return. These are observed original calls; the tool does not inject a native recovery schedule. See [KEEPER_RECOVERY.md](../docs/KEEPER_RECOVERY.md).

Use `--until outcome --ack-ppu-transfers` to continue through the immediate keeper outcome handoff. The opt-in host policy acknowledges one pending `$0498` graphics descriptor per modeled frame and records that write; it does not execute PPU transfers or NMI/audio/random effects. It avoids the queue overflow that stalled the long goal observation. Outcome mode distinguishes actual handoffs from pre-keeper diversions and errors. See [CONTINUOUS_SHOT_OUTCOMES.md](../docs/CONTINUOUS_SHOT_OUTCOMES.md) for four original observations and remaining native timing/state gaps.

`--host-buttons N` supplies the same explicit held/edge mask each modeled frame (byte0..255, default128=A). Mask129=A+Right supports the observed side11 keeper catch menu. The report discloses the mask and separately records later `flight_contacts`, including result/candidate loss and power after each handler returns. See [MIRRORED_SHOT_SCENE.md](../docs/MIRRORED_SHOT_SCENE.md) for controlled and crowded original observations; live-flight selection/scheduling is not yet connected in the native scene.

The exporter now writes `CMJSHOT3`, adding16bytes for the two supported action0 flight variants after version2 geometry. The reader retains versions1/2. An explicit native burst now calculates those contacts and feeds remaining power to the keeper; automatic flight selection/scheduling and terminal handler effects remain separate. See [FLIGHT_CONTACTS.md](../docs/FLIGHT_CONTACTS.md).

Shot modes now also return witnessed `flight_player_control_handoff`/`flight_ball_recovery_handoff` boundaries before a keeper is reached. `terminal_flight` separates comparison-entry state from actual handoff state; no keeper outcome is invented. See [TERMINAL_FLIGHT_TRACES.md](../docs/TERMINAL_FLIGHT_TRACES.md) for two continuous original observations and the still-pending native handler work.

The native `--field-axis-batch`/`fieldaxis` query now exposes raw16-bit field coordinate addition with original high-byte-only bounds. This does not advance the ordered fixture or implement a complete player step. See [PLAYER_MOVEMENT_FOUNDATION.md](../docs/PLAYER_MOVEMENT_FOUNDATION.md) for original observations and movement dependencies.

`--player-movement-batch`/`playermove` exposes the joined player target-direction/stepping calculation with caller-supplied geometry. The private exporter now writes `CMJSHOT4`, adding movement profiles and formation/tactic geometry. `export_field_state.py RAM NEW.cmjstate` packs an explicit checkpoint for the persistent native `captain-field-lab`. See [NATIVE_FIELD_PHASE.md](../docs/NATIVE_FIELD_PHASE.md) for the supported phase, prepared terminal tails, format and evidence limits. Readers retain older content versions; no generated pack/state belongs in Git.

Current exporter format is `CMJSHOT5`, adding35 contact elapsed/status bytes. `export_phase_session.py RAM NEW.cmjsession` captures field state plus clock/shared recovery and an explicit phase. Use `--phase 3 --pending CODE DEFENDER ACTION SUBTYPE` only at an added-time boundary. The `captain-phase-session` laboratory joins supported terminal contacts to this persistent state; `--match-clock-batch`/`clockstep` exposes clock arithmetic/events separately. See [NATIVE_CONTACT_CLOCK.md](../docs/NATIVE_CONTACT_CLOCK.md) for ordering, usage, format and evidence limits.

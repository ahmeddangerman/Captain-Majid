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

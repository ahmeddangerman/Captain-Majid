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

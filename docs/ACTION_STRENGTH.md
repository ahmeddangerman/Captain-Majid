# Action strength: first native behavior contract

Source: user-supplied Japanese ROM, PRG bank 26 at CPU `$8000` and bank 27 at `$A000`; fixed banks 30/31 at `$C000/$E000`.

The external research repository at commit `ec03cf06e6bdae0bcd232e3a423e9d259757dc64` helped locate the routine. Its instructions were then read directly from our ROM. No external source or comments are incorporated into the implementation.

## Confirmed arithmetic

Run the original code from CPU `$819F` until `$81B8`, before its continuation into other gameplay logic.

- Input power is a 16-bit little-endian word at RAM `$061C/$061D`.
- Input loss is one unsigned byte at RAM `$0619`.
- Subtract loss from power with 16-bit wraparound.
- If bit 15 of the resulting word is set, write zero; otherwise keep the result.
- Output replaces RAM `$061C/$061D`.

For ordinary game values below 32768 this behaves like subtraction clamped at zero. The sign-bit rule is part of the observed behavior and is preserved for synthetic boundary inputs too.

Examples: `300 - 37 = 263`; `256 - 1 = 255`; `20 - 37 -> 0`; `32768 - 1 = 32767`; `65535 - 0 -> 0`.

## Scope and uncertainty

This is a contact-loss stage in action resolution, not a complete shot formula. Other code determines when to apply this loss and how much loss to store. Player stats, energy, random generation, goalkeeper outcomes, goals, clock changes, and animation are outside this routine. No timing or random generator is inferred from this arithmetic.

The comparison harness executes an isolated routine using a real 6502 interpreter. It is not a full NES emulator or evidence of complete in-game shot fidelity.

## Comparison

`tools/verify_action_strength.py` checks the ROM's PRG+CHR SHA-256 before execution, generates boundary and seeded sample inputs, executes original instructions, and compares the resulting power with the native executable's batch arithmetic mode. It checks the complete 16-bit output, not a hash or rounded display value. Generated cases stay in memory.

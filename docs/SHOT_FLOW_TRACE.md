# Shot flow trace from the Japanese ROM

This is a direct instruction trace from the verified Japanese ROM. The external disassembly was used only to locate routines; the notes below were checked against the ROM bytes with a 6502 disassembler.

## Confirmed order

1. The action caller at `$8298` selects action data and runs the energy/action-power helper beginning at `$8F72`.
2. With no selected defenders (`$0600 == 0`), it stores result code 0 at `$82AA` and enters the next result dispatcher.
3. With defenders, it iterates the records in `$0601...` at `$82B6–$82EF`. Each selected defender runs through `$8349`, which invokes energy handling at `$8FF3`, chooses action/defender threshold data, then calls the comparison at `$8EE9`.
4. The result code is stored at `$8148`. Codes 0 and 1 advance to the next selected defender at `$83BC–$83D1`; codes 2 and 3 leave the loop and enter the result dispatcher at `$83F5`.
5. The keeper comparison is a separate later route at `$854A–$856B`, followed by the story override and keeper result handlers. It is not safe to apply it after every field-defender comparison.

## Integration boundary

The native `calculate_defender_contact` reproduces one comparison and its code-1 power loss. `resolve_field_contacts` now applies caller-selected contacts in order, carries forward power and subtype, continues on codes 0/1, and stops on codes 2/3. The existing direct `resolve_explicit_shot` path still assumes a keeper attempt; the field loop is a separate stage until the stopped branches and original selectors are integrated.

The `$83F5` indirect table sends codes 0/1 to `$8408`, code 2 to `$843F`, and code 3 to `$844B`.

- Codes 0/1 enter the selected action continuation at `$8408` and may then dispatch to additional mode-specific routines.
- Code 2 runs the defender experience helper and goes through `$8BDF`. That routine adjusts ball coordinates and runs field recovery. Possession is chosen later, so the immediate code-2 handler leaves it unchanged.
- Code 3 awards defender experience, then branches by the selected defender. Defender IDs 0 and 11 are keepers and transfer possession directly to that keeper's side. A field defender transfers possession when defense action is 2. Other field-defense actions enter a separate animation/continuation path without an immediate possession write.

The native `resolve_defender_outcome` now records these handoffs. Its immediate code-3 possession writes matched 144 original-ROM cases. It does not predict the later result of code-2 ball recovery or the other code-3 animation path. Defender selection and threshold/variation-table loading remain explicit inputs until their original selectors are reconstructed.

## Evidence boundary

This trace establishes call order and loop exits. It does not yet reproduce the complete screen/action setup, content selection, event messages, rewards, animations, ball recovery, match clock, restart logic, or a full gameplay trace. The native TCP fixture also remains a contact-loss demo rather than a match session.

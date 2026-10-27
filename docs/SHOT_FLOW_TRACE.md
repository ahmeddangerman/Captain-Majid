# Shot flow trace from the Japanese ROM

This is a direct instruction trace from the verified Japanese ROM. The external disassembly was used only to locate routines; the notes below were checked against the ROM bytes with a 6502 disassembler.

## Confirmed order

1. The action caller at `$8298` selects action data and runs the energy/action-power helper beginning at `$8F72`.
2. With no selected defenders (`$0600 == 0`), it stores result code 0 at `$82AA` and enters the next result dispatcher.
3. With defenders, it iterates the records in `$0601...` at `$82B6–$82EF`. Each selected defender runs through `$8349`, which invokes energy handling at `$8FF3`, chooses action/defender threshold data, then calls the comparison at `$8EE9`.
4. The result code is stored at `$8148`. Codes 0 and 1 advance to the next selected defender at `$83BC–$83D1`; codes 2 and 3 leave the loop and enter the result dispatcher at `$83F5`.
5. The keeper comparison is a separate later route at `$854A–$856B`, followed by the story override and keeper result handlers. It is not safe to apply it after every field-defender comparison.

## Integration boundary

The native `calculate_defender_contact` reproduces one comparison and its code-1 power loss. It does not yet select a defender record, apply the loop's roster/action rules, or resolve defender codes 2/3 into match consequences. The current `resolve_explicit_shot` calculates energy, action power, keeper comparison, and goalkeeper outcome, but assumes a direct keeper attempt.

The next faithful step is a shot-flow controller that accepts already selected, ordered defender records, applies contacts until the original loop stops, and only calls the keeper path when the original flow reaches it. Defender selection and threshold/variation-table loading remain explicit inputs until their original selectors are reconstructed. Codes 2/3 must be traced through their own result handlers before they can continue to goals or saves.

## Evidence boundary

This trace establishes call order and loop exits. It does not yet reproduce the complete screen/action setup, content selection, event messages, rewards, animations, ball recovery, match clock, restart logic, or a full gameplay trace. The native TCP fixture also remains a contact-loss demo rather than a match session.

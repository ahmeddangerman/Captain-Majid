# Defender contact calculation

## Evidence and boundary

This contract comes from instructions executed directly in the verified Japanese ROM: PRG banks 26/27 at `$8000/$A000`, fixed banks 30/31 at `$C000/$E000`. The external disassembly remains a locator/reference only; its source and comments are not incorporated.

The experiment executes `$8EE9` to its return, which calls `$8D06`, followed by `$8132` to its return. A continuation at `$819F–$81B8` supplies the already verified loss stage when the classification is 1. Rewards, ball ownership, animation, clock changes, foul overrides, and goalkeeper actions remain outside this experiment.

## Caller-specific power handling

`compare_defender_contact` now returns arithmetic/classification and the candidate loss without changing incoming power. Prepared selected-defender loops use this API. `calculate_defender_contact` retains the isolated result1 loss continuation described below; existing batch and fixture callers keep that behavior. The original selected-defender loop at `$82F4..$83F5` does not invoke this isolated loss continuation in the observed scheduler paths. See [FIELD_LOOP_SCHEDULING.md](FIELD_LOOP_SCHEDULING.md). The two callers must not be conflated.

## Inputs

- Incoming power: word at `$061C/$061D`.
- Defender base power: word at `$32/$33`.
- Two existing random bytes: `$E2/$E3`. Their generation and scheduling are not inferred here.
- Variation range and offset: a pair selected from the table referenced by `$8D93`. The native API accepts the pair as content parameters. No ROM tables are bundled.
- Weakened flag: bit 7 at `$3A`.
- Eligibility flag: the negative flag at entry to `$8EE9`, preserved by `$8D06`.
- Context, defender slot, and subtype: `$0621`, `$0442`, `$043E`.
- Four classification thresholds: the selected row at `$828C`. This experiment covers the three field-defender rows (actions 0–2), each descending and ending at zero. Other actions require different callers and are outside this contract.

## Confirmed behavior

1. Average the two random bytes using the full 9-bit sum. For selectors 0–10 the preceding shift supplies no additional carry.
2. A special branch applies when eligible, not weakened, and either the context is 4 with defender slot 0/11, or the second random byte is at least 248. It sets subtype bit 7 and uses multiplier `256 + (average & 127)`.
3. Otherwise repeatedly subtract the range while the sample is strictly greater than the range, then add the offset. Equality remains at the upper bound rather than wrapping to zero. Zero ranges are excluded from the native API because the original repeated-subtraction path may never terminate.
4. If weakened, divide the defender base power by 16, rounding down. This changes the stored base power as well as the calculation.
5. Multiply adjusted base power by the multiplier. Take product byte 1 unless product byte 2 is nonzero, in which case use 255. Product byte 3 is not checked by this routine; preserve that detail for synthetic large inputs.
6. Calculated contact loss is the resulting strength divided by 4, rounded down.
7. The comparison score is `min(255, floor(((incoming_power * 64) modulo 65536) / strength))`. With strength zero, the original divider produces 65535, which becomes score 255.
8. The result code is the first threshold index for which score is greater than or equal to that threshold. Equality is significant.
9. Result code 1 invokes the contact-loss arithmetic. Other codes leave the incoming power unchanged in this isolated experiment; their complete gameplay consequences are not implemented here.

## Validation

The optional ROM comparison reads actual coefficient/threshold pairs from the user's ROM, runs original instructions under py65, and compares all eight exposed outputs with the native implementation: adjusted defender base, strength, loss, score, classification, power after the isolated loss stage, subtype, and special-branch flag.

The terminal `--defender-batch` mode accepts 15 unsigned decimal fields per line: incoming power, base power, random byte 1, random byte 2, range, offset, weakened (0/1), eligible (0/1), context, defender slot, subtype, then four thresholds. It returns the eight outputs above. Inputs are research fixtures, not player commands or a trusted online protocol.

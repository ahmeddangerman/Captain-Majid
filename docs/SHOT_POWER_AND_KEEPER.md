# Standard skill, action power, and goalkeeper comparison

## Evidence and scope

These contracts come from the verified Japanese ROM, inspected and executed locally. The external disassembly is used to locate functions, without incorporating its source, comments, or content. Original tables are supplied privately by the ROM verifier; the engine ships none of them.

This increment covers three calculations. It does not yet produce a playable full shot: stats/content selection, energy accounting, random scheduling, distance effects, field contacts, story overrides, and the final ball/score/animation changes must still be integrated.

## Standard skill path

PRG banks 28/29 contain the player skill system. Player profiles select skill data; generic opponents can use team/position profiles. The standard non-stamina path at `$80F9–$8112` calculates an index:

1. Double the byte-sized level, preserving the carry from that operation.
2. Add that doubled byte and its carry to the base skill byte, retaining the low byte of the sum.
3. Limit the resulting index to 191.
4. The action preparation path at `$8278–$828E` adds a move modifier, wraps to a byte, limits to 191 again, and reads the strength from the lookup table at `$9E4E`.

The native `standard_skill_index(base, level, modifier)` implements the two index adjustments. Lookup values remain data supplied by the eventual content layer. Stamina, player/profile selection, special skills, and selecting the right skill for ball height are outside this helper.

## Action power

PRG bank 26, entry `$8F9A`, return at `$8FF2`, takes base power at `$32/$33`, two existing random bytes at `$E2/$E3`, weakened bit 7 at `$3A`, the attack subtype at `$043C`, and the incoming CPU carry flag.

- If not weakened and the first random byte is less than 8, set subtype bit 7.
- When not weakened, the first random-byte comparison supplies carry (1 for byte >= 8). When weakened, the routine keeps the incoming carry. The caller must supply that value until the energy path is mapped.
- Form `sum = random_first + random_second + carry`. Average and round using the low bit of this sum.
- Ordinary multiplier: `(floor(sum / 2) OR 128) + (sum & 1)`.
- Subtype bit 7 multiplier: `256 + (floor(sum / 2) AND 127) + (sum & 1)`.
- If weakened, divide base power by 16, rounding down, and store that adjusted base.
- Output action power is bits 8–23 of adjusted base times multiplier, preserving original truncation.

Outputs are adjusted base, multiplier, action power, and updated subtype. This helper starts after stat/energy preparation, not before it.

## Goalkeeper comparison

The experiment starts at `$854A` after the caller selects a critical threshold, then runs through the comparison loop to `$856B`. It uses the shared variation routine `$8D06` via `$8F1F` and goalkeeper state adjustment `$8F59`.

- Eligibility for the special variation branch is `random_first > critical_threshold`; equality does not qualify.
- The shared defender variation supplies adjusted keeper power, rolled strength, and subtype/special flags. Weakening suppresses that special branch.
- Comparison numerator is the low 16 bits of incoming power times 192.
- Divide by rolled strength; a zero strength produces the original divider result 65535. Limit the ratio to 255.
- State adjustment is `min(255, max(stored_timer - elapsed, 0) + stored_penalty)`.
- Add that adjustment to the ratio, limiting the total to 255.
- The classification is the first of four descending thresholds that the score reaches; the last threshold must be zero.

The original caller selects thresholds based on goalkeeper identity; these remain explicit content parameters in the native helper. The experiment stops before story-specific overrides and result consequences. Numeric classification codes must not be described as a final goal/save decision until those later branches are integrated.

## Research interfaces

- `--skill-index-batch`: three bytes per line: base skill, level, move modifier; returns the adjusted index.
- `--action-power-batch`: base power (word), random bytes 1/2, weakened (0/1), attack subtype, incoming carry (0/1); returns adjusted base, multiplier, action power, updated subtype.
- `--keeper-batch`: incoming power, keeper base power, random bytes 1/2, variation range/offset, weakened (0/1), critical threshold, context, defender slot, subtype, stored timer, elapsed, stored penalty, then four thresholds. Returns adjusted keeper base, rolled strength, ratio, state adjustment, score, result code, subtype, special flag.

The same action/keeper research calculations can be queried through the loopback replica adapter without altering its ordered contact-loss fixture.

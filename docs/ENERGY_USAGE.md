# Energy use

## Evidence and boundary

This contract was derived from instructions in the verified Japanese ROM, with PRG bank 26 at `$8000` and fixed banks 30/31 at `$C000/$E000`. The external disassembly served as a locator only. Its source, comments, and assets are not incorporated into the native implementation.

The complete isolated function begins at `$8FFB` and returns through the `RTS` at `$904D`. Its optional wrapper begins at `$8FF3`; it either returns at `$8FFA` or enters the complete function. Neither entry calls another subroutine. An oracle can install a synthetic return address for `$0200` and execute either entry until that address is reached. Decimal arithmetic must be disabled in py65, matching the NES CPU's binary arithmetic.

The following routine at `$904E` handles a separate flag and opposing player records. It is outside this energy deduction contract. Action costs selected from content, player selection, passive running cost, recovery, action power, animation, and match events are also outside this function.

## Inputs and outputs

| Native input | Original state | Meaning |
| --- | --- | --- |
| `stamina` | Word at player pointer +1/+2 | Current energy, little endian |
| `cost` | `$043F/$0440` | Requested cost, little endian |
| `player_id` | Byte at player pointer +0 | Identity tested against `$20` |
| `side` | `$05FB` | Zero or nonzero side context |
| `action` | `$043B` | Attack action, tested against zero |
| `subtype` | `$043C` | Raw attack subtype byte |
| `skip_home_side` | Choice of entry | True selects the `$8FF3` wrapper |
| `incoming_carry` | CPU carry at entry | Preserved only when the wrapper skips deduction |

The original player pointer is `$34/$35`. Normal player records are distinct from the input globals: the original pointer table at `$CD89` maps slots 0–21 to `$0300 + 12 * slot`. The native API operates on values and does not reproduce arbitrary pointer aliasing.

`calculate_energy` returns the resulting stamina, effective cost, weakened state, and carry. The original weak marker is `$3A`: exactly zero or `$80`, replacing its prior contents. The function writes stamina back to player +1/+2, and writes the cost word only when applying the surcharge. The wrapper's skipped path writes only `$3A=0`; it leaves stamina and cost unchanged.

## Confirmed arithmetic

1. If the wrapper is selected and `side == 0`, return the input stamina and cost, `weakened=false`, and the incoming carry.
2. Otherwise, player ID `$20` increases the cost unless all three exemption conditions hold: `side == 0`, `action == 0`, and the raw `subtype >= 3`.
3. The increased cost is `(cost + floor(cost / 2)) modulo 65536`. The raw subtype comparison includes bit 7; there is no subtype mask here.
4. Compute `difference = (stamina - effective_cost) modulo 65536`.
5. If bit 15 of the difference is set, return zero stamina and `weakened=true`. Otherwise return the difference and `weakened=false`.
6. Carry is clear on the weakened path. Otherwise it is the unsigned comparison `stamina >= effective_cost`.

The sign test at `$903E` is significant. For ordinary game values below 32768 this behaves as a subtraction clamped to zero. Across all possible 16-bit fixtures it differs from an unsigned underflow test:

| Stamina | Effective cost | Result stamina | Weakened | Carry |
| ---: | ---: | ---: | --- | --- |
| 80 | 80 | 0 | false | true |
| 79 | 80 | 0 | true | false |
| 0 | 32768 | 0 | true | false |
| 0 | 32769 | 32767 | false | false |
| 0 | 65535 | 1 | false | false |
| 32768 | 0 | 0 | true | false |

The surcharge also wraps: an initial cost of 43690 becomes 65535; 43691 becomes zero; 65535 becomes 32766. An initial cost of 1 stays 1, and 3 becomes 4.

## Instruction checkpoints and flags

`$8FF3–$8FFA` performs the optional side check. `$8FFB–$9016` resets the weak marker and checks the surcharge exemption. `$9018–$902C` computes and stores the increased cost. `$902F–$903B` subtracts the effective cost. `$903E–$9045` tests the high result byte's sign and constructs the weak marker. `$9047–$904D` writes the final stamina and returns.

On a direct return, `A` and `X` equal the low byte of the resulting stamina, and `Y=1`. Carry is described above. Negative and zero reflect that final low byte; they do not encode the complete 16-bit result. Overflow reflects the high-byte subtraction and survives the weak clamp. The other status bits are preserved. Entry `A`, `X`, `Y`, and carry do not affect this path.

On the wrapper's skipped return, `A=0`, `X/Y` are unchanged, negative is clear, zero is set, and carry/overflow are unchanged. This is why incoming carry is part of the native input even though the direct deduction ignores it.

The original arithmetic explicitly sets carry before the low-byte subtraction. If weakening occurs, it sets carry and rotates the cleared weak byte, storing `$80` and leaving carry clear. A clear returned carry can therefore coexist with a nonzero resulting stamina when the subtraction wraps into a difference below 32768.

## Callers and validation

The attacking action caller at `$8F72` requests action data, clears `$3A`, and only debits for side zero. When its attack action is 2 and `$0600` is zero, it replaces both cost bytes with zero before calling `$8FFB` at `$8F97`. The following action power code reads `$3A` to apply weakening. That caller's cost override is deliberately outside `calculate_energy`.

Calls to the side wrapper occur at `$80B6`, `$8349`, `$8534`, `$8737`, and `$8A02`, after selecting defensive action data. A direct call at `$8214` follows an explicit cost of 80 and a player selection through `$C50C`. These addresses and the energy function's instructions were checked directly against the Japanese ROM; interpretation of names in the external locator is not required by the native API.

Before implementation, an independent arithmetic model was compared with original instruction execution for 291,312 boundary combinations and 12,000 deterministic random fixtures. All outputs matched, including resulting stamina, cost, weak marker, and returned `A/X/Y/P`. Those exploratory checks do not substitute for the repository's native ROM comparison.

The native unit tests use literal expected outputs for ordinary subtraction, exact exhaustion, the raw subtype boundary, player and side checks, surcharge rounding and overflow, skipped carry preservation, and the sign-test counterexamples. They can be compiled independently with:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -I engine/include \
    engine/tests/energy_tests.cpp engine/src/energy.cpp -o /tmp/captain-energy-tests
/tmp/captain-energy-tests
```

## Research interface

`--energy-batch` accepts eight unsigned fields per line: stamina, cost, player ID, side, action, raw subtype, skip-home wrapper (0/1), and incoming carry (0/1). Stamina and cost are 16-bit values; the four context fields are bytes. It returns stamina, effective cost, weakened (0/1), and outgoing carry (0/1).

The replica command is `energy` followed by the same fields. It evaluates an explicit fixture without updating the ordered session. The prepared private-ROM comparison in `tools/verify_shot_state.py` contains 9,728 energy cases. Its native comparison result is pending.

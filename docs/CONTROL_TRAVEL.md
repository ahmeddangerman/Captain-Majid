# Explicit control-travel step

The original control task updates the two-byte travel accumulator at `$E3CA..$E406`. The native `advance_control_travel` function represents the input gate and arithmetic after the caller has supplied the speed loaded by the original `$CE08` routine. It accepts field side 0 or 11. For side 0 with no direction held, it leaves travel unchanged. Otherwise, it adds one quarter of the supplied 16-bit speed when facing bit 6 is set, or adds the 16-bit one's complement of that quarter when the bit is clear. Addition wraps at 16 bits.

`advance_session_control_travel` applies that step to a control-ready saved session. The CLI exposes it as:

```text
captain-phase-session PRIVATE_PACK PRIVATE_SESSION \
  --advance-control-travel HELD SPEED_LOW SPEED_HIGH --capture PRIVATE_OUTPUT
```

All three arguments are bytes. The caller supplies the loaded speed and the held input that apply to that observed step. The operation preserves the session phase and other represented fields. It does not poll input, load speed, determine when the task runs, advance the game clock or random state, emulate audio or presentation, or prove that a frame has completed. Apply it only where an original-backed caller provides those inputs and ordering.

`tools/verify_control_travel_original.py PRIVATE_ROM --binary PATH_TO_CONTROL_TRAVEL_TESTS --exhaustive` compares the original gate and arithmetic against the native kernel for every supplied 16-bit speed in both facing directions, plus boundary combinations. The oracle deliberately substitutes the speed-loader result; it does not validate the original speed loader or live scheduling. In one original-emulator startup, 52 instruction-boundary calls across four directional probes and 36 poll-plus-travel comparisons matched their represented non-random fields. These share one starting position and player and are not general match coverage.

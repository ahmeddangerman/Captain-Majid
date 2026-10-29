# Numeric ball movement to the next cell

advance_ball_to_next_cell reproduces prepared originalE778 coordinate advancement
with supplied16-bit X/Y coordinates and displacements. It repeats additions within
the same frame until the current cell changes. A new cell stops before800F;
arrival stops before800C after snapping only high bytes to destination center.
Original fractional low bytes survive that snap. Outside-field movement forces
the destination instead of clamping to the player boundary. Already-at-destination
returns untouched, corresponding to E746's initial check.

Only coherent interior current cell/coordinates and destination0..239 are
supported. A stationary nonarrival rejects rather than looping forever;65536
additions bound a complete16-bit coordinate cycle. Inputs are copied and errors
are atomic. No current session snapshot or live frame/callback is committed by
this pure helper. Fractional state now connects through the first callback boundary; see NATIVE_SAVED_BALL_FLIGHT.md.

Real regressions failed with the unimplemented stub, then passed repeated subcell
addition, fractional negative borrow, snap preservation, outside fallback, no-op
arrival and invalid/stationary inputs. Full Make suite/build pass cleanly. CMake
updated but unavailable in executor. Fresh static review found no material issues. 326 original/native cases (six explicit
adversarial cases,64deterministic constructed inputs and all256directions using
the native velocity projection composed with stepping) match x/y, current cell,
addition count, callback boundary and forced-destination flag:

```
PYTHONPATH=tools python3 tools/check_ball_step_original.py \
 /private/Japanese-base.nes /private/new-output-directory
```

The actual built core is linked into a tiny private test driver. Original calls
execute without skips or frames; input velocity/fractions are explicit prepared
values, not a simulated complete trajectory. Private report:
../Captain-Majid-analysis/ball-step-repeatable. Ball direction/projection evidence
is separate in NATIVE_BALL_VELOCITY.md. Next: connect direction, velocity, fractional
coordinates and saved cell/arrival callbacks into real shot-flight scheduling.

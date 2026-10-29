# Ball velocity projection

calculate_ball_velocity reproduces fixed originalCE4A/CE4D signed16-bit X/Y
projection for every direction byte. It uses private replacement64-word
quarter-wave content, doubles/mirrors/masks the original index, preserves word
negation wrap and adds64mod256 only for the horizontal axis. It does not use the
coarser player movement signs or clamp ball movement to player boundaries.
No coordinates, clocks, stamina, random values or session phases change.

Synthetic quadrant/boundary/zero/signed-minimum tests failed with the stub, then
passed. Full default Make suite/build pass cleanly. Fresh review found no material
issue. All256original direction bytes match both native displacement words:

```
PYTHONPATH=tools python3 tools/check_ball_velocity_original.py \
 /private/Japanese-base.nes /private/new-output-directory
```

The checker compiles a tiny private native driver against the actual built core,
supplies the approved ROM's projection table privately, executes originalCE4A and
CE4D without skipped calls/frames, and saves every comparison. Private evidence:
../Captain-Majid-analysis/ball-velocity-repeatable. No table/ROM is committed.
CMake updated but executor lacks CMake. Complete ball flight remains unconnected.

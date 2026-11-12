# Provisional match development window

This Windows-only window is an early way to inspect and control supported parts of the native runtime. It starts from an explicitly supplied development snapshot; it is **not a fresh match, a complete playable match, or verified original timing**. It uses placeholder graphics, simplified timing/random values and automatic acknowledgements of presentation requests. Unimplemented paths stop visibly with the last valid state intact.

Build from the repository root with the already configured Windows CMake environment:

```powershell
cmake --build build --target captain-match-window -j 2
```

If you have a known compatible original control-ready RAM capture, the launcher derives its score, ball fractions and retained action from that capture, writes its temporary session outside the checkout, and opens the window:

```powershell
python tools/launch_development_match.py --ram PRIVATE_READY_RAM --pack PRIVATE_PACK `
  --distance PRIVATE_DISTANCE_TABLE --velocity PRIVATE_VELOCITY_TABLE `
  --private-temp PRIVATE_DIRECTORY
```

The launcher cannot prove that arbitrary RAM was captured at the control-ready boundary: use a checkpoint whose provenance is known. It waits until the window closes before removing its temporary session. All inputs and the temporary directory must be outside the checkout.

Supply private files from outside the checkout. The content pack must contain the appropriate opponent team and optional tables needed by the desired routes. The S format adds conservative gates for supported opponent ordinary shots; R and Q packs remain readable but stop on those decisions. Do not treat the following example values as defaults: `X`, `Y`, `ACTION`, `LEFT`, `RIGHT` and `KICKOFF` are the corresponding known bytes from the same original snapshot, and the seed is an explicitly provisional random seed.

```powershell
.\build\captain-match-window.exe PRIVATE_PACK PRIVATE_SESSION `
  --distance PRIVATE_DISTANCE_TABLE --velocity PRIVATE_VELOCITY_TABLE `
  --seed 1 --ball-fractions X Y --retained-defense-action ACTION `
  --score LEFT RIGHT KICKOFF
```

The optional `--ball-fractions`, `--retained-defense-action` and `--score` arguments must only be provided when their values are actually known. Omit them when unknown: the window shows `?` for an unknown score and the runtime reports missing metadata when a route needs it. Explicit retained action is accepted only at a supported control-ready or unopposed shot boundary. A snapshot already containing a score rejects `--score`. The displayed clock is the runtime's raw clock value, not a promise of regulation minutes.

The window begins paused. Press **Space** to run or pause, the **arrow keys** to move or choose, **X** to open the action menu or go back, and **Z** to confirm. Losing window focus pauses play and clears held keys. The current supported attack choices are ordinary shot and pass; the development view identifies other options as unsupported. It renders outfield-player positions, the runtime ball, the pass cursor and available selections; original sprites, animation, sound and live PPU/NMI scheduling are not implemented.

Some opponent cell decisions now continue through an ordinary pass or shot, including one or two nonterminal field-defender attempts on a shot. With a T pack, the home goalkeeper menu accepts ordinary catch and punch choices, and supported goals continue through home kickoff to control. Other decisions, special moves, terminal blocks and redirects, and goalkeeper save continuations may still stop; this remains a development snapshot rather than a complete match.

`tools/export_shot_content.py ROM PRIVATE_PACK --team-id N --include-cell-shots` creates the optional S pack for explicitly selected opposing team 3..6. `tools/export_pass_distance.py`, `tools/export_ball_velocity.py` and `tools/export_phase_session.py` can create the other private development files from verified local inputs. Preserve original RAM, ROM and derived output outside the Git checkout. The RAM used for a phase-9 export must be an actual compatible control-ready checkpoint, not an arbitrary snapshot.

The window and its input adapter build and automated runtime checks pass on the current Windows machine. Off-screen GDI renders of control and action-menu snapshots were visually inspected, and a launched window appeared as a responding process. The Windows computer-use helper was unavailable during this increment, so actual window appearance, keyboard interaction and focus behavior have **not** yet been verified through live UI interaction. The broader M1 possession milestone, M2 interactive acceptance, fresh startup, many action/outcome routes and a complete match remain unfinished.

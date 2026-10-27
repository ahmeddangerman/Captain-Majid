# Optional native shot scene

`captain-shot-scene` is an SDL2 presentation of the same resettable development scene as the command-line laboratory. It loads a private local `.cmjpack` and calls the shared `calculate_shot_scene` API when a shot is requested. The schematic field, players, ball path and simple pixel lettering are placeholders made for this project; no original game artwork/font is included.

## Build and launch

Install SDL2 development files using your platform's normal package manager or SDK. SDL2 version2.0.18or newer and its CMake config package are required. It is optional; normal Make/CMake core and CLI builds have no SDL2 dependency.

```sh
cmake -S . -B build-ui -DBUILD_TESTING=OFF -DCAPTAIN_BUILD_SDL_SCENE=ON
cmake --build build-ui --target captain-shot-scene
./build-ui/captain-shot-scene /path/to/private-analysis/japanese-shot-v1.cmjpack
```

For a nonstandard SDK, pass `-DCMAKE_PREFIX_PATH=/path/to/SDL2`. Multi-configuration generators may put the executable in a configuration subdirectory. Windows execution and deployment remain unverified.

## Controls

| Key | Action |
| --- | --- |
| Space | Calculate a new shot from the reset scene |
| Up / Down | Raise / lower attacker level |
| Right / Left | Raise / lower keeper level |
| D | Cycle zero through five field defenders |
| R | Clear the displayed result |
| Escape / window close | Exit |

Repeated keydown events are ignored. Changing settings clears the prior result so the displayed outcome cannot silently belong to different options. The window uses a640×480logical canvas, integer scaling and letterboxing. Resizing and display frame rate do not calculate shots, consume random bytes or alter results. The ball animation consumes an already-calculated result and a presentation clock only.

Each shot resets. The scene has explicit fixture positions, random bytes, distance/route inputs and ordinary player types documented in [LOCAL_SHOT_CONTENT.md](LOCAL_SHOT_CONTENT.md). It is not a complete match, faithful original animation system, final interface or online game. The preview's schematic player/ball locations do not represent an imported original field layout. Shader, original asset, Arabic UI and audio support remain future work.

## Headless capture

```sh
./build-ui/captain-shot-scene /path/to/private-analysis/japanese-shot-v1.cmjpack \
  --shoot --capture /path/to/private-analysis/native-shot-scene.bmp
```

This creates a software renderer on a surface, draws once and exits without a display server. Omit `--shoot` for the initial scene. Capture paths must be new `.bmp` files; exclusive file creation prevents replacing existing data. Keep captures outside Git. This is a normal development output path, not a test runner.

## Current evidence

An official SDL2 SDK was downloaded/built under `/tmp` in the cloud environment, with no system install or dependency source added to the repository. A separate CMake production build with warnings enabled built the scene, laboratory and prototype. Default Make production targets also built without SDL2. Software capture produced a640×480BMP, inspected through a PNG format conversion, displaying the native default result: power33, keeper79 and punch. Normal CLI usage after the shared-scene extraction preserved those values.

The cloud build uses software/dummy SDL support. Real window keyboard/GPU behavior, Windows execution, frame-rate equivalence and error-path behavior have not been exercised. No automated tests or suites were added/run for this increment. Static code review and the limited production/capture observations are the available evidence.

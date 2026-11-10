# Windows development shot scene

The optional scene is a resettable shot calculation with placeholder art and fixed development inputs. It is not a persistent match. It requires a compatible private shot-content pack; the pack must remain outside this checkout.

On the current workstation, from the repository root in PowerShell:

```powershell
& '.\build\demo\captain-shot-scene.exe' '..\Captain-Majid-analysis\japanese-shot-v1.cmjpack'
```

`build/demo/SDL2.dll` must be beside the executable. The local SDK is SDL 2.30.11 under `build/sdl-sdk`; the build directory is ignored. To rebuild the configured scene, run `cmake --build build/demo --target captain-shot-scene -j 2`.

Space requests a shot; Up/Down adjust attack, Left/Right adjust keeper, D changes defenders, R clears the result, and Escape exits. These are the controls implemented in the scene; the first Windows launch and visible field were confirmed, but the complete keyboard interaction and repeated-shot walkthrough still require verification.

On the current Windows build, `--shoot --capture` has been checked to create a bitmap and refuse to overwrite an existing file. A nonexistent pack produces `Unable to open local content pack`. Capture is headless and does not prove keyboard interaction.

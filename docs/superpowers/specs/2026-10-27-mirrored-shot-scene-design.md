# Mirrored development shot and catch research

Support possession side0/11 in the existing resettable native shot scene. Keep side0 defaults unchanged. Side0 uses attacker3/keeper11/field defenders14..18; side11 uses attacker14/keeper0/field defenders3..7. Derive all roster accesses, context, keeper recovery and printed stamina from the selected side. Ball coordinates remain explicit: the mirrored example uses(95,100), not the old(160,100). Do not silently mirror caller coordinates. Reuse native approach, preparation, energy and outcome APIs. Side changes require version2 geometry content; old version1 packs retain the existing default side0 fixture.

Add a byte-valued fixed `host_buttons` input to the original tracer, default128. Expose `--host-buttons` and report the actual mask for held/edge inputs; it is synthetic repeated input, not a real sampling schedule. Mask129 supplies A+Right for the original side11 keeper menu's catch choice. Keep bounds and hardware guards.

Record later ball-flight comparisons and their power before/after handler dispatch separately from initial selected-defender contacts. A crowded constructed side11 snapshot loses power33→21 through four such contacts; a controlled snapshot with opposing field players away from the route reaches catch with power33, strength177, ratio35 and keeper stamina980. Do not hard-code that crowded loss into native shots or claim the missing live-flight stage is ported.

No tests added/run. Original observations, production builds, normal usage and independent static review provide limited evidence. Keep snapshots and derived reports private; a mirrored resettable scene is not a persistent match or online mode.

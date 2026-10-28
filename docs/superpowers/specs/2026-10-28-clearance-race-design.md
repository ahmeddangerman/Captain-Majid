# Clearance race

Continue phase33 from DE96. Select the nearest eligible field player on the active
side, then the other side, excluding saved carrier and opposing player. Give both
the destination cell. For each round, move active candidate with selector35 speed,
then other candidate with selector36 speed; first cell arrival wins. Preserve
original raw movement/fractions/status handling. The distance byte is the loop
counter: zero wraps and permits256 rounds. Elapsed becomes1, without match-clock,
energy/reward or random scheduling.

On win, through DF0B, commit winner/side and snap coordinate high bytes to the ball
while preserving fractions. Report side-music request and later script28/62;
stop before map/presentation tail. Timeout stops at DED4 before script52/recovery.
Do not execute live frames or infer downstream handlers.

Use privately exported256 pairs of replacement speeds, indexed by the existing
movement speed-profile mapping. Only reached candidate profiles are loaded.
Atomic errors and replay rejection. New phases34(winner presentation pending) and
35(timeout pending); save original430/431/432 (remaining/candidate IDs) in optional
race context. Session13 marker D appends presence+3bytes to session12:336bytes,
readers1–12 retained. Clearance context/progress retained in both new states;
validation permits transitional ownership only at pending setup/timeout.

# Logical control polling and movement

Connect the prepared control-ready state to one original logical post-yield input poll atE14A. Use explicit held/pressed NES masks, preserving original per-poll flags/defender cycling, decision timer and raw axis movement. Do not choose a wall-clock or NMI schedule.

Side0: set script bit6, prioritize heldB as an action-menu request (clear selected count/script flags), otherwise clear bit6 on any direction and set facing from Left bit. Side11: motion request is always1; pressedA cycles controlled defender forward, otherwise pressedB backward with1..10 wrap and status redraw requests. A+B prioritizes A.

For ordinary polling, a nonzero timer decrements and returns to control-ready. Timer0 resets to10 and performs the held-direction movement. Both opposite axis bits still select the positive direction, diagonals use full speed on each axis, and no status gate is invented. Side0 moves the carrier using raw selector32 speed; side11 moves the selected defender using selector34. Preserve fractional/clamped coordinate arithmetic. Refresh ball/cell from its carrier afterward.

Stop at an explicit control-clock boundary beforeE1E7, or after updating the last carrier cell atE19B when opposing-carrier cell changes require an AI decision. HeldB side0 stops at the action-menu boundary before its script runs. Clock, AI action selection, subsequent stamina/field updates and live frame scheduling remain next work.

Add private pack7 raw selector32 speeds (256words), retain1–6readers. Add motion-request byte0532 to optional control context in snapshot5 (305bytes), retain1–4readers with legacy motion flag default0 because the first poll overwrites it. New control phases10clock,11menu,12AI share strict context/owner/side/count invariants. Snapshot4 is a supported setup checkpoint; its added byte is overwritten before use.

Expose a copied session polling operation and explicit lab commands. Capture bounded original post-yield observations; use production builds/normal usage and one fresh static review, with no tests added/run. Original ROM/tables/RAM remain private. Progress is a deterministic logical input boundary, not a playable game or faithful real-time/NMI loop.

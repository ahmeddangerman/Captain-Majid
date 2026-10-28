# Native terminal contact rewards

## Goal

Derive original named-player reward accounting and connect the observed terminal field-contact reward caller before contact-clock processing. Keep experience separate from field coordinates and preserve development boundaries.

## Reward helper

Original C4C8 ignores type0 and types35+. For named types1..34, campaign offsets select a packed group/experience-slot mapping. The group and explicit action selector choose a multiplier; the opponent-stage table supplies the base. Preserve the observed low-byte scaling exactly, including its discontinuity when the base exceeds255. Saturate the selected global16-bit experience total at65535. No immediate level/stat changes are inferred.

All mapping/multiplier/base values come from a private content extension. Validate bounds and reject unsupported content/state instead of indexing outside arrays. The global experience roster has11slots and is independent of the22field player slots.

## Session integration

Retain the existing post-reward terminal API. Add an explicit rewarded terminal operation for the observed field defender caller, which chooses reward selector2 and invokes it once for result3, twice for result2 through the original shared loose-ball handler. Then run the existing terminal sequence. Do not repeat these rewards on added-time resume. Capture original reward-entry/exit snapshots to establish whether intermediate presentation touches any represented state; retain explicit timing limits.

Extend phase-session state with optional campaign/opponent/experience context. Older snapshots remain loadable without fabricated reward context; the rewarded operation rejects missing context. New bounded snapshot encoding uses explicit presence and zero unused padding. Preserve old private content readers while appending reward tables in version6.

## Evidence and scope

Record direct original reward observations for gates, packed mappings, stage scaling and saturation. Compare coherent represented terminal outputs through normal native commands and inspect persistence/resume behavior. Build production Make/CMake targets; no tests added/run under developer constraint. Fresh whole-plan static review at completion, then continue recovery/control work. Campaign progression, level thresholds, match-end rewards, presentation/NMI and full legal actions remain separate.

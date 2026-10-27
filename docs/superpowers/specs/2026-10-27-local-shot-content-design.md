# Local shot content and native laboratory

## Goal

Load replaceable shot data into the native engine from a local file, and expose the joined calculation in a small command-line laboratory. Keep all original data private and outside source control.

## Approach

A Python exporter validates the known Japanese ROM and writes a bounded version1 `.cmjpack` containing profile mappings/skills, attack/defense/keeper moves, curves, comparison tables and punch intervals. A C++17 reader constructs the existing content APIs with strict size/truncation/trailing-data checks. No dependency or embedded original tables. A separate `captain-shot-lab` executable runs an explicit development scene through the native prepared calculation and prints its result. Optional interactive mode changes levels/subtype/defender count and fires independent shots.

Alternatives considered: directly parsing ROM files in the core couples simulation to cartridge layout; embedding tables publishes original data. A replaceable local data pack keeps the simulation/content boundary explicit.

## Scope and format

- Binary magic `CMJSHOT1`,32-byte source payload SHA256 provenance, shared mappings, field/keeper skill rows, three move-content sections, field/keeper comparison data, punch intervals. All multibyte counts little-endian.
- Ordinary mappings35bytes; alternate count<=221; generic slot offsets11; team types<=256; field profiles1..256 of31bytes; keeper profiles1..256 of8bytes.
- Attack/defense selectors15each, keeper selectors4; move lists5/5/4 with0..64records of4bytes; each section has192curve bytes.
- Field group arrays5each, eligibility1..256bytes, variation1..128pairs, thresholds4; keeper range/offset and thresholds4; punch intervals8.
- Original exporter leaves generic team types empty because runtime team selection is unresolved. Ordinary types are supported by the lab. No graphics/audio/text/campaign/team roster importer is implied.
- Export refuses existing output and requires `.cmjpack`; input files never overwritten. Generated packs ignored by Git.
- Laboratory is a side0 fixed scene at ball(160,100), attacker type1/slot3, keeper type33/slot11, optional field action2 defenders14..18, stamina1000. Explicit staged random bytes are attacker/field(100,100), keeper(0,100); distance adjustment45 and clear route/status gates follow the observed fixture. This is a constructed scene, not general original scheduling or a match.
- Default shot uses level20, keeper level20, subtype0, two defenders and keeper action1. Each shot resets; no fabricated match persistence.
- No tests added/run; production builds and normal export/lab usage provide limited evidence. No original files, packs, snapshots, reports or assets committed/pushed.

## Next work

Replace explicit fixture gates/randomness/distance with native approach and movement scheduling, implement persistent atomic match commits and rendering, then attach ordered session commands. Pack format is a development format with explicit version rejection.

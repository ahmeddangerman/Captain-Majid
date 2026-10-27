# Field script parser trace

## Goal

Observe original script-parser writes between defender comparisons, using explicit private RAM snapshots. Separate original execution from host actions and omitted presentation work. This supports resolving main-loop power-loss timing without inventing an engine rule.

## Approach

Add a command-line research tool using the existing guarded PRG memory adapter and py65. Execute the original banks16/17 script selector and parser to their returns. Record writes to selected gameplay addresses, presentation requests, instruction counts and host actions. Do not execute rendering, audio, the scheduler or frame interrupts. Abort on unmodeled hardware, absent cartridge RAM or a bounded-execution limit.

Alternatives: a full NES emulator capture would include real frame effects but needs a separately configured emulator/input capture; skipping script calls gives no evidence about parser writes. The bounded parser is the implementation choice for this increment and does not replace a full trace.

## Contracts

- Accept only the verified private Japanese ROM; no original content bundled.
- Require an explicit 2048-byte internal RAM snapshot. Optional cartridge RAM must be exactly8192bytes; reject cartridge access if absent.
- Restrict scripts to5,6,7, the field-contact script IDs currently under investigation.
- Reject snapshots with mapper PRG mode bit6 set in shadow `$22`; this entry requires banks16/17 at `$8000/$A000`. Initialize mapper select and shadow `$23` consistently with `$22|7`.
- Watch both bytes of shot power by default and identify original writes by PRG bank and PC.
- Preserve the supplied snapshot; outputs are private research artifacts written outside Git.
- Log host initialization and presentation acknowledgement separately. A successful parser trace is not proof that omitted presentation/frame effects leave power unchanged.
- No engine behavior changed. No automated tests added or run under the current developer instruction.

## Scope limit

Constructed snapshots are experiments rather than gameplay captures. Trace completion concerns the script parser only. Future complete-loop tracing must include clock/status changes, real presentation callbacks and random scheduling.

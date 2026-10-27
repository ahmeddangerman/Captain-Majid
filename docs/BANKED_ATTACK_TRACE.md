# Original banked attacking-stage trace

`tools/verify_banked_attack.py` executes the Japanese reference's complete `$8F72` attacking routine through its return. The original `$C54B` dispatcher loads PRG banks28/29, executes `$819D` and the actual player-profile/move loader, restores banks26/27, applies energy and calculates action power. No calls are skipped.

The trace's CPU memory adapter models the PRG windows used by those instructions, including the MMC3 PRG mode and register6/7 writes. It mirrors the internal2KiB RAM, allows cartridge RAM from `$6000`, and rejects PPU/APU/hardware/expansion-space accesses through `$5FFF` and unsupported mapper writes. Synthetic memory tests cover these guards, RAM mirroring, PRG switching and mode changes. It has no frame/interrupt emulation and is a verification tool rather than the product runtime.

Each fixture verifies the original writes `(6,28),(7,29),(6,26),(7,27)`, then compares selected skill/index, loaded strength/effect/cost, final stamina/cost/weak/carry, upper move flags, adjusted base, multiplier, power and subtype. The multiplier must be captured at `$8FDD`: original multiplication clobbers its temporary storage afterward. On side11, observed carry at `$8F9A` is passed as the native caller's explicit carry input; this preserved output is not independently derived by the engine.

**Result:3,882 constructed cases matched all14 native outputs.** Coverage includes both possession sides, valid action0..4/situation/move combinations, ordinary field types, generic opposing profiles, zero/exact/insufficient stamina and high-word boundaries, level carry boundaries, subtype bit7 and2048 seeded cases. These are constructed routine inputs, not captured matches. Alternate-profile cases and asynchronous original RNG scheduling are not part of this corpus.

Run with the approved private ROM and py65:

```sh
PYTHONPATH=/path/to/py65 python3 tools/verify_banked_attack.py /path/to/reference.nes
```

The script validates the approved payload hash, reads content only in memory, compiles a data-free native adapter in a temporary directory and transmits supplied content through stdin. It emits no extracted game assets. The product engine is unchanged by this verification increment.

Next: extend banked tracing through original field comparison/data selection and keeper/outcome paths, resolve remaining distance/interrupt rules, then introduce the ordered shot command once the full calculation contract is established.

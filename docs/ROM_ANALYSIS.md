# Captain Tsubasa II ROM Analysis

**Analysis date:** 2026-10-26  
**Status:** Structural comparison and the first text/script formats are mapped; gameplay analysis remains.

This document records hashes and structural findings only. ROM files and reconstructed patched ROMs are excluded from the repository.

## Inputs

| Input | Size | SHA-256 |
| --- | ---: | --- |
| Japanese ROM | 393,232 bytes | `bf5038afe4c9df1c1c7eff0bc74a12f3cd8ed994b9aab92617d066d9d10ad746` |
| Complete Arabic ROM | 524,304 bytes | `19107f3e9bdc39276db4b816fc7439b167930a4e5837d23e01e5a078c8e5e690` |
| Extraordinary Arabic IPS patch | 38,045 bytes | `807ccad1ef18799d298129b3f759d0aac1b4f317b0a11e5fd81457c3327007c3` |

The Japanese ROM matches the No-Intro entry for `Captain Tsubasa II - Super Striker (Japan).nes` exactly: CRC32 `149DAC0C`, MD5 `BFA2A7C745841C51527430F231F27A26`, and SHA-1 `49BB04A2165F80D1EB936B259E68CB9CF2B57143`.

The patch documentation's expected MD5 `77012436A85B73852E0A4F36856373DF` represents the same 393,216-byte PRG+CHR payload with the alternate iNES header `4E45531A101041000000000000000000`. No game data differs. A corrected-header working copy was derived locally from the verified upload, yielding the expected MD5 exactly; the original upload remains unchanged.

## Cartridge layout

| Version | Header | Mapper | Mirroring | PRG code/data | CHR graphics |
| --- | --- | ---: | --- | ---: | ---: |
| Japanese reference | NES 2.0 | MMC3 / mapper 4 | Horizontal | 256 KiB | 128 KiB |
| Complete Arabic ROM | iNES 1.0 | MMC3 / mapper 4 | Vertical | 256 KiB | 256 KiB |
| Extraordinary patch applied to Japanese ROM | Preserves Japanese header | MMC3 / mapper 4 | Horizontal | 256 KiB | 128 KiB |

The complete Arabic ROM doubles the CHR graphics capacity. Visual tile extraction confirms Arabic font and modified screen graphics, while exact runtime assignments for the added banks still need to be traced.

## IPS patch base

The IPS format does not contain a source-ROM checksum, so its intended base cannot be proven from the patch alone. The evidence strongly indicates the verified Japanese ROM:

- The patch makes 229 writes totaling 39,160 bytes.
- Its first write begins at file offset `0x0004E0` and it never modifies the 16-byte NES header.
- Its last write ends at `0x060010`, exactly the final byte boundary of the Japanese ROM.
- It does not write into the complete Arabic ROM's additional 128 KiB CHR area.
- Applying it to the Japanese ROM produces a structurally valid 393,232-byte mapper-4 image.

Applying the IPS patch to the exact `77012436...` header variant produces MD5 `77C1CBD0CE94A09E3604F2535E21DAAD` and SHA-256 `0A2C2A6F21F428A1018D58A03EFB152EC6A1E966B000D83E9B6B4EEBF8A9A8FC`. This reconstructed file is retained only in the local analysis workspace.

## How the Arabic versions differ

Relative to the Japanese reference:

| Region | Complete Arabic ROM | Extraordinary patched variant |
| --- | ---: | ---: |
| Header bytes changed | 4 | 0 |
| PRG bytes changed | 30,903 | 33,260 |
| Existing 128 KiB CHR bytes changed | 4,583 | 3,551 |
| Additional CHR data | 131,072 bytes | 0 |

The two Arabic adaptations touch 25,021 of the same PRG offsets and 3,095 of the same CHR offsets, but only 1,098 overlapping positions end with the same byte. They therefore appear to be distinct technical implementations rather than small revisions of one translation.

Across the Japanese-sized range, the complete Arabic ROM remains 90.97% identical to the Japanese reference. The Extraordinary output remains a Japanese-sized ROM and uses a different set of translated code and graphics changes.

## Port implications

- Treat the Japanese ROM as the behavioral reference.
- Treat the two Arabic versions as separate content and compatibility sources until their text engines and scripts are mapped.
- Extract PRG bank maps, text routines, string tables, font tiles, and screen layouts from both Arabic implementations.
- Keep localization data independent from the native game simulation so Arabic, English, and Japanese can share gameplay code.
- Do not assume the expanded Arabic CHR banks are required by the Extraordinary version.

## First bank and startup findings

- The Japanese and complete Arabic ROMs share interrupt vectors `NMI=$C500`, `RESET=$FFF0`, and `IRQ=$C506`.
- Their reset stub selects MMC3 bank register state through `$8000`, then jumps through the fixed-bank entry table at `$C503`.
- The Extraordinary variant preserves the NMI and IRQ vectors but changes reset to `$FFE8`. Its added stub initializes MMC3 PRG-RAM control at `$A001`, disables PPU rendering through `$2000/$2001`, then rejoins the original entry table at `$C503`.
- Visual CHR extraction confirms that bank 0 contains the primary Japanese/Arabic glyph sets and bank 15 contains title/logo graphics. Both Arabic versions replace these areas differently.
- The largest program changes in both Arabic versions are concentrated in 8 KiB PRG banks 3, 4, and 5, with additional translation-related changes in banks 24, 25, and 31.
- The first dialogue pipeline and byte format are documented in [TEXT_FORMAT.md](TEXT_FORMAT.md). Static message rendering now produces readable Japanese and Arabic previews directly from ROM data.
- PRG banks 3 through 6 are scenario-script banks with self-indexed pointer tables. The scenario loader, ID ranges, bank selection, and extraction format are documented in [TEXT_FORMAT.md](TEXT_FORMAT.md).

## Next analysis milestone

1. Trace representative scenarios in an emulator to label scenario IDs and confirm provisional control-code meanings.
2. Map the expanded CHR banks in the complete Arabic ROM to the screens that load them.
3. Begin mapping gameplay state, match rules, and audio after the text/script pipeline is validated dynamically.
4. Boot-test the reconstructed Extraordinary variant in a suitable NES emulator before treating it as confirmed playable.

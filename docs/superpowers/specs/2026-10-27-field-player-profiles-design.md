# Field player profile lookup

Continue the approved engine roadmap by resolving ordinary player records, generic opposing team positions, and alternate player records into existing defender preparation. Supply all profile/table content externally; bundle no original data. Use C++17 and preserve the existing preparation API.

`FieldPlayerRecord` contains ID, type, flags, alternate type and level. `FieldPlayerProfiles` contains ordinary and alternate type-to-profile mappings, 31-byte field skill views, the eleven generic slot offsets, and caller-selected team bytes. `resolve_field_player_profile` returns effective type, profile ID, and skills. `prepare_profiled_field_defender` feeds skills and level into existing move preparation.

Support field IDs 1..10 and 12..21. Generic type zero is supported for opposing IDs 12..21 only, through slot offset `id-11`. A resolved type >=35 selects the alternate table; flag bit7 replaces it with record alternate type, which must remain >=35. Ordinary types ignore the alternate flag. Reject unavailable table/profile entries and unsupported IDs before indexing. Stamina and keeper profiles remain later work.

Approach: decode type mappings and profile skill views into replaceable content, rather than execute original machine instructions in the product or embed ROM tables. A raw ROM importer remains separate work. Compare the composed native preparation to the complete original loader using private content in memory.

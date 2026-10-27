# Native field coordinate step

Implement original `$E912` as `advance_field_coordinate(uint16_t coordinate, uint16_t displacement, FieldAxis axis)`. Values are raw16-bit words; displacement is a modulo16-bit bit pattern, avoiding signed C++ overflow/conversion. Add the words with wrap, clamp only the resulting high byte to X48..207 or Y80..175, and preserve the low byte even when the high byte clamps. Reject invalid enum values. CPU flags/scratch state are outside the contract.

Expose this pure helper through the existing native research protocol as `fieldaxis COORD DELTA AXIS` and `--field-axis-batch`, with axis0/1. The stateless request does not mutate the ordered fixture. This is not a whole movement/AI step: speed/direction, player status, target cells, elapsed scheduling and persistent commits remain separate.

Record bounded original observations privately; production builds and normal native query usage provide limited evidence. No tests added/run. Do not introduce original tables/assets or claim full movement equivalence.

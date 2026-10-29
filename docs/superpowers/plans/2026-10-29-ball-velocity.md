# Ball velocity projection plan

OriginalCE4A/CE4D uses a64word quarter-wave table. Double the direction byte,
mirror when doubled bit7 is set, mask126 and divide2 for table index; negate the
word with16bit wrap when original direction bit7 is set. Horizontal projects
(direction+64) modulo256; vertical projects direction. Preserve ball displacement
semantics separately from clamped player movement. No gameplay/RNG/time writes.

- [x] Synthetic real quadrant/boundary/zero/signed-minimum tests failed against stub.
- [x] Implement pure BallVelocityContent/calculate_ball_velocity and build targets.
- [x] Compare all256directions against original calls using private table input.
- [x] Default full suite/build, final review, dated log and local commit.

# Defender selection implementation plan

1. [x] Add unit tests for ROM selection order, both side directions, strict radius edges, status skips, team-specific caps, radius zero, and invalid side.
2. [x] Implement `select_field_defenders` with explicit position/status inputs and ordered ID output.
3. [x] Add a strict research batch command for the selector and expose it from the executable.
4. [x] Add a Py65 differential verifier that executes `$E4D7` in the verified Japanese ROM over boundary fixtures and seeded cases.
5. [x] Register source and tests in Make and CMake; run unit, CLI, repository, and ROM comparisons.
6. [x] Update the ROM trace and progress/roadmap notes with the new behavior and its limits.

# Saved shot approach plan

Continue phase45 at originalDCFD throughDD08, before ball-flight scheduling.
Reuse calculate_shot_approach. Only random_first may change in represented prior
state. Save061A counter255,062B distance,0638 destination and carry/loose route in
new phase46 context. Snapshot19J appends5bytes=344total; retain readers1..18.
Do not replace ball5FE cell or coordinates with destination; they are distinct.

- [x] Add failing real approach regression.
- [x] Add context/validation/I/O and native wrapper/CLI, Make/CMake targets.
- [x] Compare originalDCFD throughDD08, thresholds127/128, parity and unopposed.
- [x] Full suite/build, review, dated evidence/log and private commit.

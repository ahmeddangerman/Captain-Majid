# Ordered selected-shot timing

The native phase session can now step one selected defender at a time, preserving
original status/clock ordering before the field-result dispatch boundary. This is
the continuation to use for timing work; the older `shot-defenders-file` command
remains a batch calculation checkpoint with its documented timing limitations.

## Usage

Starting with a prepared phase20 shot:

```
captain-phase-session PACK PREPARED --step-shot-defense 0 0 100 100 --capture NEXT.cmjsession
```

Four decimal bytes supply action, subtype and two random samples. The player ID
comes from the next saved selected slot. Supported choices remain ordinary field
action0/2, situation0, subtype0/128. Energy/profile/comparison read the current
saved roster; attacker preparation and its stamina debit are not repeated.

Interactive commands:

- `step-shot-defense ACTION SUBTYPE RANDOM1 RANDOM2`
- `resume-shot-clock` after an explicit added-time presentation acknowledgement
- `dispatch-shot ELAPSED` after all selected defenders fail, or no defenders exist

The command-line equivalents are `--step-shot-defense`, `--resume-shot-clock` and
`--dispatch-shot`. Final elapsed is explicit replaceable data. Original action0
uses1 without defenders (90DD/9109) and2 after exhausting a selected prefix
(9085/908E). Terminal/interruption results skip this extra clock step.

## Ordered behavior

Each comparison runs the interruption reset, then9095. Result0/1 applies the
configured status to the reached defender; result2/3/4 applies status5 to the
carrier. Keeper/shared recovery and match clock then advance. No next comparison
is prepared before a clock pause resumes. The final dispatch clock only advances
elapsed/recovery/clock and does not assign another player status.

Session9 is330bytes: the session8 layout plus presence and five progress bytes
(cursor, result, last defender/action/rolled subtype). Phases23–27 distinguish
next contact, contact added time, final clock pending, final added time and
field-dispatch ready. Session readers1–8 remain available. Period-end may retain
progress for inspection but cannot resume through added-time commands.

Original617 is the active-selected-keeper count during this stage. Supported
choices are field players, so it resets to0 through the represented control
last-carrier-cell alias, without interpreting it as a movement-cell update.

## Evidence and limits

- Focused synthetic regression failed against a throwing stub, then passed with
  ordered status/clock effects, prefix completion, terminal/interruption stopping,
  added-time snapshot roundtrips/resume, invalid/repeated calls and period-end.
- Full `make -j2 check` and production `make -j2 all` passed using a temporary
  archive of the same core sources (`CORE=/tmp/cm-staged-core.a`), avoiding repeated
  compilation of every core file for every test. No compiler warnings/errors.
  CMake is not installed in this executor.
- Three original constructed CPU observations were rerun with final RAM capture.
  One passed defender (clock451→448, elapsed1→4) and interruption4 (451→449,
  elapsed1→3) match all324 represented prior-format bytes after setting the native
  phase/version tags. The six appended progress bytes are native metadata and
  are not an original-memory comparison.
- Capture3 matches gameplay/clock fields (451→449, elapsed1→3), with one difference
  at represented byte298: native facing0 versus original64. Tracing establishes
  this is the script parser ED0F copying presentation argument052A to0517. This
  increment does not implement that script presentation side effect, so the
  capture observation is not a full represented-state match.
- Fresh independent static review reported no material findings; it noted a
  missing direct regression for terminal-result added-time save/resume. Shared
  phase24 resume is exercised by failed contacts; direct terminal coverage can
  be extended when dispatch handlers are connected.

Private evidence is under
`/workspace/Captain-Majid-analysis/ordered-shot-clock-0sqp740y`.
ROM/RAM/content/snapshots remain outside Git. These are constructed observations
with disclosed host task/frame/display acknowledgements, not complete hardware
or full-match equivalence.

Next: trace and connect the initial selected-field dispatcher27 into its actual
rewards/possession/approach routes. Keep it distinct from later-flight terminal
handlers until original evidence establishes the shared path. Script effects,
frame/random scheduling, restart execution and a complete match remain pending.

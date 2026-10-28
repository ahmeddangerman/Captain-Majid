# Capture clearance setup

The action0 capture-animation entry (phase29) now prepares its original clearance
through E678..E6CC, stopping before DE96 flight scheduling. This is a numeric
continuation with explicit presentation requests, not a rendered animation.

Original E678 switches active side, requests team music and task2, then E688 picks
one of eight destination pairs with randomE2&7. Source ball-X/Y bit7 independently
selects ones-complement reflection (XOR255). Target coordinates and their field
cell are stored, followed by the distance adjustment. The previous carrier ID
is retained: active side11 can coexist with the side0 carrier during clearance.
No clock step, stamina charge, reward or random generation is added.

## Private content and usage

```
python3 tools/export_capture_clearance.py ORIGINAL.nes PRIVATE.cmjclearance
captain-phase-session PACK ANIMATION.cmjsession --prepare-clearance-file PRIVATE.cmjclearance --capture FLIGHT.cmjsession
```

The exporter verifies the approved ROM fingerprint and writes eight coordinate
pairs plus distance as17 decimal bytes. It uses only Python's standard library;
its initial accidental dependency on the CPU verifier was removed before the
successful export. Content is private, ignored by Git and never bundled as assets.
Interactive usage is `prepare-clearance-file PRIVATE.cmjclearance`.

Invalid/missing/extra bytes and reached outside-field targets reject atomically.
Unreached destination entries are not evaluated. Repeated clearance setup rejects.
Audio and task2 are requests; execution, actual frame timing and RNG remain pending.

Phase33 stores the flight boundary and distance context. Session12 marker C is
332bytes: session11 plus presence/value. Older versions1–11 still load; a version11
phase29 entry rewrites with absent flight context. Only33 accepts that context.
Its owner/side transition and retained selection have explicit validation rules.

## Evidence

Focused regression first failed against a throwing stub, then passed. It covers
all32 sample/reflection combinations with synthetic content, unchanged owner,
side flip, cell selection, clock/energy/reward preservation, replay, invalid target,
new snapshot roundtrip and direct version11 entry compatibility. The full existing
suite and production Make build passed using a same-source temporary core archive
(`CORE=/tmp/cm-clearance-core.a`); CMake remains unavailable. Fresh review found
no material issues; its requested legacy entry coverage was added and passed.

Four original CPU/task observations and native CLI operations match the324 prior
represented bytes after phase/version normalization, plus the new distance byte
compared independently with original062B. Six retained progress bytes are native
metadata and excluded from the original-memory comparison:

| Source high bits / sample | Native332-byte SHA256 |
| --- | --- |
|00 /0|a430679bc0fd3914bc25e8124090cbc9616c79efc46388649d9de0142949c1a1|
|X /1|18b3f5beabc6e8100cfe07625e69f40ab762c81b16119750c1f8fd0bca4aa25d|
|Y /4|f547e9713602e24cf31c6409fa7afbf5333b87f6b5168171793333f0fc530589|
|XY /7|805b950be43c2182ac71e0083810b8d670bfeef83ddc5a7a3aaecafe0a85b2db|

Private evidence: `/workspace/Captain-Majid-analysis/capture-clearance-ua_vap65`.
The observer now has `--clearance-only` to enter E678 and stop before DE96. It uses
explicit cooperative-task/frame/display acknowledgements; no calls are skipped.
No NMI, audio hardware, PPU or true frame timing is emulated. Comparisons start at
an original animation-entry state, including its existing script-facing value;
they do not resolve that earlier presentation difference or prove a full shot.

Next: original DE96 clearance-flight scheduling and its recovery handoff, then
remaining restart/keeper/story and nonterminal shot paths. A complete playable
match, production renderer and online match remain unfinished.

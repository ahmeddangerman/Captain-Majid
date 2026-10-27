# Native contact status, clock and phase session

## Goal

Connect the original contact status/time caller at `$9095` to the existing native player/terminal phase. Preserve the order of state writes and clock diversions so a terminal contact can progress from coherent post-reward input without injecting hard-coded elapsed/status values.

## Clock

Implement the match-clock decisions after original `$D1A0`: raw remaining word, extra-time byte, warning flags, script flags and ball-mode byte. Preserve zero-step bypass, wrapped subtraction/sign tests, low-byte warning comparison, delayed period-end behavior and random-sign extra-time addition. Emit typed continuation/events for added-time presentation and period end instead of executing rendering/audio calls. Period end retains the old extra-time byte when the original branches before its store. Added-time presentation is a deferred boundary, not silently acknowledged.

## Contact prelude

Replaceable content supplies fourteen field elapsed bytes, fourteen keeper elapsed bytes and seven defender status bytes. The explicit original carry branch selects whether a field defender receives its configured status or a field attacker receives status5; keeper defenders bypass those status writes. Add elapsed with byte wrap, recover both keeper roles/shared timer using the existing helper, then advance match-clock state. No experience rewards or presentation/NMI refresh are inferred.

## Coherent terminal sequence

The session receives post-reward/presentation input at the contact-clock boundary. For later result2, copy defender coordinates before clock processing; clear selected count only after the clock returns ready. For result3, process the clock before possession transfer/player advancement. If time ends, abandon the contact continuation; if an added-time presentation is requested, retain a typed pending contact. Explicit resume requires the caller to have supplied/accepted the post-presentation state; the lab's frozen-input resume is a development operation, not live hardware timing.

## Persistence and content

Use a phase-session state containing the represented field state, match clock, shared keeper recovery state, phase and optional pending terminal context. Provide bounded explicit serialization/private RAM export and a native persistent session laboratory. Do not call it a complete game save or authenticated network protocol. Keep the existing239-byte field snapshot usable.

Private pack version5 appends35contact bytes after version4; readers retain old formats. No original data/assets are committed. Runtime typed clock events can later drive replacement presentation without using render speed as simulation time.

## Evidence and limits

Record bounded original clock/contact observations and coherent before/after phase-session snapshots privately. Build production targets, inspect normal native session usage and request one fresh static review of the whole plan. No automated tests added/run under the active developer constraint. Experience/level-up rewards, real NMI/audio/controller scheduling, complete period transitions and a playable match remain further work.

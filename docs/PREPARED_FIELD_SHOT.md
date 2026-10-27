# Prepared field shot handoff

`resolve_prepared_field_shot` joins prepared defender contacts to their field handoff and the supported action0 goalkeeper continuation. Its result includes the updated field roster, each visited defender's preparation/energy/contact, the immediate field outcome, and an optional continuation containing goalkeeper/outcome results.

## Stage connections

- The original no-defender branch writes result0. This wrapper initializes the field result to0 even when the low-level context fixture contains a different initial result.
- Field result2 invokes ordinary recovery when a fixture is present, or returns an unresolved recovery route otherwise. Recovery excludes the current ball owner and the last visited defender; stale owner/opposing-player fixture fields are overwritten. Recovery coordinates, availability, post-generator random bytes and selected-defender list remain explicit phase inputs.
- Field result3 dispatches using the **last visited** defender's ID and chosen action. It can transfer field-player possession or return the defense animation route. Later selected defenders are not consulted and keeper fixtures are not evaluated.
- Results0/1 enter the existing action0 gates. Unsupported action, remaining active defenders, field proximity and keeper status can divert play. Otherwise remaining shot power flows into keeper comparison and goal/save handling.
- Keeper identity follows the possession side. Outcome side/owner follow the field context and current owner; stale fixture values are overwritten. Returned keeper outcomes contain goal counts, ownership, ball state and other immediate writes. A caller must still commit these outputs to persistent match state.

## Subtype correction

ROM keeper comparison operates on defensive subtype `$043E`; subsequent outcome logic reads attacking subtype `$043C`. The previous `resolve_keeper_outcome` composition incorrectly copied attack subtype into the keeper roll and copied the keeper's resulting subtype back into the outcome. This increment preserves the keeper's supplied move subtype and sends the attack subtype directly to the outcome handler, correcting both direct shots and action0 continuation. A regression test failed before the change and now checks that result1 can produce special turnover using attack subtype3 while the keeper's subtype stays0.

The last field defender's subtype likewise never becomes the attack subtype. The wrapper takes the attack subtype from `FieldDefenseContext`, representing the value after attacking power generation.

## Evidence and remaining work

Native integration checks cover goals on both sides, power loss before a rebound, catch/punch, terminal capture/animation, recovery with stale fixture identities, missing recovery, empty lists, every continuation gate, invalid reached phases, untouched unvisited phases, separate subtypes and input preservation. Existing private-ROM oracles validate the comparison/outcome/recovery components; this joined wrapper has **native integration evidence**, not a full original-loop differential trace.

Attacker power/profile preparation, goalkeeper profile and stamina preparation, automatic comparison tables, random scheduling, boundary restarts, rewards, animation and ordered match commands remain separate. The existing network fixture still demonstrates contact-loss replication, not playable online matches.

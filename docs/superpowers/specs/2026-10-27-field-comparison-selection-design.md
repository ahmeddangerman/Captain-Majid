# Field comparison content selection

Reproduce original bank26 `$834C..837F`: map attacking action through supplied attack group, multiply by4 wrapping byte, add defense group (fixed2 for keeper0/11), use resulting byte for eligibility and doubled wrapped offset for variation pair. Eligibility uses bit7. Classification uses the fixed four caller thresholds after comparison resets X/Y to0. Supply every byte as replaceable content; bundle no original tables.

Expose pure `select_field_comparison` and `configure_field_comparison` over existing contact input. Bounds-check actions0..4, slot0..21 and selected content entries. Keep all unrelated input fields unchanged. Empty/missing selected content rejects; zero variation range may be returned by selection but existing arithmetic rejects it. Do not claim every constructed action/slot combination occurs in actual gameplay.

Use banked private ROM to execute selection through classification with no skipped calls, comparing selected index, coefficients, eligibility, thresholds and arithmetic outputs. Initial strength/power/random/weak values are supplied fixtures; loader/energy and later loop handler are not joined here. Code4 override at8E33 and power loss in the main caller require further tracing before integrating selection into a verified complete loop.

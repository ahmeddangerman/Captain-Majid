# Evidence-fed field-round comparison

The original field round may span many frames. Its current native batch accepts per-player buttons but freezes random and travel state across the round. Add a separate operation that consumes 22 explicitly captured pre-poll samples: buttons, random bytes, and an optional previously observed travel-task invocation with held input and loaded speed. The caller supplies events and ordering; the operation does not synthesize frames, NMI, audio, presentation, or missing events. Preserve the existing batch operation and saved-session format.

The operation must keep input sessions immutable on errors, validate all file records before execution, and stop on the existing menu/other boundaries. Compare it against instruction-boundary original captures, including the final represented state and supplied random/travel effects. This is a comparison facility, not a general scheduler or playable match.

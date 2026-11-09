# NMI-input field-round comparison

Add a separate evidence-fed field-round mode that calculates each random update using the existing original-backed random kernel. The caller supplies the initial random index and, for each of 22 polls, the observed NMI memory samples, incoming carry, buttons and optional travel invocation. The evolving random bytes come from the session and the previous native player step; the result returns the final random index separately. Existing snapshots remain unchanged.

The operation is a bounded comparison of explicitly supplied events. It does not derive NMI timing, generate sound or presentation state, or infer missing memory samples. Validate the entire input and keep errors atomic. Compare its result with captured original instruction boundaries and the prior explicitly sampled-random round.

# Object-record wall-clock delay experiment

The same recorded race (`/tmp/race/r1.inp`, SHA-256
`4d24c52b96e5c199f301dec65b847e44fcf21d5f39dfc8a9476e703883c902f9`) was
replayed three times through frame 1800 while tracing the main-CPU
`0xFFDD00–0xFFDDFF` window:

| Run | Difference | Writes | Event stream |
| --- | --- | ---: | --- |
| normal-a | none | 57,631 | baseline |
| normal-b | none | 57,631 | identical to normal-a |
| wall-delay | 2-second host sleep at emulated frame 900 | 57,631 | identical to normal-a |

All traces were untruncated. Writes begin at frame 37 and end at frame 1799;
the prominent course-transition burst is at frame 883. The normalized event
arrays share SHA-256
`7f01bb7c8b4887cbe4024812e140a03878cf0ab414867df6be8597d9ac284db7`.
The MAME log hashes differ because host-runtime text is not a stable event
identity.

## Verdict

The object-record activity is **wall-clock-invariant** and follows emulated
execution, not elapsed host time. This run does not distinguish
**frame-locked** from **position-locked**, because the same replay advances
the craft along the same trajectory at the same emulated frames in every run.
A position perturbation that preserves comparable emulated time is still
required for that distinction.

The transition burst is a literal spawn/update landmark only. The writer PCs
and address offsets identify touched bytes, but do not prove object identity,
track position, armor, weapons, ammunition, or a spawn counter.

## Saved-state steering perturbation

The same common `late_drive` save state was forked with held-left and centered
steering while tracing the full `0xFFDD00–0xFFDE00` range. Both streams were
untruncated. Their first aligned value difference occurs at relative frame
303, at `0xFFDD86`, with the same writer PC `0x03D18` but data `0x0000` in
the left fork versus `0x8080` in the center fork. Per-frame event counts first
separate at frame 304 (28 versus 29). The writer-PC sets remain effectively
shared, so this is a state/value perturbation rather than a new spawn routine.

This establishes that the `0xFFDD86` cluster responds to steering/trajectory
state from the common checkpoint. It does not establish whether the underlying
trigger is track position or emulated frame, and it does not assign the field
to armor, weapons, or a specific object. Provenance is in
`reference/experiments/stunrun/m5-object-record-steering-differential.metadata.json`.

A sidecar write trace over `0xFF9578–0xFF957D` shows that the frame-883 burst
is not a course-index transition: `0xFF9578` is not written in frames 850–900.
Instead, `0xFF957A` changes from `0x0003` to `0x0002` at frame 879 and the
adjacent `0xFF957C` transition field is written at frames 879/882. This ties
the observed burst to an input/transition-state neighborhood, while leaving
the object trigger and track-position relationship unresolved.

Full provenance and hashes are in
`reference/experiments/stunrun/m5-object-record-wall-delay.metadata.json`.

The focused `0xFFDD86–0xFFDD87` follow-up captures 3,041 writes in each fork
with the same writer-PC set. The first value difference is still frame 303:
held-left writes `0x0000` while center writes `0x8080`. The two traces have
the same total event count and 1,499 active frames, but their frame sequences
are not identical: periodic two-write phase shifts begin at frame 350. This
is consistent with trajectory-dependent update cadence after the initial
divergence, but it is not enough to label the field as a position cursor or a
spawn trigger. Provenance is in
`reference/experiments/stunrun/m5-dd86-steering-cadence.metadata.json`.

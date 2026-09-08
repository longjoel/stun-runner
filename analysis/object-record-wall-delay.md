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

Full provenance and hashes are in
`reference/experiments/stunrun/m5-object-record-wall-delay.metadata.json`.

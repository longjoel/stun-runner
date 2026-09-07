# Object-record timing landmark

This is the current evidence-backed result for the 68010 work-RAM window
`0xFFDD00–0xFFDDFF`. The window is associated with object/animation activity
in existing writer traces, but its field ownership and gameplay meaning are
not promoted here.

## Repeatable saved-state observation

Two independent MAME snapshot runs loaded the same saved live-drive state and
advanced with no additional input. Both captured relative frames 2–600 at
`0xFFDD00` and produced byte-identical snapshots at every common frame. The
window's coarse lifecycle was:

| Relative frames | Observation |
| ---: | --- |
| 2–38 | 49 nonzero bytes, including the established object/animation record neighborhood |
| 39–121 | all 256 bytes zero |
| 122 | first four nonzero bytes reappear around `0xFFDD87–0xFFDDAC` |
| 123 onward | the record repopulates; by frame 200, 48 nonzero bytes remain stable in the sampled shape |

The exact-repeat result is useful because it rules out run-to-run noise in
this saved-state fork. It does not distinguish a position-triggered spawn
from a frame-triggered update: the loaded state already fixes the gameplay
history, and no deliberate timing/position perturbation was applied.

## Interpretation boundary

The safe current label is **DETERMINISTIC OBJECT-RECORD TIMING; SPAWN
TRIGGER UNKNOWN**. Do not label `0xFFDD86`, `0xFFDD87`, or neighboring bytes as
armor, weapon, player position, or a spawn counter from this experiment alone.
The next discriminating experiment is a matched fork with the same saved state
and a controlled delay or movement difference, then comparison of the first
repopulation frame and the record's writer PCs.

## Provenance

The exact paths, snapshot geometry, MAME log hashes, and repeatability result
are recorded in
`reference/experiments/stunrun/m5-object-record-timing.metadata.json`.
The snapshots and saved state remain local; no raw RAM dump is committed.

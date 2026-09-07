# Object-record timing landmark

This is the current evidence-backed result for the 68010 work-RAM window
`0xFFDD00–0xFFDDFF`. The window is associated with object/animation activity
in existing writer traces, but its field ownership and gameplay meaning are
not promoted here.

## Historical saved-state observation

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

The exact-repeat result was useful as an initial observation, but it is not a
stable current baseline. A fresh revalidation from the same saved state with
the current pinned MAME produced 49 nonzero bytes at every sampled frame
through relative frame 180; it did not reproduce the historical clear
interval. Throttled and `--nothrottle` controls agreed byte-for-byte. The
historical snapshots remain provenance-backed artifacts, but their
clear/repopulation schedule must be treated as **NOT REPRODUCED** until the
execution-condition difference is identified.

## Course-transition checkpoint comparison

To separate ordinary live updating from a transition phase, two race-course
checkpoints were forked for 600 frames with no additional input:

| Checkpoint | First record-window write | Clear phase | Record-window events |
| --- | ---: | --- | ---: |
| `state-4800.sta` | relative frame 218 | `0x021302` clears the 256-byte window | 11,503 |
| `state-5400.sta` | relative frame 1 | already past the clear phase | 11,503 |

After initialization, both forks use the same updater family. The high-volume
writers are `0x03D18C`, `0x03D18E`, `0x03D192`, `0x03D1B2`, `0x03D1EC`,
`0x03D1F4`, `0x03D1FA`, `0x03D248`, and `0x03D24C`; the small differences in
their counts are consistent with phase alignment, not a new writer or a new
record format. The transition checkpoint also reaches the same one-frame
secondary update cluster seen in the settled checkpoint, shifted by about one
frame.

The useful human-facing conclusion is therefore narrower: a course transition
can expose a clear/reinitialization phase in the object-record window, while a
settled checkpoint starts inside the recurring updater loop. The experiment
does not identify which records are spawned, nor does it justify assigning any
field to armor, weapons, score, timer, or player state. The object-record
lifecycle remains **PARTIALLY BOUNDED; OBJECT IDENTITY UNKNOWN**.

## Interpretation boundary

The safe current label is **OBJECT-RECORD WINDOW OBSERVED; LIFECYCLE
CONDITION UNKNOWN**. Do not label `0xFFDD86`, `0xFFDD87`, or neighboring bytes
as armor, weapon, player position, or a spawn counter from this experiment
alone. The next discriminating experiment is a matched fork with a freshly
created state and writer-PC/input telemetry, after the historical/current
execution discrepancy is explained.

## Provenance

The exact paths, snapshot geometry, MAME log hashes, and repeatability result
are recorded in
`reference/experiments/stunrun/m5-object-record-timing.metadata.json`.
The transition comparison is recorded in
`reference/experiments/stunrun/m5-transition-object-writer-trace.metadata.json`.
The snapshots and saved state remain local; no raw RAM dump is committed.

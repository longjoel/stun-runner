# Object-record input differential

This is a follow-up to
[`analysis/object-record-timing.md`](object-record-timing.md). It uses the
same saved live-drive state and the same main-CPU RAM window
`0xFFDD00–0xFFDDFF`, but forks the run at relative frame 2 with three input
modes:

| Fork | Event at relative frame 2 |
| --- | --- |
| `fork_center` | AD Stick X = 128; release P1 buttons |
| `fork_hold_left` | AD Stick X = 0; release P1 buttons |
| `fork_hold_right` | AD Stick X = 255; release P1 buttons |

## Result

The three immediate input forks are byte-identical at every sampled frame
`2, 30, 39, 60, 90, 120, 121, 122, 123, 150, 180`. The delayed-left and
delayed-right forks (event at frame 80) are also byte-identical to a fresh
no-input control at every sampled frame through 180. At relative frame 180,
all current runs contain 49 nonzero bytes.
This means the experiment did **not** distinguish left, center, and right
steering in this saved-state window.

A prior comparison appeared to show a difference against an older no-input
replay. A fresh no-input control from the same state, under both throttled and
`--nothrottle` execution, is byte-identical to both delayed forks. The older
clear/repopulation replay is now classified as a historical execution
condition discrepancy, not an input effect.

The defensible conclusion is therefore:

> This saved-state experiment shows no reproducible left/center/right
> differential. The field semantics, lifecycle trigger, and the cause of the
> historical clear interval remain unknown.

The harness now logs each applied input event as `MAME_MEMORY_INPUT`. The next
discriminating experiment is a freshly created state plus writer-PC telemetry,
after the historical/current execution discrepancy is explained.

## Provenance

All runs used MAME `0.289 (mame0289-dirty)`, loaded
`/tmp/stunrun-latedrive1200.sta`, sampled 256 bytes at `0xFFDD00`, width 8,
and ran without throttle through relative frame 180. Raw snapshots remain
local. Full paths, result hashes, and canonical log hashes are recorded in
[`reference/experiments/stunrun/m5-object-record-input-differential.metadata.json`](../reference/experiments/stunrun/m5-object-record-input-differential.metadata.json).

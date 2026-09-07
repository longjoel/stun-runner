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

The three input forks are byte-identical at every sampled frame
`2, 30, 39, 60, 90, 120, 121, 122, 123, 150, 180`. At relative frame 180,
all three contain 49 nonzero bytes and their snapshot bytes match exactly.
This means the experiment did **not** distinguish left, center, and right
steering in this saved-state window.

All three forks do differ from the earlier no-input replay of the same saved
state. The difference is only one byte at frame 2 (`0xFFDD1B`, value `1`
instead of `2`), then the object-record bytes remain populated where the
no-input replay had a clear interval at relative frames 39–121. At frame 180,
the fork-versus-no-input differences are concentrated at offsets
`0x60`, `0x82–0x8A`, `0x9B`, `0xA0`, `0xAA`, `0xB0`, `0xBB`, and `0xD1`.

The defensible conclusion is therefore:

> An input event applied immediately after loading the saved state changes the
> subsequent object-record lifecycle relative to the untouched replay, but
> these three modes do not provide evidence that the record responds to
> steering direction. The field semantics and spawn trigger remain unknown.

The likely next discriminating experiment is to add explicit telemetry for
the input port and writer PCs, then fork with a delayed input event at several
relative frames. That will separate “input event changes initialization” from
“object record follows player position” without assigning gameplay meanings to
the bytes prematurely.

## Provenance

All runs used MAME `0.289 (mame0289-dirty)`, loaded
`/tmp/stunrun-latedrive1200.sta`, sampled 256 bytes at `0xFFDD00`, width 8,
and ran without throttle through relative frame 180. Raw snapshots remain
local. Full paths, result hashes, and canonical log hashes are recorded in
[`reference/experiments/stunrun/m5-object-record-input-differential.metadata.json`](../reference/experiments/stunrun/m5-object-record-input-differential.metadata.json).

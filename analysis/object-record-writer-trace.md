# Current object-record writer boundary

The fresh saved-state fork was first traced across main-CPU RAM
`0xFFDD00–0xFFDDFF` for relative frames 1–180. That short trace captured 42
writes, all at relative frames 1 and 2. No writes touched this window at
frames 3–180.

The writers are the existing object-record initialization cluster:

| PC | Role in this trace |
| ---: | --- |
| `0x03D18C`, `0x03D18E`, `0x03D192` | paired record initialization writes |
| `0x03D1B2`, `0x03D1EC`, `0x03D1F4`, `0x03D1FA` | record field/terminator setup |
| `0x03D248`, `0x03D24C` | adjacent record initialization |

The touched addresses are concentrated at
`0xFFDD86–0xFFDDC2`, including the known
`0xFFDD86–0xFFDD91` object/animation neighborhood. Because the current
loaded-state fork performs no later writes, its 49 nonzero bytes are a
snapshot of initialized state rather than evidence of a live spawn/update
cycle.

This explains why steering forks from this state are not useful for assigning
left/center/right semantics in the short run: the steering event does not
reach a writer for this window during the first 180 frames. It does not
explain the older clear/repopulation capture, which remains a separate
execution-condition discrepancy.

## Extended transition and directional layout

Matched 600-frame traces from the same saved state show the live transition.
All three forks perform a 17-word zeroing write at relative frame 220. The
record update loop begins at frame 303 and continues through frame 599. The
snapshot callback can observe the pre-write value at frame 220, so the write
trace—not that single frame's snapshot—is the authoritative clear boundary.

At frames 300 and 600, the center/left/right snapshots differ in five bytes
overall. The union of the directional offsets is six bytes because left and
right use different lanes. Values below are ordered by offsets
`0xFFDD86`, `0xFFDD87`, `0xFFDD88`, `0xFFDD8A`, `0xFFDD90`, `0xFFDD91`:

| Fork | Direction-dependent bytes |
| --- | --- |
| center | `80 80 80 92 6E 00` |
| left | `00 00 01 92 01 02` at frame 300; final two bytes become `01 01` at frame 600 |
| right | `FF FF FE FE 6E 00` |

The surrounding values at `0xFFDD8C`, `0xFFDD8E`, and the later repeated
record bytes remain shared in these samples. Left adds observed writer PCs
`0x03D208` and `0x03D234–0x03D240`; right retains the common writer family.
The static listing identifies `0x03D012` as indexing 16-byte records from
base `0xFFDD86`. Together, the lane-shaped directional differential and the
record indexing support the label **directional object/animation record**.
They do not establish armor, weapon, ammo, score, timer, or player-status
semantics.

## Provenance

The short raw trace remains local at
`/tmp/stunrun-m5-object-writers-current`. The extended matched traces remain
at `/tmp/stunrun-m5-object-writers-current-600-center`, `...-left`, and
`...-right`. They used MAME `0.289 (mame0289-dirty)`,
`/tmp/stunrun-latedrive1200.sta`, the documented input forks, `--nothrottle`,
tap frame 1, and the inclusive address range `0xFFDD80–0xFFDDA0`. Exact
event counts, addresses, writer PCs, and log hashes are in
`reference/experiments/stunrun/m5-object-record-writer-trace.metadata.json`.

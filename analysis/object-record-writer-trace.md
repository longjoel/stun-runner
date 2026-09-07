# Current object-record writer boundary

The fresh saved-state fork was traced across main-CPU RAM
`0xFFDD00–0xFFDDFF` for relative frames 1–180. The trace captured 42 writes,
all at relative frames 1 and 2. No writes touched this window at frames 3–180.

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
left/center/right semantics: the steering event does not reach a writer for
this window during the bounded run. It does not explain the older
clear/repopulation capture, which remains a separate execution-condition
discrepancy. A future experiment must create or load a state immediately
before the suspected gameplay transition, then trace through that transition.

## Provenance

The raw trace remains local at
`/tmp/stunrun-m5-object-writers-current`. It used MAME `0.289
(mame0289-dirty)`, `/tmp/stunrun-latedrive1200.sta`, no scripted input,
`--nothrottle`, tap frame 1, and the inclusive address range
`0xFFDD00–0xFFDDFF`. Exact event counts, addresses, writer PCs, and the log
hash are in
`reference/experiments/stunrun/m5-object-record-writer-trace.metadata.json`.

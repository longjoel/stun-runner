# Checkpoint Schema — Current M1 Contract

The current canonical machine checkpoint is `stunrun-checkpoint/v1`, emitted by
`mame/lua/checkpoint.lua` and captured in
`reference/checkpoints/m1-machine-map/state.json`. The older
`reference/checkpoints/title/state.json` artifact remains preserved as legacy
M0 evidence.

## Canonical fields

These fields are required and compared exactly when the same MAME/ROM baseline
is used:

| Field | Meaning | Status |
|---|---|---|
| `system`, `description`, `mame` | machine identity | exact baseline identity |
| `frame`, `time_seconds` | bounded time identity | exact for the captured run |
| `processors.*.tag` | debugger-visible device tag | exact runtime tag |
| selected processor registers | `pc`, `sr`, `sp`, `st`, `astat`, or `p` | exact captured values |

The checkpoint metadata separately records the ROM manifest, start condition,
script, launch flags, screenshot hash, and selector status. Those provenance
fields are required for comparing artifacts but are not part of the compact
state payload.

## Deliberate omissions

No semantic `game_state`, player/object fields, RAM-region hashes, or command
queue fields are promoted yet. The frame-600 checkpoint is an observed
title/attract boundary, not a proven gameplay selector. Such fields may be
added only with repeatable evidence and an explicit exact-versus-normalized
comparison rule.

This is sufficient for boot/title machine identity and processor-state
reconciliation. The M1 checkpoint now adds an exact ADSP program-region
summary and the machine selector `adsp_program_loaded`, both repeatable across
two independent captures. IRQ-0003 remains open for the semantic extension needed by
reproduction and native gameplay checkpoints. M1 has now supplied repeatable
region/access observations that are eligible inputs to that extension—ADSP
program upload, serial-buffer block/FIFO transfer, and control/IRQ writes—but
no semantic gameplay field has been promoted. The SW1-off replay fixture in
`reference/experiments/stunrun/sw-off-loading-boundary.metadata.json` is a
repeatable loading/blank negative control, not a gameplay path or canonical
numeric checkpoint. The replay harness now supports an exact-frame snapshot
and exit, so future visual and numeric captures can share a frame boundary.

The frame-1800 gameplay fixture in
`reference/checkpoints/m1-gameplay/state.json` uses the same compact schema and
is byte-for-byte repeatable across two fresh configurations. It is a
provenance-backed gameplay boundary, not yet a semantic `game_state` contract.

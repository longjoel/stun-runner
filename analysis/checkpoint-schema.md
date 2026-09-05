# Checkpoint Schema — Current M1 Contract

The current canonical machine checkpoint is `stunrun-checkpoint/v1`, emitted by
`mame/lua/checkpoint.lua` and captured in
`reference/checkpoints/title/state.json`.

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
reconciliation. IRQ-0003 remains open for the semantic extension needed by
reproduction and native gameplay checkpoints.

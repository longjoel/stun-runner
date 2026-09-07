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

## Reusable state forks

A MAME save state is a full-machine checkpoint, not merely a RAM dump. It can
therefore be loaded once and used as the origin for multiple bounded
experiments; the frame counter and input schedule supplied to the tools are
relative to the loaded state. This avoids replaying boot and setup for every
RAM or visual probe.

Create a checkpoint at a requested absolute run frame:

```sh
tools/mame-memory-snapshot /tmp/stunrun-roms-system /tmp/checkpoint \
  --device :mainpcb:maincpu --base 0xff9500 --count 0x86 \
  --frames 900 --targets 900 --input collision_probe_center \
  --save-state /tmp/stunrun-center900.sta --nothrottle
```

Fork it with relative frames and independent inputs:

```sh
tools/mame-memory-snapshot /tmp/stunrun-roms-system /tmp/fork \
  --device :mainpcb:maincpu --base 0xff9500 --count 0x86 \
  --frames 240 --targets 60,120,240 \
  --load-state /tmp/stunrun-center900.sta \
  --input fork_center --nothrottle
```

The installed MAME 0.289 workflow was rechecked from the frame-900 state with
relative targets 10, 20, and 30; all three snapshots completed with return
code zero. Save states remain local artifacts and depend on the pinned MAME
build and validated ROM set. Raw RAM snapshots remain the comparison evidence;
the state supplies the CPU, DSP, video, timer, and peripheral context needed
to resume deterministically.

Recorded `.inp` playback is a separate stream, not a seekable suffix. An
observed launch combining `-state 1` with `-playback r1.inp` loaded the state
successfully but reported one playback frame and applied no recorded events.
Do not treat that combination as continuation from the checkpoint; use a
relative scripted input schedule, or create a dedicated suffix recording from
the loaded state.

Instruction traces can also fork from a saved state without replaying setup:

```sh
tools/mame-trace /tmp/stunrun-roms-system /tmp/object-trace \
  :mainpcb:maincpu 310 mame none 300 \
  --load-state /tmp/stunrun-latedrive1200.sta
```

The trace and its frame counter are relative to the loaded state. The wrapper
stages the state in a private MAME slot and removes that temporary staging
directory after MAME exits.

The frame-1800 rendered-scene fixture in
`reference/checkpoints/m1-rendered-scene/state.json` uses the same compact
schema and is byte-for-byte repeatable across two fresh configurations. The
identical no-input render means it is not a gameplay boundary or semantic
`game_state` contract.

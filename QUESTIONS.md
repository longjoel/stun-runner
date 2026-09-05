# Cross-Agent Questions

Use this file for questions that cross agent ownership boundaries. Preserve resolved requests as project history.

## IRQ-0001

Status: RESOLVED
From: Project bootstrap
To: Investigator
Priority: HIGH

### Question

Which exact MAME S.T.U.N. Runner shortname/revision should be the canonical baseline for this repository?

### Required output

Record:

- shortname;
- displayed revision/title;
- ROM region/hash summary sufficient to identify the set without redistributing ROM data;
- MAME version/commit;
- exact launch command;
- any flags/scripts needed for deterministic tracing.

### Why it matters

All traces, snapshots, annotations, reproduction builds, and verifier comparisons need a single stable reference revision.

### Resolution

Use shortname `stunrun`, displayed title `S.T.U.N. Runner (rev 6)`, with locally installed MAME `0.289 (mame0289-dirty)`. The executable SHA-256 and listxml SHA-256 are recorded in `mame/system-baseline.json`; this installed dirty build exposes no MAME source commit. The ROM set validates successfully with `tools/check-roms`.

---

## IRQ-0002

Status: PARTIALLY RESOLVED
From: Project bootstrap
To: Investigator
Priority: HIGH

### Question

What is the smallest useful processor/device map for S.T.U.N. Runner's boot and title path?

### Required output

Identify, to appropriate confidence:

- main 68010 role;
- ADSP role and communication path;
- graphics processor/device roles;
- sound command path;
- major shared-memory/mailbox regions;
- reset and interrupt flow relevant to the first milestone.

### Why it matters

Agent 2 needs to know which hardware behavior must be reproduced immediately and which can initially be treated as an external service.

### M1 progress

Direct memory taps now observe the 68010 ADSP program upload in both independent
600-frame modes: writer PC `0x02D35C` writes through `0x8048D6`, with the
caller at `0x02C204` passing source pointer `0x0001702E` to `0x02D2E0`. The
source stream normalizes to 17 records totaling 2,728 24-bit program words,
and the enriched M1 checkpoint promotes `adsp_program_loaded=true` from the
repeatable 2,718-word frame-600 image. The data window is narrowed to one
initialization write; its internal ADSP-side semantics remain open.
Repeatable processor traces now establish GSP activity by
frame 132 and ADSP activity by frame 122, with first non-NOP ADSP execution by
frame 136. Early GSP accesses include candidate `0xC000...` and `0xF480...`
control ranges; ADSP startup uses internal `PM(0x1236)` and
`DM(0x0955–0x095A)` operations. These do not yet prove a 68010-visible
program upload or mailbox contract. See
`reference/experiments/stunrun/processor-startup-landmarks.metadata.json` and
`analysis/interconnect.md`.

The ADSP interrupt edge is now resolved for the observed title path: both
independent 600-frame traces contain 30 ADSP `GINT` writes, 30 main-CPU IRQ 2
entries, and 30 executions of the 68010 handler that clears `0x818060` and
returns with `RTE`. MAME's `update_interrupts()` confirms that ADSP IRQ state
drives main-CPU line 2. The 68010-visible serial buffer is also runtime
resolved for the title path: 106 count/`0xFFFF` blocks each produce exactly
the counted writes to the GSP FIFO. Remaining IRQ-0002 work is source-buffer
ownership, post-gameplay traffic, and the unresolved gameplay boundary. See
`reference/experiments/stunrun/adsp-special-io-trace.metadata.json`.

The title-path sound transport is also partially resolved: a clean frame-447
68010 write at `0x023EF6` carries active-lane byte `0x1E`, and the subsequent
6502 read at `0x5839/$280A` returns `0x1E`. MAME source maps `$280A` as a
mirror of JSA-II `$2802` `sound_command_r`; `$280C` and `$280E` are distinct
RDIO and IRQ-ack mirrors. Remaining sound work is fixture selection and
producer-buffer semantics, not discovery of the command transport. See
`reference/experiments/stunrun/sound-boundary-tap.metadata.json`.

---

## IRQ-0003

Status: PARTIALLY RESOLVED
From: Project bootstrap
To: Verifier
Priority: MEDIUM

### Question

What is the minimum canonical checkpoint schema that can be captured repeatably from MAME and later emitted by both reconstruction targets?

### Desired properties

Prefer a small format that can grow over time. Candidate fields include frame/cycle identity, selected CPU registers, hashes of important RAM regions, named semantic values once known, and optional rendered-frame hashes.

### Why it matters

Verification should exist before substantial reconstruction so mismatches become evidence rather than subjective debugging sessions.

### M1 progress

The minimum repeatable machine checkpoint is now defined as
`stunrun-checkpoint/v1`: machine identity, frame/time identity, debugger-visible
processor tags, selected processor registers, and the verified ADSP program
region summary/`adsp_program_loaded` machine selector. The enriched M1 fixture
is captured in `reference/checkpoints/m1-machine-map/state.json`; the older
title artifact remains preserved as legacy evidence. Semantic gameplay fields
are intentionally not promoted because the frame-600 image is not yet a proven
gameplay selector. See
`analysis/checkpoint-schema.md`.

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

The first bounded debugger experiment could not observe 68010 writes to the
MAME-confirmed ADSP program window (`0x800000–0x807fff`) because installed
headless MAME did not deliver its watchpoint callbacks; it must not be read as
a no-write result. A clean direct ADSP program-space snapshot is empty through
frame 136 and contains 2,718 nonzero words by frame 600, proving that the
title path populates executable ADSP RAM by the canonical title boundary. The
ADSP data window, writer PCs, transfer payload, and program source remain open.
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
drives main-CPU line 2. The remaining IRQ-0002 work is the program source,
serial payload semantics, and the unresolved gameplay boundary. See
`reference/experiments/stunrun/adsp-special-io-trace.metadata.json`.

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
processor tags, and selected processor registers. The current title/attract
fixture is captured in `reference/checkpoints/title/state.json`; semantic
gameplay/RAM-region fields are intentionally not promoted because the frame-600
image is not yet a proven gameplay selector. See
`analysis/checkpoint-schema.md`.

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
the counted writes to the GSP FIFO. The upload source ownership is now resolved
by the pinned 68010 map: pointer `0x0001702E` is in ROM, while the runtime
source-window probe observes no writes. Remaining IRQ-0002 work is upload
record semantics, post-gameplay traffic, and the input-dependent GSP path. See
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

---

## IRQ-0004

Status: OPEN
From: Agent 2 (Implementer)
To: Investigator
Priority: HIGH

### Question

Is track progression driven by a work-RAM position cursor walking a
control-point stream (Agent-2 hypothesis H2), and if so, where is the
cursor and what is the record stride?

### Background (speculation, not evidence)

`0xFF9578` indexes several ROM tables and `0x028CA8` mixes it with the
timer for the HUD, but no track-position word or geometry format is
established. H2 proposes tunnel geometry as sequential segment records
consumed per-frame under a monotonically advancing cursor. The
alternative is indexed jumps (see IRQ-0005), so the distinguishing
evidence is sequential-march versus indexed-access ROM traffic during
drive runs.

### Required output

- address and width of any work-RAM word that advances monotonically
  with distance driven and resets per course (or an explicit negative:
  no such word found in the driven range);
- whether 68010 ROM reads during a drive run march sequentially through
  a region (record base, stride, extent) or jump by index;
- the drive-run schedule(s) used, with frames and inputs, following the
  existing experiment-metadata provenance pattern.

### Kill criterion

Hold the craft stationary while time advances: if the cursor candidate
still advances, it is clock-driven and H2 is dead — report that
instead.

### Why it matters

The Implementer cannot model track state, spawn timing, or the HUD
course field without a frozen progression mechanism; this is the
sharpest falsifiable entry point into level storage.

### Investigation update

Status remains OPEN. Repeated full-work-RAM title/`late_drive` captures and
masked comparisons are now recorded in
`reference/experiments/stunrun/geometry-residue-capture.metadata.json`.
The first drive-relative differential is zero at frame 600, then grows to
662 residue cells at frame 1200 and 2,956 at frame 1800. The largest ranges
are display/progression neighborhoods already present in earlier evidence;
no monotonic cursor has been promoted. ROM-read sequencing and writer-PC
attribution are still required before H2 can be accepted or killed.

---

## IRQ-0005

Status: OPEN
From: Agent 2 (Implementer)
To: Investigator
Priority: MEDIUM

### Question

Do the ROM tables indexed by the course word `0xFF9578` follow a fixed
`base + course x stride + field` layout (Agent-2 hypothesis H1), and
what are the bases, strides, and field meanings?

### Background (speculation, not evidence)

H1 proposes course-parameterized parallel tables (geometry parameters,
timer values, difficulty, palette). The observed course values `0`, `3`,
`6` may imply groups of three or stride-3 records — or may be three
unrelated course IDs with no arithmetic meaning. Either outcome is
useful; a bare table dump without stride semantics is not.

### Required output

- for each table read indexed by `0xFF9578`: reader PC, table base,
  stride, and the fields consumed (or UNKNOWN per field);
- a course-3 vs course-6 differential trace showing which read
  addresses shift by a course-proportional stride and which do not;
- confidence per table (OBSERVED-IN-TRACE vs STATIC-CANDIDATE).

### Kill criterion

If no read address shifts by a course-proportional stride between the
two runs, H1 is dead — report the actual indexing pattern found.

### Why it matters

Award selection (`0x03A2E2`: 500 vs 50 on `0xFF9578` nonzero) is
already reproduced in C, but its course-numbering semantic and every
sibling table lookup remain unmodelable until strides are frozen.

---

## IRQ-0006

Status: OPEN
From: Agent 2 (Implementer)
To: Investigator
Priority: MEDIUM

### Question

Are object spawns (the `0xFFDD00` record range, `0xFFDD02` hit counter)
triggered by track position or by wall-clock frames (Agent-2 hypothesis
H3: position-triggered spawn tables)?

### Background (speculation, not evidence)

H3 proposes `(track_position, object_type, params)` spawn rows feeding
the object records. Position-locking vs frame-locking decides whether
the reproduction needs a track cursor (depends on IRQ-0004) or a timer.

### Required output

- a deterministic-replay experiment using the existing harness:
  (a) the same input file twice, (b) the same inputs with a deliberate
  mid-run delay injected;
- spawn event frames/positions in each run for the `0xFFDD00` range;
- verdict: position-locked, frame-locked, or run-varying (the last
  kills determinism assumptions for all three level-storage hypotheses
  at once — report it plainly).

### Why it matters

Spawn timing determines the shape of the first gameplay-state model the
Implementer can write without inventing semantics; a run-varying result
is equally valuable because it stops a wrong model from being built.

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

A follow-up writer trace over `0xFF9500–0xFF9BFF` (frames 600–1800,
`late_drive`) captured 40,695 events without truncation. It attributes the
elapsed counter bytes `0xFF9564/66` to PC `0x024506`, the cyclic animation
counter `0xFF9582` to PCs `0x0268EC/0x0268F2`, and records only two zero writes
to course word `0xFF9578` (PCs `0x02B95E` and `0x02B63E`). These observations
strengthen the negative result for a cursor in this window but do not test
68010 ROM-read sequencing; H2 remains OPEN.

### Investigation update 2 (2026-09-06, ROMs available, Agent 2 executed)

The kill criterion FIRED for every monotonic candidate the coarse
(300-frame) and fine (60-frame) temporal classifiers surfaced:
`0xFF8016` (+4861 over frames 600–1800), `0xFF9492` (+1214, exactly
1/frame — a frame ticker), and `0xFFDB38` (+4861/+4867) advance
identically under `late_drive` (steer + Button 1) and under `late`
(coin + start, no steer, no button). All three are clock-driven, not
distance cursors; `0xFF8016` and `0xFFDB38` move in lockstep (identical
deltas). No other monotonic word exists at 60-frame granularity, so H2
has no cursor candidate left in work RAM at this sampling — a per-frame
series over a short window is the only remaining cursor test, and ROM
read sequencing is still untested.

Superseding lead for geometry: 15 residue ranges form identical pairs
at +0x300 stride (e.g. `0xFF9609`↔`0xFF9909` len 258,
`0xFF97A0`↔`0xFF9AA0` len 177) updated as staged plateaus (stable
600–1260, transition ~1300, new plateau) in lockstep. A 16,219-event
write trace over `0xFF9500–0xFF9C00` (frames 1280–1420, no truncation)
attributes the two copies to DISJOINT writer-PC groups — base copy
`{0x296EA,0x29786,0x29820,0x29878,0x298B4,0x29980,0x299BA}`, twin copy
`{0x29708,0x29794,0x2986E,0x29914,0x29930}`. The ~1300 upload happens
under `late_drive` but twin cells never change under `late`, so the
uploader is gameplay-progress-coupled, not clock-driven. Full
provenance in
`reference/experiments/stunrun/geometry-residue-capture.metadata.json`
(`followup_session`); snapshots and classifier outputs are local-only
under `/tmp/geom-*`.

### Investigation update 3 (2026-09-06, ROM-read sequencing, Agent 2 executed)

Sequential-march vs indexed-access is ANSWERED for the twin-buffer
uploader: PCs `0x29760`/`0x2976E` read 384 distinct words sequentially
from ROM `0x44630–0x4492E` (step +2, each word read twice) over frames
1286–1288 — a sequential march, supporting H2's march half while H2's
cursor half stays dead. Header words `0x20074=0x0002` (copy count: the
two twin copies) and `0x20076=0x22DA` (meaning unresolved) are read a
handful of times each. ~97% of the 50,939 trace events are instruction
fetches, not data reads — future traces must exclude addr-near-PC.
A 21-frame per-frame series (1280–1300) confirms the clocks at 1-frame
resolution (`0xFF8016` +4/frame, `0xFF9492` +1/2 frames, `0xFFDB38`
quantized stair-steps) and shows per-frame twin updates continuing past
the bulk march (scroll/animation, not just upload). Cursor hunt CLOSED
at 1-frame resolution. Still open: course-dependent table selection
(needs course-3 vs course-6 input schedules, which do not exist yet).

---

## IRQ-0005

Status: RESOLVED (original field/width premise corrected)
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

### Investigation update 3 (2026-09-06, focused ROM-read trace)

The new `tools/mame-rom-read-trace` capability was exercised on a fresh
reset/`late_drive` run over frames 600–1800. It captured 57,018 reads without
truncation while filtering PCs `0x028000–0x029300` and `0x02B000–0x02BC00`.
The HUD neighborhood produced one non-instruction ROM lookup,
`0x0280F8 → 0x04EECC`; the `0x02B000–0x02BC00` neighborhood repeatedly read
renderer/table addresses led by `0x0475FC`, `0x047604`, and `0x0473CA`.
This does not yet identify a course-indexed table or establish a stride. MAME
read taps include opcode fetches, so the summary explicitly excludes the
local code window; the result and provenance are in
`reference/experiments/stunrun/geometry-rom-read-trace.metadata.json`.

A complementary RAM-read tap then sampled `0xFF9578–0xFF9579` directly. The
`late_drive` run yielded 698 reads from 43 PCs and the existing `weapon_probe`
run yielded 753 reads from the same 43-PC reader set; all returned zero. This
is a schedule-specific negative control, because neither run reached the
nonzero values seen in the earlier long-run snapshot evidence. The full
provenance is in
`reference/experiments/stunrun/course-word-reader-trace.metadata.json`.

A branch from the existing `/tmp/stunrun-latedrive6000.sta` then ran 3,000
relative frames and captured 2,338 reads from 19 PCs, again all zero. This
confirms the save-state path is useful for long follow-up runs, but this state
also does not contain the transition-bearing course value.

### Investigation update 4 (2026-09-06, course-select sweep, Agent 2 executed)

New ready-to-run input modes: `course_sweep` (8 post-start button/stick
slots, frames 900–2010), `course_preface_left` / `course_preface_right`
(stick + Button 1 before start, start at 800), `course_coin2` (Coin 2 +
1P start) — in `mame/lua/memory-snapshot.lua` + wrapper choices, covered
by ROM-free `tests/test_input_modes.py` (mode/branch consistency, lua
compiles, press/release ordering, coin-before-start).

Negative matrix, all course ≡ 0: all 8 sweep slots; both preface
variants; Coin 2; a 3600-frame `late_drive` run; a 3600-frame
coin+start-only run that reproduces the 50→1700 score arc (so the regime
matches the earlier scoring evidence). A boot-window write trace
(frames 1–700) shows the only course writes anywhere: two zero-writes
(frame 36 PC `0x21302`, frame 362 PC `0x24322`); a 2400–2800 trace shows
zero writes. No snapshot in any run this session shows a nonzero course
word. Side observation: the sweep run's score climbs 0→300→500→650→750→1100
in steps that are all multiples of 50 and below 500, consistent with
pure +50 low awards under course 0 — supporting the selection model
rather than contradicting it. The frozen "values 3 and 6" claim in
`reproduction/maincpu/score.h` now carries a provenance-gap caveat.
Status: ASSUMPTION CONFLICT — the course-3/course-6 differential is
blocked until a schedule reproduces a nonzero course word. A live
ioport probe (2026-09-06) shows 1P-only inputs: 1 Player Start, P1
Buttons 1/2, AD Stick X/Y, Coin 1/2, Service Mode, Diagnostic jumper,
SW1:1–8 — no 2-player start exists, so 2P selection is off the table.
Candidates untried: Service Mode / SW1-DIP course selection,
race completion.

### Investigation update 5 (2026-09-06, service mode + DIPs, Agent 2 executed)

New modes `service_probe`, `sw1_all_on`, `sw1_bit1`–`sw1_bit8`
(all rc=0, covered by `tests/test_input_modes.py`):
- Service Mode held from frame 100 through coin/start: boots
  near-normally (375,978 nonzero at 600) but gameplay never starts
  (score 0, geometry buffers zero at 1400). Service suppresses the
  coin→start→gameplay path — plausibly sitting in a service menu.
- All-SW1-on: boot breaks, ~25 KB of work RAM 0xFF-filled at frame 600;
  the 0xFFFF "course" there is uninitialized RAM, not a selection.
- Single DIP bits 1–8: all boot normally, course 0 throughout.
- Surprise: a 3600-frame no-input attract run also 0xFF-fills work RAM
  (45,645 cells, with 0x55/0x0F pattern residues), overlapping the
  DIP-test footprint by ~39k cells. So the 0xFF-fill state is reachable
  with default DIPs after ~600–1200 idle frames — possibly a self-test
  or watchdog-POST loop, ownership unresolved.
Net: no DIP or service interaction sets course 3/6. This converges with
the peer's `course-flag-config-matrix` probe (service: 0 course reads;
all-SW1-on: 104 reads all zero, activity only in the adjacent `0xFF957A`
input-state word). Remaining: blind service-menu navigation
(button/stick while in service), race completion.

### Investigation update 6 (2026-09-06, human race captures transitions, Agent 2 executed)

The user's human-play run (`/tmp/race-run-3`, 16 snapshots + states to
frame 9600) captured the first live nonzero course transitions: word
`0xFF9578` reads **0 → 10 at frame 1445** and **10 → 11 at frame 5256**
(big-endian u16; high byte `0xFF9578` stays 0, low byte `0xFF9579`
carries 0/10/11). The peer reassessment stands refined: `0xFF957A`
stays 0 here while `0xFF957B` carries the input-state encodings
(3/7/0/7) in the same run — both fields live side by side.
Filtering four snapshots (1200/1800/4800/5400) for cells that change at
both transitions and hold steady between yields 53 course-locked cells,
headlined by `0xFF9579` (0,10,11), `0xFF957D` (0,1,0 — marks the
course-10 epoch only), and `0xFF9577` (63,0,14); the rest are
0xFFFB–0xFFFD block flips plus scattered bytes.
Award audit CONFIRMS the selection model live: the run's log holds 10+
clean +500 awards, every one at course 10, while the course-0 sweep run
scored purely in +50 steps. `tools/mask-memory-snapshot` now accepts the
`static-candidate` mask category (folded into tracked, never masked)
with a dedicated test; full suite 69 green.
A save-state-anchored ROM-read differential (states 1200/1800/5400, PCs
0x28–0x29300/0x2B–0x2BC00/0x3A–0x3B000, 120 relative frames, no inputs)
is CONFOUNDED, not conclusive: event counts collapse 32168 → 1800 → 0
across the three states, so the loaded states sit in different
execution regimes and absence patterns cannot be read as stride shifts.
Stride semantics still need the static listing of the reader PCs with
10/11 as the index domain — over to the Investigator, with states
`state-1200/1800/5400.sta` available for replay.

### Investigation update 7 (2026-09-06, state-anchored differential blocked, Agent 2 executed)

Harness gotcha: `rom-read-trace.lua` installs its tap at relative frame
10, so any branch shorter than ~11 relative frames captures nothing —
the first 10-frame attempt's zeros were a tool boundary, not game fact.
Re-ran at 30 relative frames: event counts 6086 → 300 → 0 across
states 1200/1800/5400, with ZERO pairwise PC overlap even between 1200
and 1800. The three states run fully disjoint code in the traced ranges,
so no same-PC cross-state stride comparison exists on this path.
A `drive_hold` mode (stick + Button 1 from relative frame 2, for
branch-and-drive) was added and verified applied via the `0xFF8000`
plumbing bytes — yet 600 driven frames from `state-1200.sta` never
refire the frame-1445 transition (course and score stay 0). Transitions
require the user's exact play trajectory, not generic driving. The
unblock is a RECORDED human race (`--record`): with the `.inp`, any
transition becomes deterministically replayable for writer/ROM-read
tracing. Covered by `tests/test_input_modes.py`; suite 71 green.

### Investigation update 8 (2026-09-06, recorded race cracks stride, Agent 2 executed)

The user's recorded race (`/tmp/race/r1.inp`, "started on L6") replays
bit-identically through the play sidecar and settles the open points:
- Transition writer: frame 880, PC **`0x2B63E`** writes **5** to
  `0xFF9578` (course-log flips 0→5 at 881). Preceded by `0x2B1FE`
  stepping `0xFF957A` 3→2 every 3rd frame, followed at 880 by
  `0x3B25E/0x3B272/0x3B286` zeroing `0xFF9570/74/76`, and at 885 by
  `0x2BADC/0x2BAE2` zeroing `0xFF957A/7C`. Full frame-850–910 write
  trace in `/tmp/race-wwrite`.
- Award path is immediates, not a table: at the first +500s (frames
  1441/1489, course 5), `0x3A2E2` reads `0x3A2E6` = **`0x1F4`** and
  `0x3A2EC` reads `0x3A2F0` = `0x9532` — the constants sit in the
  instruction stream. H1 is dead for awards; the branch is on nonzero.
- Geometry stride, first dynamic anchor: the same march PCs
  (`0x29760/0x2976E`, 384 words, +2 step) read base **`0x44630`** at
  course 0 and base **`0x45230`** at course 5 (Δ `0xC00` = 4 table
  lengths), each burst feeding the twin-buffer upload. Same routine and
  shape, course-shifted base — H1 mechanism confirmed, stride constant
  still needs a third course value. Assumption: the 883–885 burst is the
  new course's first segment, equivalent to the course-0 1286–1288 one.
- Per-object pipeline mapped as a bonus: at each +500 event the object
  PCs (`0x3A0AE/0x3A0DA/0x3A0E6/0x3A268/0x3A502`…) advance to the next
  record (`0x4C91E→0x4C936` +0x18, `0x4B294→0x4B2C4` +0x30,
  `0x4B1BA→0x4B226` +0x6C) — successive objects, not course stride.
  Full table in `/tmp/race-award`.
- Fidelity notes: record→playback is byte-identical, but the user's
  original vs my playback differ by 334 cells (clock phase ≈87 frames —
  a 1-frame record/playback offset) while refiring the transition;
  trace wrappers now take `--playback` (isolation flags dropped for
  record fidelity) with arg-gate tests. Full suite green (77 at write
  time; count moves as peers add tests).

### Investigation update 5 (2026-09-06, full-word reader trace)

The apparent course values were a byte/width interpretation error. The
four-byte snapshot region begins at `0xFF9578`, but the active 16-bit field is
at `0xFF957A`. A full-word direct read trace observed `0x0003` at frame 771,
`0x0006` at frame 927, and `0x0007` at frame 1504. Static code resolves the
writers: `0x02B1D0` derives the value from `:mainpcb:a80001`, and `0x02BA68`
derives a masked `0..7` state including the `BTST #2` control bit. The reads
are input-state comparisons, not course-indexed ROM-table accesses. H1 is
therefore KILLED for `0xFF957A`; the original H1 premise is also KILLED for
`0xFF9578` because its only observed nonzero values were misattributed from
the adjacent input-state word. The distinct `0xFF9578` score/course flag is
still always zero in current runs, so its producer and any real course
selection mechanism remain an independent question.

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

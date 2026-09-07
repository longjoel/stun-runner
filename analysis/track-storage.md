# How S.T.U.N. Runner stores track data

This is the current human-readable picture of the track/road data path. It is
deliberately split into **observed** facts and unresolved interpretation. The
words in the buffers have not yet been decoded into curvature, elevation, or
segment semantics.

## The short version

The 68010 selects a 384-word table in program ROM, copies it into two 768-byte
work-RAM buffers, applies a slow animation/update pass, and submits the base
buffer toward the TMS34010 GSP through the FIFO at `0xC0000C`.

```text
68010 ROM table (384 × 16-bit words)
              │  +2-byte sequential march
              ▼
base RAM      0xFF9584–0xFF9883  (768 bytes)
twin RAM      0xFF9884–0xFF9B83  (768 bytes, base + 0x300)
              │
              ├─ animation readers/writers touch both copies
              │
              └─ PC 0x02248E reads the base buffer
                   384 source halfword reads
                   192 observed FIFO writes to 0xC0000C
```

The 384→192 relationship is a bus/lane observation. In three independent
late-drive bursts, each FIFO value matched every other source read:
`fifo[i] == source[2*i]`. That is not yet a claim about the logical geometry
record format.

## ROM table selection

The table reader pair at PCs `0x029760` and `0x02976E` performs the same
ascending 384-word march. The observed ROM bases are:

| Observed course/state | ROM base | Evidence status |
| ---: | ---: | --- |
| 0 | `0x044630` | observed in trace |
| 5 | `0x045230` | observed in recorded race |
| 10 | `0x045530` | observed in recorded race |
| 11, 12 | `0x044930` | observed in recorded race |

These bases share a `0x230 mod 0x300` alignment, but they do **not** form one
general course-proportional stride. Several course values reuse table slots.
The native code therefore exposes a finite observed lookup and rejects
unknown values instead of inventing a formula.

The state/index word at `0xFF9578` is written by the normal progression path
at `0x0320BC` and was observed to advance `0→1→2→3→4→5` in a human race.
It controls progression/award behavior, but its displayed track-label meaning
has not been proven. The adjacent input-state fields must not be confused with
it.

## RAM representation

The settled copies are byte-for-byte equal in the saved road-buffer fixture:

- base: `0xFF9584–0xFF9883`
- twin: `0xFF9884–0xFF9B83`
- twin offset: `+0x300` bytes
- table size: `0x300` bytes / 384 16-bit words

The bulk uploader has disjoint writer-PC groups for the two copies. The base
copy also receives a slower animation/update pass, while the twin is used by a
separate animation reader. The bulk upload occurs in staged plateaus rather
than as one permanently stable write; intermediate snapshots can therefore be
mid-update and should not be treated as complete tables.

One saved race checkpoint makes the distinction concrete. After loading
`/tmp/race/state-1200.sta`, the 768-byte base/twin content was unchanged at
relative frames 2, 10, and 30. Its first 720 bytes (words 0–359) exactly match
the ROM slot at `0x045230` (the observed course-5 slot), while only the final
48 bytes (words 360–383) differ. This is strong evidence for a ROM table copy
followed by a bounded runtime tail update; it is not evidence that the whole
RAM buffer is a permanently direct ROM image. The tail's field meaning and
writer remain unresolved.

The long race adds an important qualification: the tail is stable within a
settled interval. Across frames 1200–4200, 5400–7800, and 9000–12600, every
captured tail snapshot has the same 48-byte tail hash for its interval, while
the three interval hashes are different. More importantly, the four observed
ROM slots have an identical 48-byte suffix, but each settled RAM tail differs
from that common ROM suffix. The tail is therefore runtime-derived state
overlaid on a common ROM template, not a per-course ROM payload and not merely
continuously changing animation residue. Its fields remain unidentified, so
“runtime tail” is the safe name rather than a decoded track-record claim.

The longer recorded race shows the same mechanism across transitions. Stable
snapshots match these source slots through word 359:

| Recorded frame range | RAM course word | Matching ROM slot | Tail status |
| ---: | ---: | ---: | --- |
| 1200–4200 | 10 | `0x045530` | words 360–383 differ |
| 5400–7800 | 11 | `0x045230` | words 360–383 differ |
| 9000–12600 | 12 | `0x044630` | words 360–383 differ |

Snapshots at frames 4800, 8400, and 13200 are transition/in-flight states and
do not match a complete slot. This explains why a narrow ROM-read trace around
the nominal transition frame can miss the load: the useful source identity is
more reliably recovered from the settled RAM buffer immediately afterward.

The write trace around the first course-10 load separates the two mechanisms,
and the static listing explains the operation:

- PC `0x029760` writes the 24-word base tail during the table copy.
- PC `0x02977E` writes the corresponding twin-copy data.
- PC `0x0298C0` then writes the observed 24-word base-tail slice at frames
  954, 955, 958, 960, 962, 964, and 967.

The `0x0298C0` writes show a repeated-byte sequence descending from roughly
`0xA3` toward `0x19`, with a distinct final record. This is consistent with a
time/ramp or animation update, but its logical meaning is still unknown. The
listing shows that `0x0298C0` is actually the body of a 768-byte loop:
`base[i] = base[i] - twin[i]` for each byte. The paired loop at `0x0298FA`
performs `base[i] = base[i] + twin[i]`. Thus the trace sees only the tail
because that was the selected address window; the animation routines are not
tail-only routines.

Full-window write taps make the ownership unambiguous. During the course-10
replay, `0x0298C0` produced 5,376 lane events over the complete base window
(`0xFF9584–0xFF9882`) and zero writes in the twin window; the paired
`0x0298FA` pass produced the same 5,376/zero-twin shape. Each is therefore a
repeated full-buffer base transform, with the apparent tail ramp caused by the
selected bytes' values rather than by a tail-specific writer.

The table-copy listing also shows that the base loop copies raw source bytes,
while the twin loop divides each source byte by a stack parameter before
storing it. In the settled race snapshots that parameter is `1` at
`0xFF954E`, which explains the observed byte-for-byte twin. The storage
mechanism therefore supports a scaled twin even though the current fixtures
exercise the identity case. The static caller at `0x02446C` visibly pushes
`1` as that loader's divisor argument before calling `0x0296AA`; other callers
must not be assumed to use the same value without a corresponding trace.

The same state machine uses `0xFF954E` as a countdown gate: for example,
`0x02991C` decrements it and calls the subtract loop while it remains nonzero,
while `0x029ADE` decrements it and calls the add loop. The startup path loads
`0x78` there before entering the roadway path. This is a transform/ramp timer
candidate, not the separately established HUD time-remaining field at
`0xFF9568–0xFF956B`.

Snapshot correlation supports that role without assigning a complete semantic
name: in the long race, settled course buffers have `0xFF954E = 1` and
`0xFF9550 = 1`, while transition snapshots show nontrivial timer/mode pairs
(`0x0005/0x001A` at frame 4800 and `0x008A/0x0017` at frame 8400). Those
transition buffers are also the snapshots that fail to match a complete ROM
slot. The values are useful selectors for future experiments, not proof that
`0xFF9550` is a particular gameplay mode. Static XREFs show that `0xFF9550`
is a broader game-state dispatch latch; its involvement in the road transform
is one consumer of a shared state machine.

The byte arithmetic is now represented literally in
`reproduction/maincpu/road_buffer_math.c`. It is a standalone slice rather than
part of the fixture shell: the shell's captured geometry input represents a
pre-transform buffer, while this C slice models the later subtract/add phase
with 8-bit wraparound and a configurable byte count. The same slice now models
the preceding raw-base/scaled-twin copy loop, including divisor values other
than the observed `1` case. Its focused native test is
`road-buffer-math-slice-c`.

For future RAM work, `tools/analyze-track-tables` also accepts repeated
`--snapshot PATH` arguments. It reports the best matching ROM slot, exact and
different word ranges, whole-buffer and settled-tail hashes, and base/twin
differences; when applicable it also reports the common ROM-tail hash. This
makes
settled-versus-in-flight classification repeatable from ordinary
`stunrun-memory-snapshot/v1` artifacts.

The companion `tools/diff-memory-snapshot` confirms the transition boundary in
the same recording: the track window changes by 0 bytes from frames 1200→1800,
by 1,221 bytes from 4800→5400, and by 182 bytes from 8400→9000. The first is
a stable roadway interval; the latter two are transition updates affecting
the base/twin storage, not merely a course-word display change.

At the changed-offset level, the 4800→5400 update changes 603 shared base/twin
offsets plus 15 twin-only offsets; no base offset changes without its twin.
The 8400→9000 update changes 91 shared offsets with no asymmetric footprint.
This supports synchronized copy/update work with occasional twin-side residue,
not an independent base-only rewrite during these transition windows.

The reusable snapshot workflow is:

1. Load a recorded race checkpoint with `tools/mame-memory-snapshot`.
2. Capture `0xFF9584` for `0x600` bytes at a settled frame.
3. Run `tools/export-geometry-native-state`.
4. The exporter verifies the `+0x300` twin and emits a 768-byte big-endian
   fixture for the native geometry boundary.

No commercial ROM bytes or raw RAM dumps belong in the repository; the
experiment metadata records provenance and local paths only.

## What the GSP submission tells us

The road consumer at PC `0x02248E` reads the base buffer sequentially. A fresh
late-drive trace saw complete bursts at frames 1290, 1293, and 1297. The
recorded-race replay saw repeated bursts from frames 1085 through 1293,
including a split burst across frames 1088–1089.

Separate read/write taps, using the same deterministic schedule, matched the
FIFO payload at frames 1290, 1293, and 1297. Other writers also use
`0xC0000C`, so the matching road subsequence must be identified by PC and
position within the broader FIFO stream.

The GSP receives the values through the MAME-confirmed GSP I/O window
`0xC00000–0xC03FFF`; `0xC0000C` is the narrow runtime FIFO sink. The GSP's
interpretation of these road words remains open.

## A visible pattern inside one ROM table

The traced course-0 table at `0x044630` is 384 words long. Looking only at
word boundaries, its tail has a particularly clear shape:

```text
words 0–143       prefix / mixed data; structure not yet identified
words 144–191     16 records × 3 words
words 192–239     16 records × 3 words
words 240–287     16 records × 3 words
words 288–335     16 records × 3 words
words 336–383     16 records × 3 words
```

Thus the final 240 words are five adjacent arrays of sixteen 6-byte records.
Several fields show smooth or interleaved ramps, and the last block contains
repeated `0x00BA`/`0xBA00`-like values. Those patterns make a coordinate,
lookup, or rasterization structure plausible, but they do not identify which
word is X, Y, width, curvature, a flag, or a terminator. The current confidence
label is **ROM-DATA-SHAPE OBSERVED; FIELD SEMANTICS UNKNOWN**.

The first 144 words also contain repeated ramps and packed-looking values, but
no equally safe record boundary has been promoted there. The same structural
analysis should be repeated for the observed course-5/10/11 table bases before
assuming that all table slots use identical layouts.

### Cross-slot ROM comparison

The local, MAME-validated main-CPU ROM set makes it possible to compare the
slots directly without committing their contents. The four observed windows
are not four unrelated formats:

```text
words   0–23       identical in all four slots
words  24,27       differ only in the course-0 slot
words  25–26,28–47 identical in all four slots
words  48–143      slot-specific payload (three 16-record blocks)
words 144–383      identical in all four slots (five 16-record blocks)
```

In other words, 286 of 384 words are identical across course 0, 5, 10, and
11/12. The course-5, course-10, and course-11/12 slots share all 288 words
outside the 96-word variable payload; course 0 has two additional differences
in the otherwise shared prefix. This supports a cautious storage model of
**common header + slot-specific payload + common command/geometry suffix**.
It does not prove that the 96-word region is the whole track identity, nor
does it assign X, Y, width, curvature, flags, or terminators to any field.

The strongest safe conclusion is therefore that table selection changes a
bounded payload inside a largely shared 768-byte command/data template. The
record-shaped 16×3 grouping remains an observed byte layout, not a decoded
semantic record type.

### Follow-up comparison attempt

The existing long-play recording was replayed while filtering directly for all
four observed 384-word ROM slots. The first table load was reproduced at
`0x045530` (course 10): both reader PCs fetched the complete 384-word range at
frame 953. Targeted windows around the later recorded transitions (frames
4300–4700 and 7800–8200) produced no reads from `0x044630`, `0x044930`,
`0x045230`, or `0x045530`.

That negative result is window-specific, not evidence that those slots are
unused. The broader 925,004-event `race2` march survey later observed the same
reader PCs touching slot `0x044630` at frames 5975 and 6049. This reinforces
the settled RAM mapping while showing why a narrow transition window is a
poor proxy for the complete loader lifecycle. The remaining uncertainty is
the segment-to-slot selection sequence, not whether later slot loads can
occur. The ROM tracer accepts `--address-range` filters so this question can
be repeated without collecting the entire program-ROM fetch stream.

## What is still unknown

- Which word or bit fields describe lateral position, height, width, or
  curvature.
- Whether one 384-word table is one segment, a ring of segments, or a command
  block with mixed metadata and coordinates.
- Why the producer selects each shared ROM slot for every progression state.
- The exact timing/ownership of the base-only animation pass.
- Whether other gameplay regimes use a different consumer schedule.
- How the GSP FIFO words become road pixels and how they combine with the
  already decoded VRAM/palette path.

Those questions remain explicitly open; the current native slices model only
the observed copy, lane, and transport mechanisms.

## Evidence index

- `reference/experiments/stunrun/geometry-residue-capture.metadata.json` — ROM
  march, copy writers, staged updates, and table validation.
- `reference/experiments/stunrun/m5-save-state-road-buffer-fixture.metadata.json` —
  repeatable saved-state RAM fixture.
- `reference/experiments/stunrun/m5-road-buffer-consumer-trace.metadata.json` —
  base-buffer consumer reads.
- `reference/experiments/stunrun/m5-road-buffer-recorded-replay.metadata.json` —
  recorded-race replay bursts.
- `reference/experiments/stunrun/m5-road-fifo-lane-differential.metadata.json` —
  384-read/192-write lane match.
- `reference/experiments/stunrun/m5-track-table-slot-comparison.metadata.json` —
  targeted cross-slot replay, ROM-content comparison, and its negative
  later-window result.
- `reproduction/maincpu/geom_upload.h` and `road_fifo.h` — native literal
  mechanisms and their provenance comments.

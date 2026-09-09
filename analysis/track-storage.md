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

The static lookup at ROM `0x047406` makes that reuse explicit. The word at
`0xFF9578` is used as a zero-based index into 23 longwords:

| State/index | ROM table base | State/index | ROM table base |
| ---: | ---: | ---: | ---: |
| 0 | `0x044630` | 12 | `0x044630` |
| 1 | `0x044930` | 13 | `0x044930` |
| 2 | `0x044C30` | 14 | `0x044C30` |
| 3 | `0x044F30` | 15 | `0x044630` |
| 4 | `0x045530` | 16 | `0x044930` |
| 5 | `0x045230` | 17 | `0x044C30` |
| 6 | `0x044630` | 18 | `0x044F30` |
| 7 | `0x044930` | 19 | `0x045530` |
| 8 | `0x044C30` | 20 | `0x045230` |
| 9 | `0x044F30` | 21 | `0x044630` |
| 10 | `0x045530` | 22 | `0x044930` |
| 11 | `0x045230` |  |  |

This is the missing storage-level explanation for the runtime observations:
state 10 selects `0x045530`, state 11 selects `0x045230`, and state 12 wraps
back to `0x044630`. The entries at `0x044C30` and `0x044F30` are proven ROM
references but have not yet been promoted as runtime-observed settled buffers.
The selector is therefore a finite state-to-slot map, not a stride formula and
not evidence that each state owns a unique table.

The native/reproduction boundary exposes this literal map through
`stunrun_geom_selector_table_base()`. It returns the ROM address for all 23
indices, but deliberately does not load ROM data or assign a semantic name to
the index. The older `stunrun_geom_observed_table_base()` helper remains the
smaller four-state runtime-observed contract.

The corresponding listing path at `0x024A14–0x024B40` bounds the index to
`4..0x16`, uses a second ROM-side nine-byte-per-state table at `0x048180` for
another resource selector, and then reads the road-table pointer from
`0x047406` before calling `0x0297A6`. It finally stores `0xFF9584` in
`0xFFDB3C`, the active road-buffer pointer. That is direct evidence that the
table copy and the later road consumer share the base-buffer contract; the
second selector's semantic role remains open. More specifically, the code
uses `state * 9` to select a byte from `0x048180`, scales that byte by four,
and indexes a separate longword table at `0x048068`; that pointer is passed as
another argument to the transition routine. The road-table argument in the
same call is independently loaded from `0x047406`. Thus `0x048180` must not be
reported as a second geometry-table array merely because it is indexed by the
same state word.

The state/index word at `0xFF9578` is written by the normal progression path
at `0x0320BC` and was observed to advance `0→1→2→3→4→5` in a human race.
It controls progression/award behavior, but its displayed track-label meaning
has not been proven. The adjacent input-state fields must not be confused with
it.

## Live ROM-read sequencing

A bounded replay of `/tmp/race/r1.inp` traced the main-CPU ROM reads around the
table-copy routines (`0x029760`, `0x02976E`, and `0x0297F4`) while restricting
addresses to `0x044000–0x046000`. The capture was not truncated: 5,856 bus
events were recorded from frames 900–4800. MAME reports duplicate lane events
for some 16-bit reads; after collapsing adjacent duplicates, every observed
run advances by exactly two bytes with no backward jump or non-sequential
address.

The first copy pass at frames 1082–1083 reads the contiguous range
`0x044630–0x04492E` through both copy-loop PCs. The same table is revisited
after the transition at frames 1324–1325. Runtime update passes at
1139–1141 and 1298–1300 also march forward from `0x044630`, but only partial
prefixes are visible because those passes span frame boundaries. Later passes
at frames 3288–3289 and 3361–3362 read the contiguous `0x044930` slot. These
observations directly support a 384-word, `+0x2` source march for the table
loader and show slot reuse during one recorded race.

This is useful negative evidence for the open “one RAM cursor walks a single
ROM control-point stream” hypothesis: the observed table-loading traffic is
slot-selected and sequential per load, rather than a single long-lived ROM
address that advances with every frame. It does **not** rule out an indexed
cursor over the copied RAM buffer, nor does it decode the words into curvature,
elevation, or segment records. The raw result hash and exact command are
preserved in
`reference/experiments/stunrun/m5-track-rom-read-sequence.metadata.json`.
The reduction is repeatable without ROMs through
`tools/analyze-rom-read-sequence <result.json>`; it reports the per-frame/PC
source-event count, collapsed address count, contiguous runs, and any
non-sequential break.

## The course-transition dispatcher (static evidence)

The code around `0x024CE4` is the clearest static bridge between progression
state and table loading found so far. In the relevant branch it:

1. calls `0x03DF36` with `0xFF9532`, `D2`, and a count of `10`;
2. stores the returned pointer in `0xFF951A`;
3. if the return is nonzero, selects ROM table `0x044630` through
   `0xFF9558`, clears the table-copy counters, and calls `0x0297A6`;
4. later calls `0x03E5C4` with `0xFF951A` and `0xFF9532`, then conditionally
   calls the next table/transition routine at `0x0296AA`.

This is useful control-flow evidence, but it is not yet a decoded
`course = table[index]` formula. `0x03DF36` walks a list of 24-byte records
backward from a caller-supplied base, comparing each record's first longword
against its first argument, and returns a record pointer or zero. The meaning
of the caller's `D2` base and the records' remaining fields is still unknown.
The nearby `0x03E5C4` routine is primarily a display/HUD state routine; its
call to the object-record renderer does not make it a track-table consumer.

The runtime traces collected so far do not reach `0x024CE4`, `0x03DF36`, or
`0x03E5C4` during the saved late-drive/object checkpoints. That is consistent
with those checkpoints being inside an already-running roadway state, not
evidence that this dispatcher is dead. The next clean experiment is to save
just before a recorded course transition and trace the dispatcher while
retaining the input recording; loading a state without the corresponding
replay input does not reproduce the transition.

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

Read taps close the arithmetic contract: PC `0x0298BE` read the complete twin
window 5,376 times on the same eight subtract-pass frames, and PC `0x0298F8`
did the same on the eight add-pass frames. The observed pairs are therefore
literal `base[i] = base[i] - twin[i]` and `base[i] = base[i] + twin[i]`
passes, with 8-bit bus-lane writes, rather than an inferred relationship from
the final snapshots alone.

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
`stunrun-memory-snapshot/v1` artifacts. When the supplied ROM manifest maps
the selector region, the same report now emits the complete 23-entry
`0x047406` state-to-table lookup under `state_table_selector`.

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

A narrowed RAM-read trace resolves the apparent twin-buffer ambiguity. Each of
the three complete `0x02248E` bursts reads 384 unique words from
`0xFF9584–0xFF9882` and zero words from `0xFF9884–0xFF9B82`. The twin is not an
alternate input to this FIFO drain. Instead, `0x0298BE` reads the twin while
`0x0298C0` reads the base in the paired animation/transform path; both paths
can split across adjacent frames. The safe model is therefore:

```text
ROM table → base + twin working copies
                 ├─ 0x0298C0 / 0x0298BE: paired animation/transform reads
                 └─ 0x02248E: base-only road FIFO drain
```

Separate read/write taps, using the same deterministic schedule, matched the
FIFO payload at frames 1290, 1293, and 1297. Other writers also use
`0xC0000C`, so the matching road subsequence must be identified by PC and
position within the broader FIFO stream.

The GSP receives the values through the MAME-confirmed GSP I/O window
`0xC00000–0xC03FFF`; `0xC0000C` is the narrow runtime FIFO sink. The GSP's
interpretation of these road words remains open.

### Road-buffer to display timing boundary

A paired deterministic `late_drive` capture sampled the main road buffer and
GSP VRAM at the same frame landmarks around the three observed FIFO bursts.
The counts are useful as a timing observation, not as a pixel-field decode:

| Interval | Main base-buffer bytes changed | GSP VRAM words changed |
| --- | ---: | ---: |
| 1288 → 1290 | 621 / 768 | 16,696 / 32,768 |
| 1290 → 1293 | 606 / 768 | 0 / 32,768 |
| 1293 → 1297 | 606 / 768 | 3,306 / 32,768 |

The frame-1290 road burst therefore lands near a large GSP-visible update, but
the continuously changing main buffer does not imply a same-frame VRAM change:
the GSP state is identical from 1290 through 1293, then changes again by 1297.
This supports a buffered or display-scheduled consumer boundary. It does not
identify which words control curvature, width, or scanline placement, and the
capture does not prove that every changed VRAM word came from the road FIFO.
The current label remains **ROAD FIFO → GSP DISPLAY TIMING OBSERVED; FIELD
SEMANTICS UNKNOWN**.

The lane-level payload check adds one more boundary. At frames 1290, 1293,
and 1297, the 192 road FIFO values each match the even-indexed words of the
same-frame 384-word base buffer, exactly. In the mixed FIFO trace those
subsequences begin at event offsets 9,347, 12,691, and 16,036. The 192 payload
values have no broad verbatim representation in the corresponding 32,768-word
GSP VRAM snapshots (zero or one value present, depending on the frame), so the
road stream is not simply copied into visible VRAM. The safe mechanism model is
now **base buffer → every-other-word FIFO lane → GSP-side interpretation →
display memory**; the GSP-side command/geometry format remains unknown.

A bounded GSP program-space write trace further qualifies the display side. Four
writer PCs touch the low VRAM aperture during frames 1288–1296:

| GSP PC | Literal/runtime role | Observed cadence |
| --- | --- | --- |
| `0xFFF454E0` | broad VRAM writer; operation not decoded | frames 1288, 1290–1296 |
| `0xFFF43030` | `FILL L` | frames 1289 and 1293 |
| `0xFFF46590` | `PIXBLT B,XY` | frames 1288, 1292, 1296 |
| `0xFFF47AB0` | paired VRAM writer; operation not decoded | frames 1288, 1292, 1296 |

The periodic pixel-blit pair is phase-shifted from the main-CPU road FIFO bursts
at 1290, 1293, and 1297. This is consistent with a shared GSP display scheduler
or command queue. It is not evidence that any one writer consumes the road
table; a road-specific producer/consumer pairing remains open.

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
words  48–143      slot-specific payload (record boundary not promoted)
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

The static selector names two additional slots, `0x044C30` and `0x044F30`,
that were not present in the first four-slot runtime comparison. A six-slot
ROM comparison shows they use the same layout: 286 of 384 words are common,
with only words 24, 27, and 48–143 varying. This expands the storage model
without expanding the runtime claim: the two slots are structurally proven
track-table candidates, but their live use still needs a trace or settled RAM
match.

A bounded `late_drive` startup probe over frames 800–1700 found no reads from
the alternate-slot window, and a wider positive-control window also found no
table-copy reader events. The schedule therefore does not reach this loader
in that interval. This is retained as a path/window negative only; the
selector's alternate targets remain runtime-unconfirmed rather than unused.

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

### Full settled snapshot sweep

The retained `race2` playback was also checked at 22 six-hundred-frame
landmarks against all six statically identified slots. Once the buffer had
settled, the observed sequence was:

| Settled frames | Best ROM slot | Exact RAM words | Runtime tail |
| --- | --- | ---: | --- |
| 1200–4200 | `0x045530` | 0–359 | 360–383 |
| 5400–7800 | `0x045230` | 0–359 | 360–383 |
| 9000–12600 | `0x044630` | 0–359 | 360–383 |

Frames 4800, 8400, and 13200 are transition observations rather than settled
selectors: they differ from their eventual best slot in 361, 47, and 47 words
respectively. Frame 600 is an early/non-settled negative. This strengthens the
course-to-slot sequence as a RAM-backed observation and gives the native
boundary a repeatable settled input, but it does not decode the 24-word tail or
any individual coordinate/width/curvature field. The complete command,
result hash, and frame-by-frame classification are in
`reference/experiments/stunrun/m5-track-table-snapshot-sweep.metadata.json`.

For human inspection, `tools/track-visualizer/index.html` is an offline,
single-file viewer for one or more `stunrun-memory-snapshot/v1` captures. Drop
in snapshots containing `0xFF9584–0xFF9B83` to scrub the base/twin windows,
compare adjacent frames, and inspect individual byte lanes. Its readout labels
the course and score addresses as candidates/observations rather than
asserting semantics.

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
- `reference/experiments/stunrun/m5-road-transfer-twin-read.metadata.json` —
  narrow proof that the FIFO drain is base-only and the twin belongs to the
  paired animation/transform path.
- `reference/experiments/stunrun/m5-track-table-slot-comparison.metadata.json` —
  targeted cross-slot replay, ROM-content comparison, and its negative
  later-window result.
- `reproduction/maincpu/geom_upload.h` and `road_fifo.h` — native literal
  mechanisms and their provenance comments.

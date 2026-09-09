# GSP FIFO-fed work-buffer consumer

The recorded race now has a matched GSP instruction trace for the road-buffer
window. The trace is runtime evidence from the installed MAME build; the
original GSP listing is still not useful at this address because the saved
state image is populated dynamically.

## Observed entry and loop

The GSP routine beginning at `0xFFF48C20` is entered 15 times in the recorded
race window (relative frames 1280–1300), compared with 5 entries in a matched
no-input run. Its setup is:

```text
FFF48C40: ANDI FFh,A4
FFF48C70: MOVE A4,A0
FFF48C80: SLL  4h,A0
FFF48C90: MOVE A0,A1
FFF48CA0: ADDI FFF6F650h,A0
FFF48CD0: ADDI FFF70650h,A1
FFF48D00: MOVE *A5+,A6,0
FFF48D20: SRL  1h,A2
FFF48D30: INC  A2
```

The loop at `0xFFF48D50` reads from the FIFO-fed source pointer `A5`, writes
through both `A0` and `A1`, and combines adjacent byte-lane reads using `SLL
8h` and `OR`. The indexed source read at `*A5(10h)` and the final `ADDK 8h,A5`
are retained literally; their bus-unit interpretation is not promoted here.

This establishes an input-dependent GSP FIFO-to-work-buffer consumer and two
destination-buffer families. It does not establish that either buffer is
road geometry, nor does it identify the meaning of any payload word.

## Destination timing check

Three snapshots of each destination were taken at frames 1280, 1290, and
1300 in matched `late_drive` and no-input runs. Between frames 1290 and 1300,
the driven run changed 235 of 256 sampled words at `0xFFF6F650` and 195 of
256 at `0xFFF70650`. The no-input run changed 1 and 0 words respectively.
This timing correlation strengthens the consumer attribution while leaving
the buffer contents and field semantics UNKNOWN.

The literal setup is now represented by the dependency-free C slice in
`reproduction/gsp/work_buffer_consumer.c`. It exposes the masked selector and
the two `+0x10` destination strides, plus the observed byte-lane composition.
The `gsp-work-buffer-consumer-slice-c` test passes. This is a transport/layout
contract for the native boundary, not a decoded road renderer.

A PC-filtered write trace strengthens the loop shape. Each destination writer
produced 896 full-word writes in seven 128-write bursts at frames 1280, 1283,
1286, 1289, 1292, 1295, and 1298. The base-side writer covered 128 addresses
from `0xFFF6F650` through `0xFFF70630` at `0x20` address steps; the twin-side
writer covered 128 corresponding addresses from `0xFFF70660` through
`0xFFF71640`. The differing initial offsets are retained as observed selector
state, not assigned a semantic buffer role.

The two output streams must not be modeled as identical copies. In the first
captured burst, corresponding writes include base `0xBD08` versus twin
`0x6565`, and only 21 of 896 same-position write pairs have equal data. The
trace supports shared loop structure and cadence, but the source reads or
lane state feeding `A0` and `A1` remain distinct. Any native model must keep
those streams separate until the source-side relationship is proven.

The next GSP routine, entered from `0xFFF42260`, copies the work-buffer
families onward. `0xFFF4AA10` sets `A0=0xFFF6F650`, `A1=0xF5000000`, and
`A2=0x80`; after the first loop, the second pass uses `A1=0xF5800000` while
continuing the source pointer. Direct write taps captured 2,048 full-word
writes to each destination over eight bursts at frames 1280, 1281, 1284,
1287, 1290, 1293, 1296, and 1299. This is a stronger work-buffer-to-display-
memory boundary, but the destination device layout and payload semantics are
still UNKNOWN.

An ordinary GSP memory-read probe over the first downstream destination
(`0xF5000000–0xF5000FFF`) observed zero readback events in the same replay
window. The pinned `multisync_gsp_map` identifies this range as the
`hdgsp_paletteram_lo_r/w` palette plane; the twin range is
`hdgsp_paletteram_hi_r/w`. The negative read result is therefore an
instrumentation/readback limitation, not evidence that the destination is an
unowned or inactive window. The exact event-level copy trace remains the
authoritative transport evidence.

Exact-race snapshots of both downstream families show 232/256 base words and
190/256 twin words changing from 1280→1290, with neither changing from
1290→1300. Those snapshot captures came from separate MAME invocations and
must not be used for same-frame source/destination value pairing. The paired
read/write event traces instead match all 2,048 source values to all 2,048
destination values across the two copy passes. The literal copy is therefore
raw at the traced event boundary; destination device interpretation remains
UNKNOWN.

The 256-word sequential copy is now represented by
`stunrun_gsp_display_copy()` in the dependency-free C slice. Base and twin
callers provide separate source arrays; the function copies exactly the
observed bounded transfer count and rejects shorter buffers. Its test is
covered by `gsp-work-buffer-consumer-slice-c`.

## Replay support

`tools/mame-trace` now accepts `--playback INP` in the same positional slot as
`--load-state`. Playback requires `INPUT=none`, passes the recording through
MAME's native input replay path, and suppresses scripted inputs. This makes
recorded races reusable for CPU instruction tracing.

`tools/mame-memory-snapshot` now accepts the same `--playback INP` form. A
real MAME smoke run replayed `/tmp/race/r1.inp` and captured the requested GSP
range at frame 30, so exact recorded input can now drive both instruction
traces and memory snapshots.

The exact `/tmp/race/r1.inp` replay also has a combined 512-word snapshot
fixture covering both families. From frames 1280→1290, 232 base words and 190
twin words change; from 1290→1300, both are stable. This replay-backed series
is the preferred input for future producer correlation because it avoids a
synthetic schedule.

The provenance record is
`reference/experiments/stunrun/m5-gsp-road-fifo-consumer.metadata.json`.

## High-memory producer check

The saved-state renderer forks were then used to check the upstream producer
without replaying the long race. A GSP program-space write trace over
`0xFFF80000–0xFFFFFFFF`, relative frames 1–10 from the common
`/tmp/stunrun-latedrive2400.sta` checkpoint, captured 1,878 events for both
`fork_hold_left` and `fork_center`. The complete event sequences were
byte-identical, including writer PCs and values. This is a useful negative
result: the left/center visible difference is not caused by a new high-memory
record write in that renderer window. It selects different preexisting
records through the already documented indexed read path.

The probe therefore moves the native producer target earlier than the
renderer fork. It does not identify the record writer or assign semantics to
the high-memory fields. Provenance is in
`reference/experiments/stunrun/m5-gsp-fork-high-write-probe.metadata.json`.

A second bounded probe starts from the title-era `state-600.sta` checkpoint and
samples the same high-memory window through relative frame 60. Only five of
32,768 stride-16 samples change. The accompanying write trace has 34,518
events, but its dominant PCs are register-save/restore and renderer routines;
it does not expose a bulk record upload. These cells are retained as
unresolved state deltas, not promoted track fields. Provenance is in
`reference/experiments/stunrun/m5-gsp-title-high-write-probe.metadata.json`.

The power-on follow-up supplies an earlier timing lead, with an important
qualification. A bounded 600-frame high-memory write trace first sees any
writes at frame 78; the first nonzero burst at frame 405 is 6,454 events, but
6,332 of them come from the broad `0xFFF59A50` initialization/clear path over
`0xFFF80000–0xFFF98BB0`. It is not evidence of a geometry-record upload.
The stronger recurring lead is a pair of 64-word transactions at
`0xFFF9FC00` and `0xFFFCFC00`, serviced by the `0xFFF41060` sentinel-dispatch
sequence and pointers near `0xFFF716A0`/`0xFFF71670`. The pinned driver map
shows these addresses are inside the mirrored shared `gsp_vram` backing, so
they should be called VRAM-backed staging/command areas rather than separate
queue devices. Their payload and semantics remain UNKNOWN. Provenance is in
`reference/experiments/stunrun/m5-gsp-queue-dispatch-probe.metadata.json`.

## Save-state-matched source/write correlation

The high-memory window was then traced during the exact visible-fork interval,
relative frames 1790–1800, from the common `late_drive` checkpoint. The read
traces at `0xFFF45A10` and `0xFFF45A40` identify the high-memory source
addresses consumed by the two `FILL XY` coordinate streams. Every one of those
source addresses also received a write event in the corresponding GSP write
trace:

| Fork | High-memory writes | Unique written addresses | Read-source addresses also written |
|---|---:|---:|---:|
| center | 19,861 | 13,747 | 3,044 / 3,044 |
| left | 18,440 | 14,232 | 3,064 / 3,064 |

The write footprints are state-dependent. The center trace includes 869 writes
from the decoded `LINE 0` primitive at `0xFFF45DD0`; the left trace has a
different writer mix and no comparable dominant line count. This means the
records feeding the coordinate loads are actively generated or rewritten by
the GSP display pipeline during the visible fork, rather than being a static
road-table copy waiting unchanged in high memory.

This is a producer-boundary result, not a semantic decode. The records may
represent roadway, objects, or other visible geometry; their ownership and
field meanings remain UNKNOWN. The native target should therefore preserve
the high-memory record source as a separate input to the geometry primitives
until a writer/consumer pair is tied to one visible region.

The frame correlation gives the producer boundary a timing shape. There were
zero same-frame write/read pairs for the source addresses in either fork. In
the center branch, the nearest write for the read addresses was three frames
earlier for 1,560 reads and five frames later for 746 reads on the A7 stream
(the A9 stream has the corresponding 1,136/544 split). The left branch has the
same dominant three-before/five-after pattern, with smaller four-, six-, and
four-frames-before transition groups. Because the write and read traces are
separate captures, this is frame-level correlation rather than instruction
ordering, but it is consistent with an 8-frame staged/ring pool. It is not
consistent with a producer overwriting the exact consumed record in place on
the read frame.

A neighboring center-fork interval validates the cadence. At relative frames
1810–1820, the A7/A9 coordinate readers consumed 2,802/2,034 source words and
the high-memory write trace contained 21,509 events. Again, no source address
was written on its read frame; nearest writes were two frames earlier for
1,846/1,338 reads and six frames later for 788/572 reads, with only small
three-before/five-later edge groups. The phase shifts while the eight-frame
spacing persists, which is the expected shape of a rotating staged pool and
not a fixed one-frame producer delay.

This correlation is reproducible with the ROM-free analyzer:

```sh
tools/analyze-gsp-read-write-correlation \
  /tmp/stunrun-m5-gsp-high-write-center-1790/result.json \
  /tmp/stunrun-m5-gsp-read-high-center-a7/result.json \
  --read-pc 0xFFF45A10 --min-address 0xFFF80000 \
  --max-address 0xFFFFFFFF --output correlation.json
```

The report schema is `stunrun-gsp-read-write-correlation/v1`; it records the
read/write overlap and nearest frame deltas without assigning semantic field
names. The neighboring-window run produces the same zero same-frame result
and the shifted `-2/+6` cadence described above.

Provenance is in
`reference/experiments/stunrun/m5-gsp-high-write-fork-1790.metadata.json`.

## Same-window queue correlation

To connect the earlier main-CPU FIFO burst with GSP activity, a same-run GSP
read/write capture was taken over power-on `late_drive` frames 1280–1300. It
recorded 40,660 high-memory writes and 8,180 indexed-reader reads without
truncation. The reads overlapped 5,644 written addresses; 673 had a
same-frame address overlap, and 7,273 had an exact address-and-value match to
an earlier write. The nearest-writer ranking is dominated by
`0xFFF41060` (5,469 reads), followed by `0xFFF414C0` (1,113) and
`0xFFF41650` (687).

The instruction trace identifies `0xFFF41060` as `MOVE *A3,A0,0` in the
sentinel dispatcher. Its surrounding control flow selects
`0xFFF9FC00`/`0xFFFCFC00`, checks for `0xFFFF`, and polls queue pointers; it
does not prove that the routine constructs the indexed records. The result
therefore establishes a common-timeline queue/VRAM boundary, not a semantic
record producer. The earlier main-CPU FIFO trace remains the upstream input
candidate, while the GSP-local queue payload and record conversion remain
UNKNOWN. Full provenance is in
`reference/experiments/stunrun/m5-gsp-fifo-source-1280-same-run.metadata.json`.

## Recorded-race phase check

The combined tracer was then run against the canonical recorded input
`/tmp/race/r1.inp`, rather than the reconstructed `late_drive` schedule. The
same nominal frame window is not the same renderer phase: frames 1280–1300
produced 5,374 reads in the high GSP range, with 4,480 reads at the FIFO-to-
work-buffer PCs `0xFFF48D50–0xFFF48E20`, and zero reads at the indexed-record
PCs `0xFFF45000`, `0xFFF45090`, `0xFFF450F0`, and `0xFFF45110`.

This explains why a playback capture cannot simply be substituted for the
earlier scripted correlation. The recorded race is still transferring data
through the FIFO consumer at this point; the indexed-record walk occurs in a
later or differently scheduled phase. The result is not a semantic payload
assignment. Its exact command, hashes, counts, and non-truncation status are
recorded in
`reference/experiments/stunrun/m5-gsp-recorded-playback-phase.metadata.json`.

The next bounded playback window (frames 1300–1500) locates the beginning of
the indexed-record phase at frame 1304. The four indexed reader PCs then
produce 68,261 reads through frame 1499 without read truncation. Its write
stream reached the configured cap, so it is deliberately not used for
producer correlation; it only supplies the exact phase boundary for a future
capture. Provenance is in
`reference/experiments/stunrun/m5-gsp-recorded-index-phase.metadata.json`.

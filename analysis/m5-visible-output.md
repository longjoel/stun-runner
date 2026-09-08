# M5 first visible output — oracle boundary

M5 is not complete yet. The original rendered checkpoint is now pinned as a
repeatable local oracle, and the native shell can emit an image artifact with
matching geometry, but the native pixels are still the deliberate blank M4
scaffold.

## Original oracle

Two fresh installed-MAME runs used the pinned title checkpoint recipe:

```sh
SDL_VIDEODRIVER=dummy mame -rompath /tmp/stunrun-roms-system \
  -noreadconfig -nowriteconfig -cfg_directory <fresh-cfg> \
  -nvram_directory <fresh-nvram> -nonvram_save stunrun -video soft \
  -sound none -skip_gameinfo -seconds_to_run 10 \
  -snapname m5-oracle -snapshot_directory <output> \
  -autoboot_script mame/lua/checkpoint.lua
```

Both produced a byte-identical `512×240` 8-bit RGB PNG:

```text
sha256: 9aa7b12be07ef28b5796c64e67e722b595664cd09b8c5daead3d7a3707b9e85e
```

The frame contains the S.T.U.N. Runner title/attract presentation, vehicle,
roadway, and HUD. This is a rendered observation, not a claim that the native
target has reconstructed those subsystems.

## Native boundary

Set `STUNRUN_RENDER_PPM=/tmp/native-frame.ppm` when running the native shell.
It writes a `512×240` P6 RGB frame and reports its deterministic pixel hash.
The current native frame is tagged `mode=blank-scaffold`; comparing it against
the oracle is expected to fail until evidence-backed title rendering is
implemented. The next investigation is to identify the smallest original
renderer/palette/command slice needed to reproduce one stable visible region.

The dependency-free `tools/compare-ppm` gate reports that comparison rather
than reducing it to a single hash. Against the two-run oracle conversion and
the current native blank frame, the observed result was:

```text
dimensions: 512x240 on both sides
changed pixels: 121610 / 122880
changed channels: 334729
maximum channel error: 255
mean channel error: 79.6585205078125
```

This is the expected negative control and is retained so the first real native
visual slice can be measured against the same oracle.

## First renderer boundary

A paired GSP trace over frames `850–930` narrows the first input-dependent
visual divergence to a pixel-blit loop. The stationary and driven traces share
the preceding `PIXBLT B,XY` and address-generation instructions; the first
aligned difference is the driven path taking `ADDXY A8,A1` at `0xFFF465A0`
where the stationary path executes another `PIXBLT B,XY` at `0xFFF46590`.
The driven trace also contains 558 occurrences of the known `F4000000` writer
set versus 405 in the stationary trace. This is the current smallest literal
renderer boundary, not yet a decoded tile, palette, or full-screen renderer.

Provenance and complete trace hashes are in
`reference/experiments/stunrun/m5-gsp-render-boundary.metadata.json`.

## GSP snapshot address units

The GSP program space reports an address shift of `3`, so a byte-oriented
snapshot must not assume that consecutive sampled addresses are consecutive
GSP values. `tools/mame-memory-snapshot` now accepts `--address-stride` and
records it in the result metadata. For the installed MAME configuration, a
16-bit GSP VRAM word capture uses stride `16`; the corresponding 8-bit view
uses stride `8`.

The GSP display registers are mapped at `0xc0000000` and are likewise
captured at stride `16`. The register indices relevant to the multisync
scanline callback are `DPYCTL=8`, `DPYSTART=9`, `DPYTAP=27`, `DPYADR=30`,
`HEBLNK=1`, and `HSBLNK=2`. The bounded GSP-state recipe now reports these
values and effective `fine_scroll` alongside the PC/status and memory checksums. The
fine-scroll control is read from GSP control register 1 and masked to the
multisync 3-bit range, matching MAME's `hdgsp_control_hi_w` behavior. This establishes the
runtime inputs needed to apply MAME's literal scanline formula to a VRAM
snapshot; it does not yet claim a decoded native renderer.

## Literal frame decoder

`tools/decode-gsp-frame` applies the multisync callback to a captured 16-bit
GSP VRAM snapshot and the currently selected 256-entry palette reads. It is
deliberately parameterized for `DPYTAP` and fine scroll because the current
low-rate state sample is a frame-boundary observation, not a per-scanline
register trace. A synthetic fixture covers the word-byte selection and row/
column formula.

The first real decode reproduces the title scene's broad structure, but is not
pixel-equivalent yet. With the frame-600 captures and explicit
`--dpy-tap 0x4c0 --fine-scroll 7`, the current comparison is:

```text
dimensions: 512x240 on both sides
changed pixels: 40806 / 122880
mean channel error: 10.939640299479167
```

This is a useful positive boundary, not an M5 completion claim. The remaining
work is to capture the display parameters at the actual scanline/update point
or reconcile MAME's screenshot crop with the frame-boundary register sample.

The decoder also has an explicitly empirical `--visible-layout` mode. Applying
the paired frame-600 VRAM and palette captures with that mode yields a
byte-identical screen:

```text
changed pixels: 0 / 122880
changed channels: 0
mean channel error: 0.0
sha256: c9b94f3f2a76ed95ffbfe6ab2a3601ffec8235fea62a691ba2fa100875a320d9
```

The recovered layout is four 512-byte visible lines per 2048-byte multisync
VRAM row (`row = y // 4`, `byte offset = (y % 4) * 512`). This is strong
evidence for the VRAM word/byte lane and palette interpretation. It is still
an analysis decoder, not the native implementation, so M5 remains open until
the native renderer consumes an evidence-backed equivalent state.

The GSP-state tool now accepts `--screen PATH` and calls MAME's
`screen:snapshot()` at the requested frame. A frame-600 paired run produced a
`512×240` PNG with the canonical oracle hash
`9aa7b12be07ef28b5796c64e67e722b595664cd09b8c5daead3d7a3707b9e85`, proving
that the memory/register capture and rendered oracle can be collected from one
deterministic run.

The native renderer now has a matching pure C primitive for this recovered
layout, covered by `native-render-boundary-c`. A fixture bridge exports the
captured words/palette through `tools/export-gsp-native-state`; when those
binary inputs are supplied to the native shell, its `gsp-visible-state` frame
is byte-identical to the paired MAME screen. The shell still has no native
game-state producer, so this proves renderer integration rather than full M5
completion. A fresh local invocation of the built shell reproduced the paired
frame with zero changed pixels and zero changed channels; the exact command,
fixture hashes, and comparison are recorded in
`reference/experiments/stunrun/m5-native-visible-bridge.metadata.json`.
The same check is now packaged as `tools/run-native-visible-bridge`, which
runs the shell, writes the actual PPM, invokes `tools/compare-ppm`, and returns
failure on either a native-run error or a pixel mismatch.

## Road-buffer consumer boundary

A fresh bounded 68010 read trace over the recorded `late_drive` schedule
captured the road-buffer consumer at PC `0x02248E`. It performed three
complete 384-word sequential drains of `0xFF9584–0xFF9882` at frames 1290,
1293, and 1297. The same PC did not read the twin window in this interval;
PC `0x0298BE` read the twin for animation while `0x0298C0` read the base.
This narrows the native producer boundary to base-buffer → FIFO transfer,
while leaving broader lifetime/scheduling semantics open. Provenance is in
`reference/experiments/stunrun/m5-road-buffer-consumer-trace.metadata.json`.
The same read tracer now accepts the recorded race input directly; replaying
`/tmp/race/r1.inp` captured repeated base-buffer bursts through frame 1293,
including a split burst across frames 1088–1089. That replay result is in
`reference/experiments/stunrun/m5-road-buffer-recorded-replay.metadata.json`.
Separate deterministic read/write taps further match the `0x02248E` FIFO
payload to every other source read: 384 source reads correspond to 192 FIFO
writes in the three-frame control window. This lane-level contract is recorded
in `reference/experiments/stunrun/m5-road-fifo-lane-differential.metadata.json`.

A same-schedule checkpoint pair now samples the main road buffer and GSP VRAM
around those bursts. The base buffer continues changing between frames 1290
and 1293 while GSP VRAM remains byte-identical, then 3,306 GSP words change by
frame 1297. This is the first bounded timing evidence for a buffered/display-
scheduled road consumer; it is not yet a decoded geometry format. Provenance is
in `reference/experiments/stunrun/m5-road-gsp-checkpoint-correlation.metadata.json`.

The same record now includes a payload check: every 192-word road burst exactly
matches the even words of its 384-word source buffer, while the values do not
appear verbatim across the downstream GSP VRAM snapshot. This strengthens the
producer boundary and indicates GSP-side interpretation, but still does not
identify curvature, width, or scanline fields.

A GSP VRAM write tap in the same interval attributes the display-side writes to
four PCs: broad writer `0xFFF454E0`, `FILL L` at `0xFFF43030`, and the periodic
`PIXBLT B,XY`/paired writers at `0xFFF46590` and `0xFFF47AB0`. Their cadence is
offset from the main road bursts, so these are shared renderer landmarks, not a
promoted road-table decoder. The writer trace is recorded in
`reference/experiments/stunrun/m5-road-gsp-checkpoint-correlation.metadata.json`.

A six-frame writer differential then compared the same GSP VRAM window under
the established `late_drive` path and the available `fork_hold_left` path.
Both paths used the same four writer PCs, but their event counts and footprints
changed: `0xFFF43030` doubled from 256 to 512 events, `0xFFF46590` grew from
800 to 992, and `0xFFF47AB0` grew from 720 to 6,352. The broad writer also
changed from 22,683 to 5,914 unique addresses. This makes the writer set a
useful input/state-dependent boundary for a future save-state-matched trace,
but the independent reset runs are not sufficient to call any writer
road-specific. Provenance is in
`reference/experiments/stunrun/m5-gsp-vram-steering-differential.metadata.json`.

The stronger save-state-matched fork uses the canonical live checkpoint at
setup frame 2400 and the exact `saved_state_left`/`saved_state_center`
relative schedules. At relative frame 1800 it reproduces the established
screen hashes (`7ebd0c…` left and `f2af2b…` center) while reporting identical
display registers (`DPYADR=0xFBDC`, `DPYSTART=0xFFFC`, `DPYTAP=0`,
`HEBLNK=0x37`, `HSBLNK=0x137`, `DPYCTL=0xF004`, fine scroll `7`) and zero
changed words in both 512-entry palette planes. The GSP VRAM snapshot changes
in 27,174 of 32,768 sampled words across 959 ranges. The paired write traces
also show state-dependent scheduling: broad writer `0xFFF454E0` changes from
25,407 to 31,853 events, while `0xFFF46590` changes from 2,064 to 1,376.
This is the first save-state-matched proof that the visible differential is
carried by GSP VRAM production rather than palette or scanout-register state;
the writer PCs are still shared renderer machinery, not a promoted road
decoder. Full hashes are in
`reference/experiments/stunrun/m5-save-state-gsp-visible-differential.metadata.json`.

The corresponding runtime GSP instruction traces decode the previously raw
writer addresses. `0xFFF45DD0` is a literal `LINE 0` primitive preceded by
endpoint arithmetic (`SRL`, `CMPXY`, `MOVX`, subtraction, and increment),
making it the strongest current candidate for a geometry-producing native
slice. `0xFFF46590` is a `PIXBLT B,XY` loop that reads a source descriptor,
masks it with `0x7F`, computes a source offset, and reads the low VRAM window
at `0x02000000`. `0xFFF454E0`, `0xFFF45A00`, and `0xFFF47AB0` are `FILL XY`
paths with computed coordinates; `0xFFF43030` is a `FILL L` setup with
`B3=0x400` and `B7=0x40200`. The left/center traces contain 16,082/10,077
executions of the line primitive and 195/194 pixel-blit instructions over the
same ten-frame endpoint window. These decoded contexts narrow the next native
work to GSP geometry/pixel primitives while preserving the semantic caveat:
the line is not yet proven to be roadway, object, or HUD output. Provenance is
in `reference/experiments/stunrun/m5-gsp-instruction-fork-trace.metadata.json`.

A PC-filtered GSP read trace then followed the two coordinate loads inside the
`FILL XY` path. `0xFFF45A10` (`MOVE *A5+,A7,1`) and `0xFFF45A40`
(`MOVE *A5+,A9,1`) read 16-bit words from dynamic high GSP memory in the
`0xFFFA…–0xFFFE…` range. After removing the instruction-fetch event at the
filtered PC, the left fork supplied 1,772 and 1,292 data reads respectively;
the center fork supplied 2,306 and 1,680. The address streams differ despite
the identical saved-state origin, which ties the visible fork to changing GSP
work-record inputs before rasterization. This narrows the producer question to
the high-memory record writer/source format; it does not yet assign those
records to roadway, object, or HUD semantics. Provenance is in
`reference/experiments/stunrun/m5-gsp-geometry-source-read-trace.metadata.json`.

The next narrow probe filtered the preceding `PIXBLT B,XY` descriptor load at
`0xFFF464E0`. In the same relative `1790–1800` window it observed 198 data
reads on the left fork and 132 on the center fork, from different addresses in
the mirrored high GSP VRAM window. The records include repeated 16-bit words,
but their values are intentionally left uninterpreted. A simultaneous read tap
at the `PIXBLT B,XY` PC (`0xFFF46590`) over the low GSP VRAM source window
returned zero events. This is an instrumentation boundary: the MAME tap does
not expose that pixel operation as an ordinary data read at the filtered PC.
The exact commands, counts, and log hashes are recorded in
`reference/experiments/stunrun/m5-pixblt-descriptor-read-trace.metadata.json`.
The reusable ROM-free decoder for this observation is
`tools/analyze-packed-text-trace`; for example:

```sh
tools/analyze-packed-text-trace <ram-read-result.json> \\
  --pc 0xFFF464E0 --output packed-text.json
```

It removes the instruction-fetch tap events, collapses the interleaved
duplicate bus observations, and preserves both the raw words and decoded byte
stream for review.
The analyzer also reports the low and high byte lanes independently, which is
important here because the GSP byte-sized operand can select one lane from a
packed 16-bit record without treating the companion byte as another glyph in
the same iteration.
For the words `7243 6465 7469 3A73`, the interleaved stream is `Credits:`;
the separate lanes are `Ceis` and `rdt:`. This is the current producer
constraint: reconstructing the live cursor requires its byte-lane state as well
as its high-memory word address.

The blit setup also loads a fixed `8 × 8` dimension pair from
`0xFFF5DB00`/`0xFFF5DB10`, followed by the split pointer words `0xDBC0` and
`0xFFF5`, giving the inferred source base `0xFFF5DBC0`. A 512-word, stride-16
snapshot at that base contains 380 nonzero words at both sampled blit frames.
This bounds an 8×8 source-tile table suitable for a future literal native
fixture. It does not yet establish whether the words are glyph rows, their
2-bit pixel ordering, or the exact coordinate/character-record relationship.

The setup values can be followed literally through the register arithmetic:
the two 16-bit reads at `0xFFF5DB00`/`0xFFF5DB10` combine as
`A7 = 0x00080008`; `MOVX` and `MOVY` therefore yield width and height 8, and
the `MPYU` computes `A11 = 64`. The next two reads at
`0xFFF5DB20`/`0xFFF5DB30` combine as `A10 = 0xFFF5DBC0`. Each source-record
byte selected by `ANDI 0x7F` is consequently mapped by the loop as
`source = 0xFFF5DBC0 + (byte & 0x7F) * 64`, before `PIXBLT B,XY` performs the
VRAM write. This is a register/data-flow contract; the address-unit and pixel
lane interpretation still require an independent glyph rendering match.

That glyph match is now established. The four captured source words for `0`
decode LSB-first to row masks `1C, 36, 63, 63, 63, 36, 1C, 00`. An independent
crop of the frame-1797 oracle at screenshot origin `(64,213)`, selecting the
foreground RGB `(255,254,0)`, produced the identical eight masks. The native
renderer now contains this bounded primitive as
`stunrun_render_gsp_glyph_8x8`, with transparent zero bits and explicit
foreground color; `native-render-boundary-c` covers its edge and mask behavior.
This proves one glyph’s source packing and raster orientation, while the full
record-to-coordinate producer remains outside the current native shell.
The same check now succeeds for source-table index `0x43` (`C`): words
`633E 0303 6303 003E` match the oracle crop at `(212,224)` with row masks
`3E,63,03,03,03,63,3E,00`. Two independent glyphs therefore validate the
generic source addressing and orientation; color selection and the broader
text-record cursor/coordinate producer remain separate work.
It also succeeds for lowercase source-table index `0x72` (`r`): words
`0000 6E3E 0606 0006` match the crop at `(220,224)` with masks
`00,00,3E,6E,06,06,06,00`. The renderer format is therefore consistent across
digits, uppercase letters, and lowercase letters.
The native table-backed helper `stunrun_render_gsp_glyph_from_table` now
implements the measured `& 0x7F` code selection and four-word tile stride.
It is a reusable renderer boundary only; the native shell does not pretend to
own the live GSP high-memory cursor until that producer is independently
reconstructed.
The captured source words for the complete `Credits:` run are also covered by
the native renderer regression at oracle placement `(212,224)`. This checks
the eight-character sequence and its 8-pixel advance as one bounded HUD
fixture; it supplies glyph codes explicitly and does not claim to reconstruct
the live GSP record cursor.

The complementary write probe does expose the `PIXBLT` destination. On the
center fork, `0xFFF46590` generated two 688-write bursts in the same window:
`0x02070610–0x020773D0` at frame 1793 and
`0x02034610–0x0203B3D0` at frame 1797. Across both bursts there were 992
unique destination addresses. This establishes that the native blit boundary
must write into the GSP VRAM pixel space, but the irregular destination ranges
and unresolved source records are not enough to infer a rectangle or texture
format yet.

Mapping each destination word through the verified visible-layout formula makes
the shape much clearer: the frame-1793 burst covers logical rows `y=112–119`,
and the frame-1797 burst covers logical rows `y=52–59`; both span x positions
`14–506`. These are VRAM-layout rows, not final screenshot rows—the display
registers remap them during scanout. The corresponding screenshots visibly
contain HUD text: frame 1797 shows `TIME 0:35.0`, `LEVEL`, and `Credits: 0`.
There are 496 unique mapped word positions per burst. The eight-row height,
combined with the `8 × 8` source dimensions and packed HUD strings in the
source records, is strong evidence that this `PIXBLT` path is drawing text or
small HUD tiles. It still does not prove the source bit order or assign each
record to a particular on-screen label.

The raw descriptor words also expose a likely text path. Interpreting each
16-bit word in little-endian byte order gives `0:35.0` at `0xFFFEA4C0` from
`3A30 3533 302E 0000`, and `Credits: 0 ` at `0xFFFEA810` from
`7243 6465 7469 3A73 3020 0020`. These are byte-level observations, not yet
semantic assignments: the `PIXBLT` loop consumes the low-byte-sized operand and
the role of the companion byte, glyph stride, and coordinates still needs to
be established. Nevertheless, the evidence shifts the most economical next
probe toward a HUD/text blit fixture rather than assuming this loop is road
geometry. The raw counts and hashes remain in
`reference/experiments/stunrun/m5-pixblt-descriptor-read-trace.metadata.json`.

The preceding GSP parser is now bounded directly. At `0xFFF45000`, the
runtime sequence is `MOVE *A3+,A5`, zero extension, `CMPXY`, `SLL 4h,A5`, and
`ADD A1,A5`: the selected record address is therefore a base plus a 16-byte
index stride. At `0xFFF45090`, the first record words are loaded into `A4`,
`A8`, and `A6`; the later coordinate path loads another `A8`, then `A7`, `A9`,
and `A10` before the `FILL XY` loop. The center fork visibly reads index words
`0x0001, 0x0011, 0x0021, …` at 16-byte address steps, while the record reader
observes repeated literal `0x0130` values in one dynamic region. These are
mechanism-level facts only: no record field is promoted to road, object, or
HUD semantics. Provenance is in
`reference/experiments/stunrun/m5-gsp-record-parser-read-trace.metadata.json`.

This mechanism is now represented by the dependency-free C slice in
`reproduction/gsp/record_parser.c`: it computes the observed `base + index *
0x10` address and exposes the raw word positions consumed by the traced
`A4/A8/A6`, later `A8`, and `A7/A9/A10` loads. The dedicated
`gsp-record-parser-slice-c` test passes. This is a native-facing transport and
record-layout contract, not yet a complete GSP producer or semantic renderer.

The recorded race also isolates a GSP-side FIFO-fed consumer at `0xFFF48C20`.
Its runtime entry occurs 15 times in the road-transfer window versus 5 times
in the matched no-input trace. The loop at `0xFFF48D50–0xFFF48E80` combines
byte lanes and writes two work-buffer families based at `0xFFF6F650` and
`0xFFF70650`. This is the next literal renderer boundary: it proves an
input-dependent FIFO-to-work-buffer path, but not that either destination or
any payload word is specifically roadway data. Details are in
`analysis/gsp-road-fifo-consumer.md` and its provenance metadata.

Destination snapshots add a timing check: from frames 1290 to 1300, the
driven path changes 235/256 sampled words at `0xFFF6F650` and 195/256 at
`0xFFF70650`, while the no-input baseline changes 1 and 0. This validates the
two-buffer update boundary as a native-facing synchronization point without
inventing payload semantics.

A common-save-state fork probe then traced all GSP writes in
`0xFFF80000–0xFFFFFFFF` for the ten-frame renderer window used by the
left/center visible differential. Both forks produced 1,878 events and the
event sequences were byte-identical. The input-dependent read streams are
therefore consuming records that were already present at the fork boundary;
the upstream record producer remains earlier in the state-building path.
This negative result prevents treating the renderer window itself as the
native record-production step. See
`reference/experiments/stunrun/m5-gsp-fork-high-write-probe.metadata.json`.

The title-era follow-up (state 600 plus 60 relative frames) also fails to
show a bulk high-memory upload: five of 32,768 stride-16 samples change, while
the full write trace is dominated by stack/register-save and renderer PCs.
Those deltas remain UNKNOWN rather than being labeled as track state. See
`reference/experiments/stunrun/m5-gsp-title-high-write-probe.metadata.json`.

The power-on high-memory trace adds an earlier producer lead, with a necessary
correction: the 6,454-event frame-405 burst is predominantly a broad
initialization/clear operation, not a proven record upload. The recurring
64-word transactions at `0xFFF9FC00`/`0xFFFCFC00`, plus the
`0xFFF41060` sentinel dispatch and `0xFFF716A0`/`0xFFF71670` pointers, are the
stronger literal candidates. The driver map places the two regions in mirrored
shared GSP VRAM, so they are VRAM-backed staging/renderer machinery rather than
proven standalone queues; they are not semantic track labels. See
`reference/experiments/stunrun/m5-gsp-queue-dispatch-probe.metadata.json`.

PC-filtered writes further bound the transfer shape: each destination receives
896 full-word writes in seven 128-write bursts across the 1280–1298 window.
The base-side addresses advance by `0x20` from `0xFFF6F650`; the twin-side
family begins at `0xFFF70660` and follows the same count. The offset difference
is recorded literally and is not yet a semantic front/back or road/object
classification.

The paired writer values are not identical copies: only 21 of 896 same-position
writes match between the base and twin traces (for example `0xBD08` versus
`0x6565`). The native boundary must therefore preserve separate output streams;
shared cadence alone does not justify duplicating one buffer into the other.

The traced GSP then copies the two streams onward: `0xFFF4AA60` writes 2,048
full-word values to `0xF5000000–0xF5000FF0`, while `0xFFF4AAD0` writes 2,048
to `0xF5800000–0xF5800FF0`, across the same eight burst frames. This closes a
work-buffer-to-display-memory transport boundary, but not the display-device
format or a native game-state producer.

The snapshot harness now supports recorded `.inp` playback as well as the
instruction-trace wrapper. This permits exact-race GSP/RAM snapshots at the
same frames used by the consumer traces, without substituting a synthetic
input schedule.

A fresh build of the integrated native shell was verified through
`tools/run-native-visible-bridge` using the captured VRAM/palette fixture. The
512×240 PPM matched the expected frame exactly: zero changed pixels, zero
changed channels, and SHA-256
`c9b94f3f2a76ed95ffbfe6ab2a3601ffec8235fea62a691ba2fa100875a320d9`. This
confirms the shell-level `stunrun_gsp_video_render` path; the fixture remains
externally supplied and is not a native game-state producer.

The canonical recorded-race snapshot series now covers both GSP work-buffer
families in one 512-word, stride-16 capture. It provides an exact replay-backed
input for the next producer/consumer correlation step.

The ordinary GSP read tap observes no readback from the first downstream copy
destination, so that address family remains a device-boundary observation
rather than a directly sampled visible framebuffer.

The exact-race downstream snapshots confirm synchronized updates. Their values
must not be paired against source snapshots from separate MAME invocations;
same-event read/write traces show all 2,048 values copied exactly across the
two passes. Destination device interpretation remains the next format target;
no pixel interpretation is promoted.

# GSP indexed-record slice

This directory contains a small, dependency-free C slice of the mechanism
observed in the M5 runtime traces. The traced TMS34010 code selects a record
with `base + (index << 4)`, then consumes raw 16-bit words at fixed positions
while preparing `FILL XY` geometry.

`record_parser.c` deliberately preserves the literal mechanism and register
destinations. It does not claim that any word is a road, object, HUD, weapon,
or other game-semantic field. The evidence is recorded in
`reference/experiments/stunrun/m5-gsp-record-parser-read-trace.metadata.json`.

The native CMake test is named `gsp-record-parser-slice-c` and requires no ROMs
or MAME.

`work_buffer_consumer.c` adds the adjacent-byte lane composition and the
masked-selector/16-byte destination-address setup observed at
`0xFFF48C20–0xFFF48D50`. Its `gsp-work-buffer-consumer-slice-c` test is also
ROM-free. The lane-stream helper writes independently supplied raw composed
words to the two observed destination families, without assigning payload or
buffer semantics.
The same slice exposes the separately observed 128-write burst address shape:
`0x20` spacing, with base origin `0xFFF6F650` and the observed twin origin
`0xFFF70660`. Its lane-stream helper keeps the A0/A1 data streams separate;
the trace shows only 21 of 896 corresponding writes matching, including
observed values such as `0xBD08` versus `0x6565`.
The write-event helper combines caller-supplied values with the observed
address sequence while refusing partial output when either stream lacks
capacity; it does not synthesize or relate the two value streams.

`stunrun_gsp_display_copy` models the later 256-word sequential copy for one
independent stream. The observed destination families are
`0xF5000000`/`0xF5800000`; address-space and pixel-format interpretation remain
outside this transport slice.

## PIXBLT text cursor slice

`text_cursor.c` models the register-level portion of the HUD/text blit traced
at GSP PC `0xFFF46590`. It consumes packed 16-bit descriptor words in
low-byte/high-byte order, stops at the first NUL byte, records the selected
lane and glyph code, advances A0 by eight address units per glyph, and advances
the low half of A1 by eight pixels per glyph. The caller supplies the y bias;
the observed `high(A1) - 0x28` relation is currently proven only for the saved
frame-1793/1797 checkpoint.

The `gsp-text-cursor-slice-c` test reproduces the captured `0:35.0` and
`Credits: 0 ` descriptor streams, including their raw descriptor addresses
and register samples. This is a transport/decoder boundary, not a generalized
gameplay HUD producer.

`tools/analyze-gsp-text-cursor` consumes the corresponding register-enabled
`stunrun-ram-read-trace-result/v1` file and performs the same bounded reduction
for humans. It filters instruction-fetch taps, folds repeated bus reads, and
emits the decoded bytes plus A0-before/A0-after and A1 cursor values:

```sh
tools/analyze-gsp-text-cursor descriptor-register-trace/result.json \\
  --pc 0xFFF464E0 --output cursor-analysis.json
```

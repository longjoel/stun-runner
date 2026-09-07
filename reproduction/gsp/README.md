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

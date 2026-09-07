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

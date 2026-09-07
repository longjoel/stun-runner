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

The provenance record is
`reference/experiments/stunrun/m5-gsp-road-fifo-consumer.metadata.json`.

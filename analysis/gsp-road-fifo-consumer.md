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

## Replay support

`tools/mame-trace` now accepts `--playback INP` in the same positional slot as
`--load-state`. Playback requires `INPUT=none`, passes the recording through
MAME's native input replay path, and suppresses scripted inputs. This makes
recorded races reusable for CPU instruction tracing.

The provenance record is
`reference/experiments/stunrun/m5-gsp-road-fifo-consumer.metadata.json`.

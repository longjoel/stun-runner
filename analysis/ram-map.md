# RAM Annotation Map

This is the RAM counterpart to the ROM landmark records. Addresses are kept
literal until a runtime trace, snapshot diff, or MAME map establishes a
stronger claim. A range being listed here does not make it a game-semantic
field.

## Hardware-defined regions

| Address/range | Owner/view | Evidence | Status |
|---|---|---|---|
| `0xFF8000–0xFFFFFF` | 68010 work RAM | pinned `driver_68k_map` | `MAME-CONFIRMED` |
| `0x0000–0x1FFF` (ADSP data space) | ADSP-2100 internal data RAM | runtime ADSP address space and direct snapshots | `MAME-CONFIRMED` |
| `0x0000–0x1FFF` (ADSP program space) | ADSP-2100 program RAM | runtime map, upload taps, program snapshots | `MAME-CONFIRMED` |
| `0x800000–0x807FFF` | 68010 view of ADSP program RAM | `hd68k_adsp_program_r/w` | `MAME-CONFIRMED` |
| `0x808000–0x80BFFF` | 68010 view of ADSP data RAM | `hd68k_adsp_data_r/w` | `MAME-CONFIRMED` |

The ADSP data/program addresses above are separate spaces; `DM($0955)` is not
the same address as a 68010 byte address. The host windows are device views,
not ordinary 68010 work RAM.

## Runtime-annotated locations

| Address/range | Literal observation | Confidence |
|---|---|---|
| `DM($0955–$0956)` | startup writes `0x1242, 0x124E`; the same pair appears in the frame-411 data-space delta | `OBSERVED-IN-TRACE + SNAPSHOT-DIFF` |
| `DM($0959–$095A)` | startup writes `0x7FFF, 0xFFFF`; the same pair appears in the frame-411 data-space delta | `OBSERVED-IN-TRACE + SNAPSHOT-DIFF` |
| `DM($0957–$0958)` | startup probe observes zero values in the reconstructed slice | `OBSERVED-IN-REPLACEMENT` |
| `0xFF9000–0xFF9003` | input-adjacent 68010 RAM record read/writes at static candidate `0x020430`; field meaning unresolved | `STATIC-CANDIDATE + OBSERVED-IN-TRACE` |
| `0xFF9004–0xFF9007` | adjacent sampled record; no late-control differential in the tested schedules | `OBSERVED-NEGATIVE` |
| `0xFFDB4A` | byte advanced by the static input candidate; late-control landmark sample | `STATIC-CANDIDATE` |
| `0xFFDAEE` | sampled update flag; no late-control differential in the tested schedules | `OBSERVED-NEGATIVE` |
| `0x80BFFE` | one observed 68010 write of `0xFFFF` during bounded title-path initialization | `OBSERVED-IN-TRACE` |

## Snapshot-diff clusters

The first full-RAM input comparison used identical clean boots and captured
the 68010 work-RAM range at frames 600, 650, 700, 750, 800, 850, and 900.
There are no differences at frames 600 or 650. The first divergence is 61
bytes at frame 700, after the Coin 1/Start events; it is 27 bytes at frame
750, then expands to 1,875 bytes at frame 800 after the steering/Button 1
event and reaches 3,163 bytes at frame 900. Large frame-900 clusters include:

```text
0xFF95E3–0xFF970A   296 bytes
0xFF98E3–0xFF9A0A   296 bytes
0xFF97A0–0xFF9850   177 bytes
0xFF9AA0–0xFF9B50   177 bytes
0xFF9718–0xFF979E   135 bytes
0xFF9A18–0xFF9A9E   135 bytes
```

These are candidate state/working regions only. The snapshot diff establishes
coincident change, not ownership or meaning. Provenance is recorded in
`reference/experiments/stunrun/main-ram-snapshot-diff.metadata.json`.

## Writer-PC attribution

A bounded write trace over `0xFF9000–0xFF9FFF` from frames 680–705 narrows the
cause of the first divergence. The first event mismatch occurs at frame 680;
the first writer present only in the late-drive run is `0x02048C` at frame 681,
writing `0xFF9006`. A concentrated follow-on path in `0x0205E2–0x02063A`
writes `0xFF94AE–0xFF94BE`, with `0x0205E2` as the repeated writer. These are
the first RAM annotations tied to input-differential writer PCs rather than
only to final snapshot contents. See
`reference/experiments/stunrun/main-ram-write-trace-differential.metadata.json`.

## Open annotation work

- correlate changed RAM clusters with writer PCs and exact input events;
- split persistent state from renderer/temporary buffers;
- add normalized field annotations only after repeated cross-run evidence;
- keep host device windows separate from ordinary 68010 RAM in selectors.

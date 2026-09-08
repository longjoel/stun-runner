# GSP indexed record walk

This note records the current literal understanding of the GSP routine around
`0xFFF44BF0–0xFFF45630`. It is based on the runtime TMS34010 instruction
trace in `reference/experiments/stunrun/m5-gsp-instruction-fork-trace.metadata.json`
and the register-enabled read traces in
`reference/experiments/stunrun/m5-gsp-record-parser-read-trace.metadata.json`.

The routine begins by loading a table base into `A5`, copying that base to
`A1`, and reading an initial value to form a secondary pointer:

```text
A12 = 0xffffffff
A11 = 0x00010001
A2  = [0xfff716c0]
A5  = incoming table base
A1  = A5
A0  = [A5]
A3  = A5 + (A0 << 4)
A13 = [0xfff77e60]
```

The loop at `0xFFF45000` then reads a 16-bit index from `A3` and advances
`A3`:

```text
index = *A3++
index = zero_extend(index)
if (index compared with A12) branch to 0xFFF44D10
record = A1 + (index << 4)
call 0xFFF45090(record)
```

Thus the selected record stride is 16 bytes. The register trace provides a
direct example: index values `0x0001`, `0x0011`, `0x0021`, … occur at
16-byte address steps in the center fork, while the selected `A5` addresses
change between the center and left forks from the same saved-state origin.

`0xFFF45090` reads the first record word into `A4`, tests control bits, then
reads two more words into `A8` and `A6`. One path derives an address from the
low byte of `A4`, reads a value through the table based at `0xFFF71D20`, and
writes that value to `0xF4000000`. The trace does not show this as a computed
code dispatch or prove what the table/device value means. The path then
prepares coordinate-like values and enters the raster loop around
`0xFFF45460`:

```text
read word -> A8;  B0 = A8;  read word -> A6
read word -> A8;  B8 = A8 << 2;  rotate -> B9
A8 <<= 16;        A6 = MOVY(A8); B1 = A8; B0 = MOVY(B1)
A6 += A11
A10 = 1
B2 = B0;          B7 = A6

repeat:
    A0 += A9;      B0 +=-with-carry B1
    A4 += A7;      A6 +=-with-carry A8
    B2 = B0;       B7 = A6
    B7 -= B2       // SUBXY
    FILL XY
    decrement-and-branch on A10
```

The trace also shows an alternate setup at `0xFFF45510` that reads three
additional words (`A8`, `A9`, `A10`) before entering the same loop. These are
literal instruction/register observations; their semantic ownership is still
`UNKNOWN`. In particular, this evidence does not prove that every record is a
road segment, nor that the text-like records found in the high GSP window are
produced by this exact indexed walk. The native implementation should retain
the indexed lookup, 16-byte stride, low-byte table lookup, and branch conditions as
mechanism until a producer-side correlation proves more.

The complete canonical center-fork replay provides the strongest current source
address census. Across relative frames 1790–1800 it captured 21,418 GSP reads
without truncation. The index-load PC `0xFFF45000` read 1,012 data events from
`0xFFFB90C0–0xFFFEA3D0` (808 unique addresses). The first three record-load
sites read 1,010 events each; at `0xFFF45090` they cover 952 unique addresses
from `0xFFFA0260–0xFFFE8B60`, with the paired `A8` and `A6` loads advancing by
two bytes. A representative frame-1791 record begins at `0xFFFD0260`, reads
`0x0130`, and has `A5=0xFFFD0260` with `A1=0xFFFD0250`.

This identifies the actual dynamic record source window used by the indexed
reader and separates it from the previously tested `0xFFF98xxx` stack traffic.
It still does not establish who populates the source records or what their
fields mean. The full replay hashes and per-PC counts are recorded in
`reference/experiments/stunrun/m5-gsp-record-parser-read-trace.metadata.json`.

A paired source-window write trace touches the same dynamic address set as the
indexed reads: 3,585 addresses overlap across 16,060 writes and 4,042 filtered
reads. However, the captures are separate MAME invocations, so the analyzer
finds zero same-frame write/read matches. The nearest write timing is centered
at three frames before the read (`-3`: 1,938 reads), but this is only a
cross-run timing comparison and cannot establish producer ownership. The result
is recorded in
`reference/experiments/stunrun/m5-gsp-record-source-write-correlation.metadata.json`;
the next producer probe must preserve a same-run or save-state-matched event
relationship.

## Human-readable evidence anchors

- Center parser trace: first sampled record reads `0x0130` at
  `0xFFFD0260`, with `A5=0xFFFD0260` and `A1=0xFFFD0250`.
- Center index trace: first sampled index read is `0x0001` at
  `0xFFFE8C70`, followed by `0x0011` at `0xFFFE8C80`.
- The same saved-state fork changes the selected address stream and read
  counts under left steering; this is state-dependent input to the renderer,
  not proof of a semantic field.
- The visible raster primitive reached by the setup is `FILL XY` at
  `0xFFF454E0`.

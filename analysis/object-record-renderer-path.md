# Object-record to renderer path

The `0xFFDD86` record is now connected to visible-output submission by both
static listing and runtime writer traces. This is a mechanism description;
the record's game-specific object identity is still unknown.

## Data path

```text
input/object update
  └─ 0x03D120 / 0x03D1A0
       └─ update fields in 16-byte record
            └─ 0xFFDD86 + index * 0x10
                 └─ 0x03D012 copies selected record to a local 16-byte buffer
                      └─ 0x03E51C consumes the record
                           └─ 0x03E3FC builds geometry primitives
                                └─ 0x02F470 writes geometry words to 0xC0000C
```

## Literal listing evidence

At `0x03D012`, the main CPU:

1. receives a record index;
2. shifts it left four bits (`asl.w #4`);
3. adds it to base `0xFFDD86`;
4. copies exactly 16 bytes to a stack-local buffer;
5. calls `0x03D254` with that buffer and a second argument.

The updater at `0x03D120` clamps three byte fields and stores them at record
offsets `+0`, `+1`, and `+2` (`0x03D18C`, `0x03D18E`, `0x03D192`). The updater
at `0x03D1A0` advances the interpolation/counter fields at offsets `+5` and
`+6`, then writes the smoothed value back to `+2` (`0x03D1B2`, `0x03D1C0`,
`0x03D1DE`, `0x03D1E2`, `0x03D1EC`, `0x03D1F4`, `0x03D1FA`). A parallel path
at `0x03D208–0x03D248` performs the same style of smoothing for offsets
`+10`, `+11`, and `+12`, eventually feeding the record's offset `+2`.

The caller at `0x03CF40` derives the record address from an index in the
mapped input/state area at `0xFFFF8000`, then calls both `0x03D120` and
`0x03D1A0`. This explains why lateral forks alter a compact lane of the
record while leaving unrelated status RAM unchanged.

At `0x03E51C`, the caller invokes `0x03D012`, then passes derived values to
`0x03E3FC`. That routine makes repeated calls to `0x02F470`. The latter emits
the primitive payload literally:

```asm
move.w  <word>, $c0000c.l       ; FIFO selector/data words
move.w  <word>, $c0000c.l
move.w  <word>, $c0000c.l
move.w  <word>, $c0000c.l
move.w  <word>, $c0000c.l
```

Thus the record is upstream of a geometry/FIFO submission path. It is not a
persistent craft-status structure.

## Runtime correlation

The matched 600-frame saved-state traces show the same path dynamically:

- the record window is cleared at relative frame 220;
- the record update loop begins at frame 303;
- center, left, and right forks differ at only five directional record bytes;
- left adds writer PCs `0x03D208` and `0x03D234–0x03D240`;
- the corresponding traces run through frame 599 without truncation.

The strongest supported semantic label is therefore
**directional object/animation geometry record**. Armor, weapon, ammunition,
score, timer, and player-health meanings remain explicitly unassigned.

## Provenance

Static evidence comes from the installed-MAME main-CPU listing
`/tmp/stunrun-listing-ff9578/maincpu-68010.lst`, generated for the pinned
MAME/ROM environment. Runtime hashes and paths are recorded in
`reference/experiments/stunrun/m5-object-record-writer-trace.metadata.json`.

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

Thus the record is statically upstream of a geometry/FIFO submission path. It
is not a persistent craft-status structure.

## Runtime qualification

A focused FIFO write tap over `0xC0000C`, filtered to the `0x02F470` entry
point, captured no events in the current saved-state interval. This is
expected to miss the routine's internal write PCs and also shows that this
particular branch was not directly exercised by the loaded state. An
unfiltered FIFO tap is heavily multiplexed by other emitters, including the
road/command paths, and reached its event budget before it could be used as a
record-specific attribution.

The evidence status is therefore precise: the **record update mechanism is
runtime-confirmed**, and its **candidate geometry/FIFO consumer is
static-listing-confirmed**, but a direct runtime record-to-FIFO payload
correlation remains open. The next proof should trace the executing
`0x03E51C`/`0x03E3FC` caller path or use a fresh state in which that display
branch is active, then pair its FIFO writes with the record bytes.

An instruction trace from the same saved state makes the boundary sharper.
Across relative frames 300–310 it observes repeated calls
`0x03CF40 → 0x03D120/0x03D1A0` and 24 executions each of the two updater
entries. It observes zero executions of `0x03E51C`, `0x03E3FC`, or
`0x02F470`. The current run therefore proves the live updater, but not that
the geometry-consumer branch is selected in this state.

A second instruction trace loaded the later `/tmp/stunrun-latedrive2400.sta`
checkpoint and covered relative frames 1–120. It again observed the updater
(`0x03D120` and `0x03D1A0`, ten entries each) and zero executions of the
candidate consumer entries. This rules out “the frame-1200 state was simply
too early” as the explanation; the consumer attribution needs a different
caller/branch or a display-focused trace.

A fresh reset-to-`late_drive` instruction trace over frames 850–1800 adds an
important distinction: `0x02F470` executes 204 times, but the observed caller
is `0x02B7E8`, which constructs its own coordinate arguments before calling
the common emitter. The same trace contains 3,848 calls to `0x03CF40`, 3,852
entries to `0x03D120`, and 3,853 to `0x03D1A0`, but zero entries to
`0x03E5C4`, `0x03E51C`, or `0x03E3FC`. Therefore `0x02F470` is a shared
geometry/FIFO primitive in this runtime path; its execution alone does not
prove that the `0xFFDD86` record supplied its arguments.

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
The bounded instruction trace is `/tmp/stunrun-m5-object-instr/maincpu.trace`
with SHA-256
`cfb7affa7e2f7fef694f5fcdb54339ad4c4d30815b5aaafaca952699fcd697b9`.
The later-checkpoint trace is
`/tmp/stunrun-m5-object-instr-2400/maincpu.trace` with SHA-256
`43ec6634bb044a315bb6786f9208738230bf8fc51211d1bfe127371c2384bb94`.
The reset-to-`late_drive` trace is
`/tmp/stunrun-m5-late-instr/maincpu-68010.trace` with SHA-256
`992037d9b7b5a71160b9740bdca8ddf43b5954d91d0f54bb4c9d7fd32188c827`.

The saved-state trace then extended the frame-1200 checkpoint through
relative frames 600–1800. It observed 2,757 calls to `0x03CF40`, 2,761
entries to `0x03D120`, and 2,767 to `0x03D1A0`, with zero entries to
`0x02B7E8`, `0x02F470`, `0x03E5C4`, `0x03E51C`, or `0x03E3FC`. This is a
bounded negative: running longer from this checkpoint does not select the
candidate consumer branch. The next search belongs at reset/start or a
specific event transition.

The long trace is
`/tmp/stunrun-m5-object-instr-state-long/maincpu-68010.trace` with SHA-256
`89b62658c80f672f82332650fb094f501ab16b708ce313b35b2d36b007435728`.

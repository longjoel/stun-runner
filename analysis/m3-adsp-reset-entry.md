# M3 ADSP reset entry and source-defined control fixture

## Provenance

- Target: `stunrun`, rev. 6
- MAME: `0.289 (mame0289-dirty)`
- Runtime inventory: `reference/inventory/stunrun.json`
- ADSP program map: `0x0000–0x1fff`, word-addressed in the Lua API
- Reset entry: `PC=0x0004` observed in the runtime inventory and the frame-136
  original-code snapshot

## Encoding proof

The checked-in `reset-loop` fixture contains four reset-vector words followed
by an unconditional `JUMP $0004` at program word `0x0004`:

```text
word 0x0000..0x0003: 0x00000000  (NOP)
word 0x0004:         0x0018004F
```

The jump encoding is derived from MAME's `adsp2100.cpp` executor and
`2100dasm.cpp`: the unconditional condition is `0xf`, the target occupies
bits `4..17`, and the jump opcode class is `0x18` (the `0x1c–0x1f` classes
are calls in the executor; the upper opcode bits overlap the target field).
Therefore:

```text
(0x18 << 16) | (0x0004 << 4) | 0xf = 0x0018004f
```

The image builder emits the 24-bit instruction in a 32-bit big-endian word
container and records entry `0x0004`; the runtime loader writes logical ADSP
word indices and verifies every word by read-back.

## Runtime acceptance

The fixture is accepted only when both conditions hold:

1. the MAME loader reports zero program-word readback mismatches; and
2. after a bounded settle interval, the ADSP PC remains at the reset-loop
   target `0x0004`.

This is an encoder/control-path fixture, not a claim about game semantics.

## First source-defined initialization prefix

The retained startup landmarks identify the original reset path as:

```text
0x0004: CALL $0780
0x0005: CALL $0834
```

The `init-prefix` fixture preserves those two call encodings, places
unconditional `RTS` stubs at both observed targets, and then loops at `0x0006`.
The calls and returns are encoded from the same MAME executor rules as the
reset loop; the stubs are deliberately source-defined and do not claim to
reproduce the original PM/DM side effects.

It installs after the repeatable original upload boundary at frame 412, reads
back every word with zero mismatches, and reaches the stable loop at `PC=0x4`
in two fresh configurations. The runtime evidence is recorded in
`reference/experiments/stunrun/m3-adsp-init-prefix.metadata.json`.

The follow-on `init-state` fixture keeps the decoded literal setup from
`0x0006` through `0x003C`, the mailbox prelude through `0x004F`, and the
bounded decoded bodies at `0x0780–0x07A1`
and `0x0834–0x0846`, then enters a source-defined bounded loop. Installed at
frame 1, before the original upload, its opt-in data-space probe reproduces the observed setup landmarks
`DM($0955)=0x1242`, `DM($0956)=0x124E`, `DM($0959)=0x7FFF`, and
`DM($095A)=0xFFFF` in two fresh MAME runs. This is a literal reconstruction
slice; the source-defined loop begins at the observed mailbox boundary
`0x0050`. A separate two-run, 60-frame bounded probe keeps the ADSP at
`PC=0x0050`, preserves those landmarks, and records `DM($001B)=0x0A00`
without assigning it semantics. Later wait/interrupt behavior and the
canonical title checkpoint remain open. Its provenance is recorded in
`reference/experiments/stunrun/m3-adsp-init-state-long.metadata.json`.

At the frame-61 pre-upload boundary, the replacement run matches the original
checkpoint's main CPU, GSP, and sound CPU fields exactly. The intentional
difference is isolated to the ADSP: the oracle remains at reset `PC=0x0004`
with empty program RAM, while the source-defined slice is executing at
`PC=0x0050` with zero loader read-back mismatches. This is a machine-boundary
comparison, not a claim of title-path equivalence.

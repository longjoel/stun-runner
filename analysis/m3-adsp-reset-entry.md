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

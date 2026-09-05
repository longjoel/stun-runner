# M1 Plan — Machine Map

M1 turns the reproducible M0 laboratory into an evidence-backed map of the
active processors, memory regions, interrupts, and inter-processor contracts.
It is an investigation milestone, not a native-port or broad decompilation
milestone.

## Entry conditions

- M0 checkpoint: commits `278ac6f` and `064c203`.
- Canonical target: `stunrun`, S.T.U.N. Runner (rev 6).
- Runtime processors: `:mainpcb:maincpu`, `:mainpcb:gsp`, `:mainpcb:adsp`, and
  `:mainpcb:jsa:cpu`.
- ROM-dependent runs use `STUNRUN_ROMPATH`; ROM contents remain outside git.
- Existing bounded launch, replay, checkpoint, listing, and trace recipes are
  the only starting machinery assumed by this plan.

## First slice: 68010 → ADSP program upload

Question: does the 68010 populate the ADSP program window during boot, and
which 68010 PCs perform the writes?

Target region: `0x800000–0x807fff`, the MAME-confirmed ADSP program window.

For every observed write, retain at least:

- emulated frame/time or debugger cycle;
- 68010 program counter;
- target address and write width;
- written value/bytes;
- the bounded experiment and ROM/MAME identities.

The experiment must stop at the existing frame-600 title boundary unless an
earlier terminal condition is proven. It must preserve the raw telemetry and
produce a summary separately under `analysis/`.

## Experiment queue

1. ADSP program upload: write telemetry during power-on → title.
2. ADSP data traffic: change-only observations for `0x808000–0x80bfff` during
   power-on, coin/start, and the first available gameplay boundary.
3. Main input polling: static XREF candidates for `0x60c000`, then a paired
   no-input/input-toggle replay.
4. Sound boundary: paired boot/coin-start runs with 68010/JSA communication
   and interrupt observations.
5. GSP boundary: identify the first runtime execution and candidate command
   submission path; do not infer graphics semantics from device names alone.

## M1 checkpoint target

Extend `stunrun-checkpoint/v1` only with fields supported by repeatable
observations. The minimum proposed normalized form is:

```text
machine identity: target, MAME build/hash, ROM manifest hash
time identity: frame and emulated seconds
processor state: exact tags plus selected PC/status registers
region observations: named address range, access direction, count, hash or
                    bounded change summary
inputs: ordered frame-relative actions
provenance: experiment, scripts, trace flags, and artifact paths
```

Raw dumps remain optional and local; metadata and hashes are the canonical
comparison surface. Exact versus normalized fields must be stated per field.

## Acceptance criteria

M1's first slice is complete when:

- the ADSP upload experiment is executable from a checked-in command/spec;
- the result is repeatable across two independent runs;
- writer PCs and target ranges are recorded, or an explicit no-observation
  result is recorded with a bounded explanation;
- `analysis/interconnect.md` distinguishes MAME-confirmed facts,
  observed-in-trace facts, and unresolved hypotheses;
- IRQ-0002 and IRQ-0003 are either resolved with evidence or preserved as
  narrowed follow-up questions;
- no semantic selector or implementation contract is changed merely to make a
  test pass.

## Stop conditions

Stop and record an UNKNOWN when the debugger cannot expose a reliable memory
watchpoint, when traces disagree across identical runs, when the runtime
inventory changes, or when a proposed meaning requires inference beyond the
captured evidence.

# Current Status

## Current milestone

**M3 — Reset and initialization**

## State

M0: **complete** — reproducible machine, harness, evidence, coverage, and emission baseline committed

M1: **complete** — evidence-backed machine/interconnect contract and bounded
input-dependent renderer boundary committed

M2: **complete** — fixed-origin ADSP replacement image is built, placed, and
executed in the pinned MAME runtime with repeatable read-back and PC advance

Original in MAME: **ROM-validated and runtime-inventoried; title checkpoint established**

Step 0 driver mining: **worked example retained; runtime tags reconciled against system MAME 0.289**

Reproduction target: **not started**

Native target: **smoke coverage baseline only; implementation not started**

Verification harness: **bounded replay/checkpoint/trace path implemented; M1 machine facts are promoted into selectors and evidence fixtures**

## Immediate objective — M3 reset and initialization

Use the M2 image loader and the frozen M1 contract to move the replacement slice
to the earliest reproducible initialization boundary:

1. Identify the earliest bounded point at which the replacement ADSP image can be installed without relying on the original upload.
2. Preserve the verified ADSP reset/control sequence and establish a stable replacement execution loop.
3. Compare the resulting machine boundary against the canonical title-path checkpoint.
4. Preserve the M1 selectors, provenance, and unresolved questions; do not reinterpret the oracle to fit the replacement.

The first useful output is not decompiled C. It is a repeatable laboratory plus a box of provenance-labeled puzzle pieces.

## Working processor inventory

Current Step 0 mining indicates the active programmable processors are:

- Motorola 68010 main CPU
- TMS34010 GSP
- ADSP-2100 geometry/math DSP
- 6502-class JSA II sound CPU

Other important modeled devices include:

- TMS34012 pixel processor / expander
- YM2151
- OKI6295

The previously assumed second TMS34010 MSP is currently **excluded** because the mined target config calls `multisync_nomsp(config)`. Runtime inventory from the pinned build must confirm this.

## Step 0 puzzle pieces already identified

From the worked driver-mining example:

```text
0x60c000              main digital input landmark
0x800000–0x807fff     68010-visible ADSP program window
0x808000–0x80bfff     68010-visible ADSP data window
0xb80000 family       documented ADC/input landmarks
```

These are hardware-interface labels, not yet gameplay semantics.

## M1 handoff

### Investigator

Use the committed M0 artifacts as inputs, then establish:

- exact target shortname/revision;
- exact MAME build/version/commit;
- ROM manifest;
- runtime CPU/device inventory;
- static listings;
- XREFs to driver-mined address landmarks;
- bounded boot/title experiment;
- `analysis/interconnect.md` observations;
- first experiments from `analysis/driver-mining/stunrun.md`.

### Implementer

Avoid broad game implementation until M1 identifies a verified slice and its machine contract.

Prove minimal replacement-code generation independently for the active targets:

- 68010
- TMS34010 GSP
- ADSP-2100
- 6502

Do not build an MSP toolchain unless pinned runtime inventory demonstrates an MSP is active.

### Verifier

Extend the M0 harness with targeted memory/interrupt telemetry and the normalized checkpoint schema from IRQ-0003.

The first oracle pipeline should support deterministic launch/input, bounded waits, checkpoint capture, deliberate failure artifacts, bounded tracing, and original-code LCOV generation.

## Current blockers

- M0 is complete; the installed package's unavailable upstream source commit remains documented in `analysis/m0-audit.md`.
- Installed MAME reports `0.289 (mame0289-dirty)` and exposes no source commit; its binary hash is recorded in `mame/system-baseline.json`.
- Static-listing generation recipe works for all four active programmable processors; generated listings remain local and are hash-recorded in `reference/listings/stunrun.metadata.json`.
- The native smoke coverage pipeline now emits standard LCOV through GCC/gcov; npm `lcov` is available, while `genhtml` is unavailable, so HTML rendering is not part of this baseline.
- Deterministic replay schema, bounded frame selector, and machine-readable failure artifacts exist; semantic title/gameplay selectors remain future work.
- M1 has established the active ADSP program writer (`0x02D35C`), its
  RAM-resident 17-record/2,728-word upload stream, the ADSP-buffer→GSP FIFO
  block contract, a frame-363 title-path FIFO submission landmark, source-mapped
  control/reset/bank handlers, and the title-path JSA command-byte pairing.
  Remaining questions are upload-stream semantics, post-gameplay traffic, the
  first input-dependent GSP submission, and a synchronized semantic gameplay
  selector. A bounded AD Stick X/Button 1 replay now produces a repeatable
  input-dependent rendered delta at frame 1800, a normalized checkpoint, a
  late GSP differential, and a first main-CPU input-path divergence. A later
  default-DIP coin/start schedule now reaches a repeatable roadway/control
  boundary with a second normalized checkpoint, but semantic selector meaning
  is still unproven. The late-applied SW1-off experiment is a repeatable loading / blank
  negative control; applying the same DIP bank before reset now yields a
  repeatable exact-frame rendered-scene checkpoint, but the identical no-input
  render means it is not yet a gameplay selector. A bounded post-start GSP trace
  now identifies two additional drive-only writers to `@F4000000` (`0xFFF45330`
  and `0xFFF479E0`), narrowing the renderer path while preserving the semantic
  selector caveat.

## Completed baseline slices

- Canonical working set: `stunrun`, S.T.U.N. Runner (rev 6), 25 ROM files; MAME `-verifyroms` passes.
- MAME-derived manifest: `roms/stunrun.manifest.json`.
- Repeatable runtime inventory: `tools/mame-inventory`.
- Runtime inventory evidence: `reference/inventory/stunrun.json`.
- Machine-map reconciliation: `reference/inventory/stunrun.reconciliation.json` (all four active processors present; no unexpected programmable device).
- Reproducible MAME launchers use `-noreadconfig -nowriteconfig`, an isolated
  config directory, and `-nonvram_save`; pinned local NVRAM hashes are recorded
  in `mame/system-baseline.json`, so persisted user DIP/input state cannot
  contaminate evidence.
- Deterministic replay: `tools/mame-replay` runs `experiments/stunrun/boot_to_title.json` and `coin_start.json`; repeated boot runs produced identical screenshot hashes.
- Bounded debugger traces: `tools/mame-trace` and `mame/lua/trace.lua`; six-frame trace metadata is under `reference/traces/stunrun/`.
- Trace-to-LCOV prototype: `tools/trace-to-lcov`; first standard `.info` report is hash-recorded under `reference/coverage/original/`.
- Minimal code-emission proof: `tools/emit-proof`; all four active processors decode emitted NOP fixtures through MAME.
- M2 first image slice: `tools/build-replacement-image` emits a fixed-origin ADSP
  NOP image, and `tools/mame-replacement-image` installs it at frame 140 after
  the original upload, releases the MAME-modeled ADSP reset/`/BR`/`/HALT`
  controls, and observes a repeatable PC advance to `0x0E2C` at frame 142.
  Evidence: `reference/experiments/stunrun/m2-adsp-replacement-image.metadata.json`.

## Last verified checkpoint

`title-attract`, captured at frame 600 / 9.96648 emulated seconds. Numeric state is recorded in `reference/checkpoints/title/state.json`; the screenshot remains local and is identified by hash in `reference/checkpoints/title/metadata.json`.

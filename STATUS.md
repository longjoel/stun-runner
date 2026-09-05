# Current Status

## Current milestone

**M0 — Reproducible machine, harness, evidence, and toolchain baseline**

## State

Original in MAME: **not yet baselined in this repository**

Step 0 driver mining: **worked example created; must be refreshed against the final pinned MAME revision**

Reproduction target: **not started**

Native target: **not started**

Verification harness: **not started**

## Immediate objective

Freeze the specimen and laboratory before interpreting the game broadly:

1. choose one MAME-supported S.T.U.N. Runner set;
2. pin one MAME version/commit;
3. refresh `analysis/driver-mining/stunrun.*` against that pinned source revision;
4. generate a ROM manifest from pinned MAME containing filenames, regions, sizes, CRC32, SHA-1, and load/interleave semantics;
5. validate the local ROM set without committing ROM contents;
6. enumerate debugger-visible CPU/device tags through the harness and reconcile them with the mined machine map;
7. generate baseline static listings for each **active** programmable processor;
8. apply driver-mined hardware landmarks and search static XREFs;
9. establish a deterministic bounded boot-to-title harness experiment;
10. define the first canonical checkpoint/failure bundle;
11. prove bounded debugger tracing and trace-to-LCOV;
12. prove/select code-emission tooling for each active programmable CPU/DSP requiring reconstruction.

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

## First handoff

### Investigator

Refresh Step 0 against pinned MAME, then establish:

- exact target shortname/revision;
- exact MAME build/version/commit;
- ROM manifest;
- runtime CPU/device inventory;
- static listings;
- XREFs to driver-mined address landmarks;
- bounded boot/title experiment;
- initial `analysis/interconnect.md` observations;
- first experiments from `analysis/driver-mining/stunrun.md`.

### Implementer

Avoid broad game implementation until M0 baseline exists.

Prove minimal replacement-code generation independently for the active targets:

- 68010
- TMS34010 GSP
- ADSP-2100
- 6502

Do not build an MSP toolchain unless pinned runtime inventory demonstrates an MSP is active.

### Verifier

Implement the harness/checkpoint path described in `MAME_HARNESS.md` and reconcile runtime inventory against the Step 0 machine map.

The first oracle pipeline should support deterministic launch/input, bounded waits, checkpoint capture, deliberate failure artifacts, bounded tracing, and original-code LCOV generation.

## Current blockers

- No canonical ROM set/revision selected yet.
- No canonical MAME version recorded yet.
- Step 0 example has not yet been refreshed against the final pinned revision.
- No MAME-derived ROM manifest checked in yet.
- No runtime device-tag inventory recorded yet.
- No static-listing generation recipe checked in yet.
- No deterministic harness trace/checkpoint recipe checked in yet.
- No TMS34010 GSP or ADSP-2100 code-emission path validated yet.

## Last verified checkpoint

None yet.

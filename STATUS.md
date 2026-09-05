# Current Status

## Current milestone

**M0 — Establish a reproducible machine and evidence baseline**

## State

Original in MAME: **not yet baselined in this repository**

Reproduction target: **not started**

Native target: **not started**

Verification harness: **not started**

## Immediate objective

Freeze the specimen and the laboratory before interpreting the game:

1. choose one MAME-supported S.T.U.N. Runner set;
2. pin one MAME version/commit;
3. generate a ROM manifest from MAME containing filenames, regions, sizes, CRC32, and SHA-1 values;
4. validate the local ROM set with MAME without committing ROM contents;
5. enumerate debugger-visible CPU/device tags;
6. generate baseline static listings for each programmable processor;
7. establish a deterministic bounded boot-to-title trace recipe;
8. define the first canonical checkpoint bundle;
9. prove or select code-emission tooling for each programmable processor.

The first useful output is not decompiled C. It is a repeatable experiment plus a verified toolchain boundary.

## Working processor inventory

Subject to confirmation against the pinned MAME build:

- Motorola 68010 main CPU
- TMS34010 GSP
- TMS34010 MSP
- TMS34012 pixel processor / expander
- ADSP-2100 geometry/math DSP
- 6502-class JSA II sound CPU
- YM2151 and OKI6295 sound devices

See `REQUIREMENTS.md` for the evidence status and toolchain acceptance rules.

## First handoff

### Agent 1 — Investigator

Establish:

- exact MAME shortname and ROM revision;
- exact MAME build/version/commit;
- launch command;
- MAME-derived ROM manifest;
- CPU/device inventory and debugger-visible tags;
- main CPU reset/vector entry path;
- baseline static listings;
- bounded boot trace;
- initial memory-map notes;
- first `analysis/interconnect.md` observations;
- reset and title/attract checkpoint evidence.

Record uncertainties explicitly and preserve raw evidence separately from annotations.

### Agent 2 — Implementer

Until the baseline exists, avoid broad game implementation.

Prove minimal replacement-code generation independently for:

- 68010
- 6502
- TMS34010
- ADSP-2100

For each, demonstrate known source/instructions -> expected bytes -> correct placement -> successful validation under MAME where practical.

Prepare only minimal native/reproduction scaffolding that does not encode unverified game semantics.

### Agent 3 — Verifier

Define the canonical checkpoint bundle and comparison format described in `EVIDENCE.md`.

Determine which state can be captured repeatably at reset and title/attract mode, and how deterministic inputs will eventually be replayed.

## Current blockers

- No canonical ROM set/revision selected yet.
- No MAME-derived ROM manifest checked in yet.
- No canonical MAME version recorded yet.
- No debugger-visible device-tag inventory recorded yet.
- No static-listing generation recipe checked in yet.
- No deterministic trace recipe checked in yet.
- No TMS34010 or ADSP-2100 code-emission toolchain validated yet.

## Last verified checkpoint

None yet.

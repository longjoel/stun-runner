# Project: S.T.U.N. Runner

## Goal

Incrementally reverse engineer Atari Games' **S.T.U.N. Runner** and produce both:

- a reproduction build that can execute in the original/emulated hardware environment; and
- a native Linux port based on the same verified understanding.

MAME is the behavioral reference implementation and laboratory environment.

## Project contracts

The main contracts are:

- `AGENTS.md` — mandatory entry point and current agent instructions
- `PROCESS.md` — three-agent reverse-engineering method and ownership boundaries
- `DRIVER_MINING.md` — mandatory Step 0 MAME driver-mining process
- `REQUIREMENTS.md` — hardware, ROM, pinned-MAME, and toolchain requirements
- `EVIDENCE.md` — raw evidence/listing/trace/checkpoint rules
- `MAME_INSTRUMENTATION.md` — stock MAME Lua/debugger instrumentation
- `MAME_HARNESS.md` — Playwright-style oracle automation harness
- `TESTING.md` — unit/integration/differential/E2E/regression strategy
- `LCOV.md` — native and original-code coverage reporting
- `PREFLIGHT.md` — readiness gate and cost-control rules
- `STATUS.md`, `MILESTONES.md`, `QUESTIONS.md` — current state and handoffs

## Step 0 — mine the driver before mining the ROM

For S.T.U.N. Runner and future targets, begin with MAME driver mining before broad disassembly or tracing.

The worked example is:

```text
analysis/driver-mining/stunrun.md
analysis/driver-mining/stunrun.machine-map.yaml
```

The purpose is to start with provenance-labeled puzzle pieces: actual instantiated processors, ROM layout, address landmarks, dynamic handlers, input addresses, interrupt/device wiring, and concrete experiments.

MAME source describes the emulator model. It does not automatically prove game-level semantics.

## Why this project

This project is intentionally simpler than the ongoing Virtual On effort. It is the proving ground for the reusable three-agent + harness + evidence workflow before applying it to harder hardware.

## Working hardware model

Current Step 0 mining against MAME source revision `a263f49884249c87b18d8b61f007dc24004e450b` indicates:

- Atari Multisync A046901 family main board
- Motorola 68010 main CPU, nominally 8 MHz
- TMS34010 GSP, nominally ~48 MHz
- TMS34012 pixel processor / expander
- Atari ADSP II A047046 board with ADSP-2100, nominally 8 MHz
- Atari JSA II sound board with 6502-class CPU, YM2151, and OKI6295

Importantly, the target-specific MAME config calls `multisync_nomsp(config)`, so the optional second TMS34010 MSP is **not currently part of the active S.T.U.N. Runner processor inventory**.

This remains provisional until the project pins MAME and the runtime harness inventory confirms the exact device configuration.

## Primary ROM set

Choose one MAME-supported S.T.U.N. Runner set and keep it fixed for canonical evidence.

Do not commit copyrighted ROM contents.

Generate and commit an authoritative manifest from pinned MAME containing expected filenames, regions, sizes, CRC32, SHA-1, offsets, interleave/load semantics, and MAME provenance. Local ROMs must validate before canonical evidence is generated.

## Evidence model

The project distinguishes:

1. **Raw reference evidence** — listings, traces, memory dumps, screenshots, maps, checkpoint metadata generated from the original under pinned MAME.
2. **Analysis** — annotations, symbols, hypotheses, semantic models, driver-mined puzzle pieces, experiment conclusions.
3. **Implementations** — reproduction and native code tested against the reference.

Raw evidence is never edited to match interpretation.

## Testing philosophy

Treat the project like conventional software engineering with an executable oracle.

```text
MAME driver mining
      ↓
known hardware puzzle pieces
      ↓
reproducible oracle experiment
      ↓
evidence-backed fixture
      ↓
semantic understanding
      ↓
reproduction implementation
      ↓
native implementation
      ↓
independent verification
```

Protect progress with:

- unit tests;
- subsystem integration tests;
- deterministic harness-driven E2E replay;
- golden/checkpoint fixtures;
- permanent regression tests;
- public CI without ROMs;
- optional local/private ROM-dependent oracle verification;
- LCOV reports for both native code and original execution coverage.

## Initial research questions

1. Does pinned-MAME runtime inventory confirm the Step 0 processor/device model?
2. What are the exact debugger-visible tags for the 68010, GSP, ADSP, JSA sound CPU, PSP, and important memory/device spaces?
3. Which 68010 routines write the ADSP program window at `0x800000–0x807fff` during boot?
4. What command/status structures live in the ADSP data window at `0x808000–0x80bfff`?
5. Which code references the main input landmark at `0x60c000` and documented ADC regions?
6. Which processor owns boot, high-level game state, graphics submission, geometry/math work, and sound command generation?
7. Which address ranges are ROM, RAM, shared RAM, device windows, and communication mailboxes?
8. What is the minimum deterministic trace required to reach title/attract?
9. What state can the Verifier checkpoint cheaply and deterministically?
10. What assembler/encoder strategy can emit replacement code for the **active** programmable processors: 68010, TMS34010 GSP, ADSP-2100, and 6502?
11. How do the 68010, GSP, ADSP, and JSA sound CPU communicate?
12. Which original routines can first be reduced into fast deterministic unit fixtures?
13. What is the first normalized checkpoint schema shared by original, reproduction, and native targets?
14. How should original execution coverage be computed and visualized per processor?

## Repository policy

- Do not commit original commercial ROM images.
- Do not commit giant raw traces merely because they exist; prefer reproducible scripts plus curated evidence.
- Preserve exact commands, MAME version/commit, ROM manifest, Step 0 machine-map revision, trace flags, and deterministic input definition for canonical experiments.
- Keep raw evidence immutable relative to semantic annotations.
- Keep semantic annotations distinct from implementation guesses.
- Treat each active processor as a separate program initially and document cross-processor behavior in `analysis/interconnect.md`.
- Prefer tiny verified slices over broad translation passes.
- Preserve meaningful regression tests permanently.
- Never change an oracle-derived expected value solely because reconstructed code disagrees.

## Expected working areas

```text
analysis/driver-mining/   Step 0 source-mined machine maps and worked evidence
analysis/                 semantic knowledge, per-CPU analysis, interconnect notes
asm/                      annotated disassembly/reconstruction sources
reference/                raw/regenerable listings, traces, maps, snapshots, metadata
mame/                     harness, Lua instrumentation, debugger scripts, experiments
roms/                     hash manifests only; no commercial ROM contents
reproduction/             original-hardware/emulator-targeted reconstruction
native/                   Linux-native implementation
tools/                    trace, compare, manifest, fixture, coverage, build helpers
tests/                    unit, integration, differential, golden, and E2E tests
symbols/                  machine-readable labels and confidence metadata
```

## Success criterion

The project succeeds when increasingly large slices can be:

1. located using known machine puzzle pieces;
2. observed in the original;
3. reproducibly exercised;
4. semantically explained;
5. reconstructed in the original environment;
6. expressed natively on Linux;
7. independently checked against the original;
8. protected by repeatable regression tests.

The reusable product is the whole method: Step 0 driver mining + MAME harness + evidence-backed three-agent reconstruction.

# Project: S.T.U.N. Runner

## Goal

Incrementally reverse engineer Atari Games' **S.T.U.N. Runner** and produce both:

- a reproduction build that can execute in the original/emulated hardware environment; and
- a native Linux port based on the same verified understanding.

MAME is the behavioral reference implementation and laboratory environment.

## Project contracts

The project is governed by five complementary documents:

- `PROCESS.md` — three-agent reverse-engineering method and ownership boundaries
- `REQUIREMENTS.md` — hardware inventory, ROM manifest policy, pinned-MAME requirements, and compiler/assembler acceptance rules
- `EVIDENCE.md` — static listing, trace, snapshot, deterministic-input, and canonical-checkpoint rules
- `TESTING.md` — unit, integration, differential, end-to-end, regression, CI, fixture, and coverage strategy
- `AGENTS.md` — mandatory entry point and current instructions for every agent session

Agents must read these before broad implementation work.

## Why this project

This project is intentionally simpler than the ongoing Virtual On reverse-engineering effort. The purpose is to exercise and refine the three-agent workflow on a system where the hardware is unusual but much more mature in MAME and better bounded than Sega Model 2.

## Working hardware model

The initial MAME-derived machine model is:

- Atari Multisync A046901 main board
- Motorola 68010 main CPU, nominally 8 MHz
- TMS34010 GSP, nominally 48 MHz
- second TMS34010-class MSP, nominally ~50 MHz
- TMS34012 pixel processor / expander
- Atari ADSP II A047046 board with ADSP-2100, nominally 8 MHz
- Atari JSA II sound board with 6502-class CPU, YM2151, and OKI6295

This remains a working machine model until the exact device configuration is frozen against the project's pinned MAME build. See `REQUIREMENTS.md` for evidence labels and toolchain implications.

The MAME source tree is authoritative for what the pinned emulator models, but not automatically authoritative for what the game program *means*. Driver names and device abstractions are evidence, not substitutes for program analysis.

## Primary ROM set

Start with one explicitly chosen MAME-supported S.T.U.N. Runner set and keep that set fixed for reproducible traces until there is a reason to compare revisions.

Do not commit copyrighted ROM contents.

Instead, derive and commit an authoritative manifest from MAME containing the expected filenames, regions, sizes, CRC32 values, SHA-1 values, and MAME provenance. Local ROMs must validate against that manifest/MAME definition before canonical evidence is generated.

## Evidence model

The project distinguishes three classes of artifact:

1. **Raw reference evidence** — listings, traces, memory dumps, screenshots, maps, and metadata generated from the original under pinned MAME.
2. **Analysis** — annotations, symbols, hypotheses, semantic models, and experiment conclusions.
3. **Implementations** — reproduction and native code tested against the reference evidence.

Raw evidence is never edited to match an interpretation. See `EVIDENCE.md`.

## Testing philosophy

Treat this like a conventional software project with an unusual oracle: the original game under pinned MAME.

The desired chain is:

```text
original behavior
      ↓
reproducible experiment
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

Progress should be protected by:

- unit tests for reconstructed routines and tools;
- integration tests for CPU/DSP/device boundaries;
- deterministic end-to-end replay;
- golden fixtures derived from original behavior;
- permanent regression tests for meaningful mismatches;
- public CI that does not require commercial ROMs;
- optional ROM-dependent oracle verification where valid ROMs are available locally.

Coverage is multidimensional. Track native source coverage, original execution coverage, tested-original execution coverage, semantic coverage, and behavioral milestone coverage separately. Do not collapse them into one misleading percentage.

## Initial research questions

1. What exact MAME machine/board configuration is selected for the chosen S.T.U.N. Runner set?
2. What are the exact debugger-visible tags for every programmable CPU/DSP and relevant memory space?
3. Which processor owns boot, high-level game state, rendering submission, geometry/math work, and sound command generation?
4. Which address ranges are ROM, RAM, shared RAM, device windows, and communication mailboxes?
5. What is the minimum deterministic trace required to reach the title/attract loop?
6. What static listings are needed to establish the executable surface of each processor?
7. Can the main 68010 program be rebuilt independently before DSP/graphics behavior is fully understood?
8. Which behaviors can initially be treated as hardware services behind stable interfaces?
9. What state can Agent 3 snapshot cheaply and deterministically at reset, title, game start, and first gameplay frame?
10. What assembler/encoder strategy will reproducibly emit replacement code for the TMS34010 and ADSP-2100?
11. How do the 68010, GSP/MSP, ADSP, and JSA sound CPU communicate, and which shared-memory/mailbox boundaries can become explicit subsystem contracts?
12. Which original routines can first be reduced into fast deterministic unit-test fixtures?
13. What is the first normalized checkpoint schema shared by original, reproduction, and native targets?
14. How should original execution coverage be computed and visualized per processor?

## Repository policy

- Do not commit original commercial ROM images.
- Do not commit giant raw traces merely because they exist; prefer reproducible scripts plus curated canonical evidence.
- Preserve exact commands, MAME version/commit, ROM-manifest identity, trace flags, and deterministic input definition for canonical experiments.
- Keep raw evidence immutable relative to semantic annotations.
- Keep semantic annotations distinct from implementation guesses.
- Treat each processor as a separate program initially and document cross-processor behavior in `analysis/interconnect.md`.
- Prefer tiny verified slices over broad translation passes.
- Preserve meaningful regression tests permanently.
- Never change an oracle-derived expected value solely because reconstructed code disagrees with it.

## Expected working areas

```text
analysis/       semantic knowledge, per-CPU analysis, interconnect notes
asm/            annotated disassembly and reconstruction sources
reference/      raw/regenerable listings, traces, maps, snapshots, metadata
mame/scripts/   reproducible debugger scripts
roms/           hash manifests only; no commercial ROM contents
reproduction/   original-hardware/emulator-targeted reconstruction
native/         Linux-native implementation
tools/          trace, compare, manifest, extraction, fixture, coverage, and build helpers
tests/          unit, integration, differential, golden, and E2E tests
symbols/        machine-readable labels and confidence metadata
```

## Success criterion

The project succeeds when increasingly large slices of S.T.U.N. Runner can be:

1. observed in the original;
2. reproducibly exercised;
3. semantically explained;
4. reconstructed in the original environment;
5. expressed natively on Linux;
6. independently checked against the original;
7. protected by repeatable regression tests.

The most meaningful long-term metric is:

> How much of the original game's behavior is understood, reproducibly exercised, reconstructed, and independently verified?

The project should also leave behind a reusable method: a new arcade target should be bootstrappable by changing the machine/ROM/toolchain contract while retaining the same evidence, testing, and three-agent workflow.

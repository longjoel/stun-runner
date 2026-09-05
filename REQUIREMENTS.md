# Project Requirements

This document defines the minimum hardware, ROM, toolchain, and reproducibility requirements for the S.T.U.N. Runner reverse-engineering project.

## Hardware baseline

The working hardware model is derived from MAME's current Atari `harddriv` family implementation and must be verified against the exact MAME version pinned by the project.

### Main board: Atari Multisync A046901

- Motorola 68010 main CPU, nominally 8 MHz
- TMS34010 graphics processor (GSP), nominally 48 MHz
- second TMS34010-class processor (MSP), nominally ~50 MHz
- TMS34012 pixel processor / expander (PSP), nominally ~50 MHz
- board RAM/VRAM and shared-memory windows as defined by MAME

### Math / geometry board: Atari ADSP II A047046

- Analog Devices ADSP-2100, nominally 8 MHz
- serial ROM/RAM resources modeled by MAME
- expected responsibilities include polygon transforms, lighting, and slope calculations

### Sound board: Atari JSA II

- 6502-class sound CPU, nominally ~1.7 MHz
- Yamaha YM2151
- OKI MSM6295 / OKI6295 ADPCM

## Evidence labels

Every hardware fact recorded in project documentation should be marked or clearly attributable as one of:

- `MAME-CONFIRMED` — present in the pinned MAME implementation
- `BOARD-DOCUMENTED` — supported by schematics, board notes, manuals, or trustworthy hardware documentation
- `OBSERVED-IN-TRACE` — directly observed during execution
- `ASSUMED` — plausible working assumption not yet verified

MAME device names and comments are evidence about the emulator model, not proof of game-program semantics.

## ROM requirements

Original commercial ROM contents are local user inputs and must never be committed to this repository.

The project must instead maintain an authoritative manifest derived from the pinned MAME version.

Required manifest fields:

- MAME shortname / set name
- parent set, if any
- MAME version and preferably commit SHA
- ROM filename
- MAME region/device tag
- expected length
- CRC32
- SHA-1
- load offset and load semantics where useful

The preferred workflow is:

```sh
mame stunrun -listxml > build/mame/stunrun.xml
```

A project tool should parse that XML and emit a checked-in machine-readable manifest, e.g.:

```text
roms/stunrun.manifest.json
```

Local setup must provide a ROM verification command using MAME, such as:

```sh
mame stunrun -verifyroms
```

A project wrapper such as `tools/check-roms` should eventually provide one stable command for agents and humans.

## Pinned emulator requirement

Canonical evidence is only comparable when generated with the same emulator definition.

Before M0 is complete, record:

- MAME release version
- MAME git commit when available
- exact executable path or package provenance
- launch command
- debugger/device tags discovered from the running machine

Changing MAME versions invalidates canonical evidence unless the evidence is explicitly regenerated or version-scoped.

## Native toolchain

The native Linux port should initially target a conventional, reproducible toolchain:

- GCC or Clang
- CMake or Meson
- SDL3 or another explicitly selected cross-platform runtime layer

The project should not prematurely standardize the rendering architecture beyond what is necessary for faithful reconstruction.

## Reproduction toolchains

The project must prove binary emission for every programmable processor before large-scale reconstruction.

### Motorola 68010

Candidate toolchain:

- GNU binutils / m68k-elf assembler and linker, or another reproducible assembler with known 68010 support

Acceptance test:

1. assemble a tiny known instruction sequence
2. compare emitted bytes to expected bytes / MAME disassembly
3. place code at a fixed address
4. execute the resulting image under MAME

### 6502

Candidate toolchain:

- ca65 / ld65 or equivalent

Use the same acceptance test as above.

### TMS34010

A suitable assembler must be identified and tested. Do not assume a compiler is required.

If no maintainable open tool exists, the project may implement a focused assembler/encoder sufficient for reconstruction. Minimum useful capability:

- labels
- origin / placement
- data directives
- instruction encoding
- includes
- raw binary output

### ADSP-2100

As with TMS34010, identify and test a usable assembler first. Prefer a small reproducible encoder over dependency on inaccessible or abandoned proprietary tooling.

## Toolchain acceptance rule

No processor is considered "supported" by the project merely because a disassembler exists.

For each CPU/DSP, M0 requires proof that the project can:

- decode known original code
- encode known replacement instructions
- reproduce expected bytes
- place those bytes correctly
- execute them in MAME or otherwise validate them against the modeled hardware

## Initial CPU separation

Treat each processor as a separate program during early analysis:

```text
analysis/maincpu-68010/
analysis/gsp-tms34010/
analysis/msp-tms34010/
analysis/adsp2100/
analysis/sound-6502/
```

Cross-processor behavior belongs in:

```text
analysis/interconnect.md
```

That document should prioritize:

- shared RAM
- mailboxes / command queues
- interrupt relationships
- CPU-to-DSP communication
- graphics command submission
- sound command submission
- ownership of major game-state transitions

Understanding processor boundaries is an early project goal, not cleanup work for later.

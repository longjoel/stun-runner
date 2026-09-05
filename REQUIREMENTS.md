# Project Requirements

This document defines the minimum hardware, ROM, toolchain, and reproducibility requirements for the S.T.U.N. Runner reverse-engineering project.

## Step 0 dependency

Before freezing the hardware/toolchain baseline, follow `DRIVER_MINING.md` and refresh:

```text
analysis/driver-mining/stunrun.md
analysis/driver-mining/stunrun.machine-map.yaml
```

Generic board-family capabilities are not enough. Toolchain requirements apply only to processors actually instantiated by the target's pinned MAME configuration/runtime inventory.

## Hardware baseline

The working hardware model is derived from MAME's Atari `harddriv` family implementation and must be verified against the exact MAME version pinned by the project.

The current Step 0 source-mining example found that S.T.U.N. Runner calls `multisync_nomsp(config)`. Therefore the optional second TMS34010 MSP is **not currently considered an active processor for this title**.

### Active programmable processors — provisional until pinned-MAME runtime inventory

- Motorola 68010 main CPU, nominally 8 MHz
- TMS34010 graphics processor (GSP), nominally ~48 MHz
- Analog Devices ADSP-2100, nominally 8 MHz
- 6502-class JSA II sound CPU, nominally ~1.7 MHz

### Other important devices

- TMS34012 pixel processor / expander (PSP)
- Yamaha YM2151
- OKI MSM6295 / OKI6295 ADPCM
- board RAM/VRAM and device/shared-memory windows as modeled by MAME

### Optional family hardware explicitly excluded unless re-proven

- second TMS34010-class MSP: current mined target config uses `multisync_nomsp(config)`

If the pinned MAME runtime inventory contradicts this, update the Step 0 machine map first, then update toolchain and coverage requirements.

## Evidence labels

Every hardware fact recorded in project documentation should be marked or clearly attributable as one of:

- `MAME-CONFIRMED` — explicit in the pinned MAME implementation/configuration
- `MAME-NAMED-HYPOTHESIS` — MAME naming suggests meaning but runtime game semantics are unverified
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

Preferred workflow:

```sh
mame stunrun -listxml > build/mame/stunrun.xml
```

A project tool should parse that XML and emit:

```text
roms/stunrun.manifest.json
```

Local setup must validate ROMs through MAME or a stable wrapper such as `tools/check-roms`.

## Pinned emulator requirement

Canonical evidence is only comparable when generated with the same emulator definition.

Before M0 is complete, record:

- MAME release version
- MAME git commit when available
- exact executable/package provenance
- launch command
- debugger/device tags discovered from the running machine
- Step 0 machine-map revision

Changing MAME versions invalidates canonical evidence unless evidence is regenerated or explicitly version-scoped.

## Native toolchain

The native Linux port should initially use a conventional reproducible toolchain.

Recommended direction until frozen by the project:

- C++20 or later
- GCC or Clang
- CMake + Ninja
- CTest-compatible unit/integration runner
- LCOV/genhtml coverage
- SDL3 or another explicitly selected platform layer

Keep game semantics independent from SDL/platform plumbing where practical.

## Reproduction toolchains

The project must prove binary emission for every **active programmable processor that requires reconstructed code** before large-scale replacement work.

### Motorola 68010

Candidate toolchain:

- GNU binutils / m68k-elf assembler and linker, or equivalent reproducible 68010 assembler

### 6502

Candidate toolchain:

- ca65 / ld65 or equivalent

### TMS34010 GSP

A suitable assembler/encoder must be identified and tested. If no maintainable tool exists, a focused project-owned encoder is acceptable.

Minimum useful capability:

- labels
- fixed origin/placement
- data directives
- instruction encoding
- includes
- raw binary output

### ADSP-2100

Identify and test a usable assembler/encoder. Prefer a small reproducible project-owned encoder over inaccessible proprietary tooling if necessary.

## Toolchain acceptance rule

No processor is considered supported merely because a disassembler exists.

For each active CPU/DSP requiring code emission, M0 requires proof that the project can:

1. decode known original code;
2. encode known replacement instructions;
3. reproduce expected bytes;
4. place those bytes correctly;
5. execute them in MAME or otherwise independently validate them.

Do not require an MSP assembler for S.T.U.N. Runner unless the pinned target inventory demonstrates an MSP is active.

## Initial CPU separation

Treat each active processor as a separate program during early analysis:

```text
analysis/maincpu-68010/
analysis/gsp-tms34010/
analysis/adsp2100/
analysis/sound-6502/
```

Cross-processor behavior belongs in:

```text
analysis/interconnect.md
```

Prioritize:

- shared RAM
- mailboxes / command queues
- interrupt relationships
- 68010 ↔ ADSP communication
- 68010 ↔ GSP graphics submission
- 68010 ↔ JSA sound submission
- ownership of major game-state transitions

Understanding processor boundaries is an early project goal, not cleanup work for later.

# Evidence, Listings, and Trace Contract

This document defines how raw evidence is generated, stored, annotated, and compared.

## Core rule

Raw evidence and agent interpretation must remain separate.

Agents may annotate or summarize evidence, but they must not overwrite the original listing, trace, dump, screenshot, or metadata used to support a conclusion.

Preferred flow:

```text
reference/raw evidence
        ↓
analysis/annotations
        ↓
implementation
        ↓
verification
```

## Evidence directory model

Recommended layout:

```text
reference/
  listings/
    maincpu/
    gsp/
    msp/
    adsp/
    sound/
  traces/
    boot/
    attract/
    gameplay/
  snapshots/
  maps/
  metadata/

analysis/
  maincpu-68010/
  gsp-tms34010/
  msp-tms34010/
  adsp2100/
  sound-6502/
  interconnect.md
```

Large reproducible traces may be excluded from git. The scripts and metadata required to regenerate them are project artifacts and should be committed.

## Static listings

Static listings answer:

> What code or data can exist at this address?

Use the pinned MAME debugger/disassembler where practical so instruction decoding agrees with the behavioral reference.

Generate a baseline static listing for every programmable processor that exposes executable program memory.

The raw listing belongs under `reference/listings/`.

Annotated copies belong under `analysis/` or `asm/`.

Do not edit the raw listing in place.

Listings should preserve, where available:

- address
- opcode bytes / words
- decoded instruction
- source region or device tag
- MAME version metadata

## Dynamic traces

Dynamic traces answer:

> What actually executed during this experiment?

Canonical traces must be reproducible and should use MAME's debugger trace facility with loop condensation disabled (`noloop`) unless a specific experiment documents another choice.

Loop-condensed traces are acceptable for exploratory human reading but are not canonical verification evidence because they discard exact execution counts.

## Two trace classes

### Discovery traces

Purpose: investigation.

Characteristics:

- short
- selective
- often one processor
- may focus on one address range or subsystem
- may begin/end at breakpoints
- generated frequently

Discovery traces do not need long-term retention if their relevant observations are captured in an experiment record.

### Canonical traces

Purpose: reproducibility and verification.

Characteristics:

- fixed MAME version
- fixed ROM manifest
- named start condition
- deterministic input sequence
- exact debugger script
- `noloop` where instruction-level comparison matters
- metadata recorded alongside output
- regenerated deliberately when the reference environment changes

Canonical traces should exist at milestone checkpoints, not continuously for the entire game.

## Canonical checkpoint bundle

A named checkpoint should eventually be represented as a bundle similar to:

```text
reference/checkpoints/title/
  metadata.json
  maincpu.trace
  gsp.trace
  msp.trace
  adsp.trace
  sound.trace
  maincpu.lst
  gsp.lst
  msp.lst
  adsp.lst
  sound.lst
  memory-map.txt
  mainram.bin
  vram.bin
  adsp-ram.bin
  registers.json
  screen.png
```

Not every file is required for every checkpoint. Metadata must state what was captured and why.

## Required metadata

Every canonical experiment/checkpoint must identify enough information to reproduce it.

Example:

```json
{
  "game": "stunrun",
  "checkpoint": "boot_to_title",
  "mame_version": "PINNED_VERSION",
  "mame_commit": "PINNED_COMMIT",
  "rom_manifest_sha256": "...",
  "start_condition": "power_on",
  "input_script": "inputs/boot-to-title.json",
  "debugger_script": "mame/scripts/boot-to-title.cmd",
  "trace_flags": ["noloop"]
}
```

## Debugger scripts

Manual debugger interaction is useful for discovery but insufficient for canonical evidence.

Store reproducible debugger command files under:

```text
mame/scripts/
```

Examples:

```text
mame/scripts/boot.cmd
mame/scripts/title.cmd
mame/scripts/game-start.cmd
```

Scripts should select explicit device/CPU tags discovered from the pinned MAME machine rather than relying on undocumented defaults.

## Device tags

Do not guess trace CPU/device names from documentation.

M0 requires enumerating the debugger-visible devices for the selected S.T.U.N. Runner set and recording the exact tags used for:

- 68010 main CPU
- GSP
- MSP
- ADSP-2100
- JSA sound CPU
- relevant memory spaces/devices

Those tags become part of the reproducible trace configuration.

## Memory maps and snapshots

At baseline, capture the MAME debugger's view of relevant memory maps.

For milestone checkpoints, prefer binary dumps for exact comparison and textual dumps when they materially aid human investigation.

Candidate state includes:

- main RAM
- shared RAM
- VRAM
- DSP RAM
- device/mailbox regions
- CPU registers
- important interrupt/status registers

Agent 3 should prefer hashes plus targeted decoded fields over checking giant dumps by eye.

## Deterministic inputs

Canonical comparisons require named, replayable inputs.

The project should eventually express experiments as a sequence of frame/time-relative actions, e.g.:

```text
power on
wait N frames
coin on
coin off
wait N frames
start on
start off
wait N frames
steer left
...
```

The implementation mechanism may be MAME Lua, debugger scripting, MAME input playback, or another stable method. The requirement is reproducibility, not a particular API.

## Initial canonical checkpoints

At minimum, establish:

1. power-on/reset
2. first stable title/attract state
3. game-start transition
4. first controllable gameplay frame

Do not create exhaustive full-game canonical traces before the trace pipeline has proven useful.

## Agent responsibilities

### Investigator

- creates and maintains trace/listing recipes
- preserves raw evidence
- annotates copies, never originals
- records evidence links/filenames for semantic claims
- creates explicit experiments for unknown behavior

### Implementer

- consumes semantic analysis and raw evidence when necessary
- may request additional traces but should not redefine reference evidence silently
- must not patch canonical evidence to make an implementation appear correct

### Verifier

- owns canonical replay/comparison procedures
- checks original vs reproduction vs native behavior
- records exact environment metadata
- classifies divergence before asking for a fix

## Evidence quality rule

A semantic claim is stronger when it can be traced to one or more of:

- a static listing
- a dynamic trace
- a memory/register snapshot
- a controlled experiment
- board/hardware documentation
- the pinned MAME implementation

"The code looks like it probably does X" is a hypothesis, not verified knowledge.

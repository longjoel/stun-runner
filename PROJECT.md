# Project: S.T.U.N. Runner

## Goal

Incrementally reverse engineer Atari Games' **S.T.U.N. Runner** and produce both:

- a reproduction build that can execute in the original/emulated hardware environment; and
- a native Linux port based on the same verified understanding.

MAME is the behavioral reference implementation.

## Why this project

This project is intentionally simpler than the ongoing Virtual On reverse-engineering effort. The purpose is to exercise and refine the three-agent workflow on a system where the hardware is unusual but much more mature in MAME and better bounded than Sega Model 2.

## Initial hardware model

Treat the following as the initial working model and verify details against current MAME source and observed execution before relying on them semantically.

- Platform family: Atari Hard Drivin' / Multisync-derived hardware
- Main CPU: Motorola 68010
- Geometry / math coprocessor: ADSP-2100 family device on the ADSP board
- Graphics subsystem: TMS34010-family devices used by the Atari driving hardware
- Sound: Atari JSA II family, with 6502-class sound CPU plus YM2151 and OKI ADPCM hardware
- Emulator reference: current MAME `harddriv` driver family; S.T.U.N. Runner board/device definitions are present upstream
- Native target: Linux

The MAME source tree is authoritative for what the emulator models, but not automatically authoritative for what the game program *means*. Driver names and device abstractions are evidence, not substitutes for program analysis.

## Primary ROM set

Start with one explicitly chosen MAME-supported S.T.U.N. Runner set and keep that set fixed for reproducible traces until there is a reason to compare revisions.

Record the exact shortname, ROM hashes, MAME version/commit, and launch command in `analysis/overview.md` as soon as the Investigator establishes the baseline.

Do not commit copyrighted ROM contents to this repository.

## Initial research questions

1. What exact MAME machine/board configuration is selected for the chosen S.T.U.N. Runner set?
2. Which processor owns boot, high-level game state, rendering submission, geometry/math work, and sound command generation?
3. Which address ranges are ROM, RAM, shared RAM, device windows, and communication mailboxes?
4. What is the minimum deterministic trace required to reach the title/attract loop?
5. Can the main 68010 program be rebuilt independently before the DSP/graphics behavior is fully understood?
6. Which behaviors can initially be treated as hardware services behind stable interfaces?
7. What state can Agent 3 snapshot cheaply and deterministically at reset, title, game start, and first gameplay frame?

## Repository policy

- Do not commit original commercial ROM images.
- Do not commit giant raw traces unless they are intentionally curated and useful; prefer scripts that regenerate them plus small representative evidence.
- Preserve exact commands and emulator versions for every canonical experiment.
- Keep semantic annotations distinct from implementation guesses.
- Prefer tiny verified slices over broad translation passes.

## Expected working areas

These are expected to emerge as needed rather than all being created immediately:

```text
analysis/       semantic knowledge, maps, subsystem notes
asm/            disassembly and annotations
evidence/       traces, snapshots, experiments
reproduction/   original-hardware/emulator-targeted reconstruction
native/         Linux-native implementation
tools/          trace, compare, extraction, and build helpers
tests/          differential and behavioral tests
symbols/        machine-readable labels and confidence metadata
```

## Success criterion

The project succeeds when increasingly large slices of S.T.U.N. Runner can be observed, explained, reproduced in the original environment, expressed natively on Linux, and automatically checked against the original behavior.
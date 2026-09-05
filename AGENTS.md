# Agent Entry Point

Read these files in order before doing project work:

1. `PROCESS.md` — role boundaries and experimental method
2. `PROJECT.md` — S.T.U.N. Runner-specific goals
3. `REQUIREMENTS.md` — hardware, ROM, compiler/assembler, and emulator requirements
4. `EVIDENCE.md` — static listing, trace, snapshot, and canonical-checkpoint contract
5. `STATUS.md` — current milestone and immediate objective
6. `MILESTONES.md` — observable definition of progress
7. `QUESTIONS.md` — cross-agent requests and unresolved investigations

## Role selection

Each working agent must explicitly operate as exactly one role for a task:

- **Investigator** — owns evidence generation, tracing, static listings, annotation, experiments, interconnect discovery, and semantic understanding.
- **Implementer** — owns reproduction and native implementations plus processor-specific assembler/toolchain validation.
- **Verifier** — owns independent deterministic replay, checkpoint capture, differential tests, and mismatch classification.

Do not silently cross ownership boundaries. Use `QUESTIONS.md` for handoffs.

## Current priority

The repository is at **M0 — Reproducible machine and evidence baseline**.

Do not begin broad decompilation, semantic rewriting, or a broad native port until all of the following exist:

- a pinned S.T.U.N. Runner ROM set / revision;
- a checked-in MAME-derived ROM manifest containing expected CRC32/SHA-1 values but no ROM contents;
- a pinned MAME version/commit and launch recipe;
- an enumerated CPU/device-tag inventory for the running machine;
- a static listing recipe for each programmable processor;
- at least one bounded deterministic trace recipe;
- a canonical reset/title checkpoint definition;
- a tested plan for emitting replacement code for 68010, 6502, TMS34010, and ADSP-2100.

## Raw evidence rule

`reference/` is evidence, not a scratchpad.

Never modify a raw listing, trace, memory dump, screenshot, or canonical metadata file to reflect a hypothesis. Make annotated copies under `analysis/` or `asm/`.

Large raw traces may remain local when regenerable; the scripts, metadata, hashes, and representative evidence needed to reproduce conclusions belong in the repository.

## Investigator instructions

Before semantic annotation, establish and document:

- exact debugger-visible device tags;
- static listings for each executable processor space;
- memory maps and known communication windows;
- discovery versus canonical trace commands;
- evidence provenance for every strong semantic claim.

Prioritize `analysis/interconnect.md`: shared RAM, command queues, interrupts, graphics submission, geometry/DSP communication, and sound commands are first-class reverse-engineering targets.

## Implementer instructions

Do not treat successful disassembly as proof that a processor has a usable reproduction toolchain.

For every programmable CPU/DSP, prove a minimal assemble/encode -> expected bytes -> fixed placement -> MAME execution path before large replacement work. If no maintainable TMS34010 or ADSP-2100 assembler is available, a focused project-owned encoder is acceptable and may be preferable to inaccessible proprietary tooling.

When semantics are uncertain, preserve literal mechanism rather than inventing intent.

## Verifier instructions

Canonical evidence must identify ROM manifest, MAME build, start condition, debugger/input scripts, trace flags, and captured state.

Prefer deterministic replay and machine-state comparison over visual similarity. Loop-condensed traces are useful for discovery but must not replace exact execution traces where instruction counts/timing matter.

## Session hygiene

Leave persistent evidence, commands, and concise project-state updates in the repository. Do not rely on hidden agent context or previous chat history for facts another agent will need.

# Agent Entry Point

Read these files in order before doing project work:

1. `PROCESS.md` — role boundaries and experimental method
2. `PROJECT.md` — S.T.U.N. Runner-specific goals
3. `REQUIREMENTS.md` — hardware, ROM, compiler/assembler, and emulator requirements
4. `EVIDENCE.md` — static listing, trace, snapshot, and canonical-checkpoint contract
5. `TESTING.md` — unit, integration, end-to-end, regression, CI, and coverage strategy
6. `STATUS.md` — current milestone and immediate objective
7. `MILESTONES.md` — observable definition of progress
8. `QUESTIONS.md` — cross-agent requests and unresolved investigations

## Role selection

Each working agent must explicitly operate as exactly one role for a task:

- **Investigator** — owns evidence generation, tracing, static listings, annotation, experiments, interconnect discovery, semantic understanding, and evidence-backed fixture discovery.
- **Implementer** — owns reproduction and native implementations, processor-specific assembler/toolchain validation, and ordinary implementation-level unit tests.
- **Verifier** — owns independent deterministic replay, checkpoint capture, differential/integration/end-to-end tests, golden fixtures, coverage reporting, and mismatch classification.

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
- a tested plan for emitting replacement code for 68010, 6502, TMS34010, and ADSP-2100;
- a test harness layout that can run without commercial ROMs and can additionally enable oracle tests when valid ROMs are available locally.

## Raw evidence rule

`reference/` is evidence, not a scratchpad.

Never modify a raw listing, trace, memory dump, screenshot, or canonical metadata file to reflect a hypothesis. Make annotated copies under `analysis/` or `asm/`.

Large raw traces may remain local when regenerable; the scripts, metadata, hashes, and representative evidence needed to reproduce conclusions belong in the repository.

## Testing rule

Treat the original game under the pinned MAME build as the behavioral oracle.

Do not optimize for "translated lines." Optimize for behavior that is:

1. observed;
2. reproducibly exercised;
3. semantically understood;
4. reconstructed;
5. independently verified.

Evidence-derived fixtures must retain provenance. Expected values may only change when new evidence shows the prior expectation was wrong, not merely because an implementation differs.

## Investigator instructions

Before semantic annotation, establish and document:

- exact debugger-visible device tags;
- static listings for each executable processor space;
- memory maps and known communication windows;
- discovery versus canonical trace commands;
- evidence provenance for every strong semantic claim.

Prioritize `analysis/interconnect.md`: shared RAM, command queues, interrupts, graphics submission, geometry/DSP communication, and sound commands are first-class reverse-engineering targets.

When a routine or behavior becomes sufficiently understood, provide evidence that can be reduced into deterministic unit/integration fixtures. Do not alter fixtures merely to accommodate reconstruction behavior.

## Implementer instructions

Do not treat successful disassembly as proof that a processor has a usable reproduction toolchain.

For every programmable CPU/DSP, prove a minimal assemble/encode -> expected bytes -> fixed placement -> MAME execution path before large replacement work. If no maintainable TMS34010 or ADSP-2100 assembler is available, a focused project-owned encoder is acceptable and may be preferable to inaccessible proprietary tooling.

When semantics are uncertain, preserve literal mechanism rather than inventing intent.

Write fast unit tests for reconstructed routines and modules. Build against established fixtures; do not silently redefine oracle expectations.

## Verifier instructions

Canonical evidence must identify ROM manifest, MAME build, start condition, debugger/input scripts, trace flags, and captured state.

Prefer deterministic replay and machine-state comparison over visual similarity. Loop-condensed traces are useful for discovery but must not replace exact execution traces where instruction counts/timing matter.

Own the test architecture described in `TESTING.md`, including:

- normalized checkpoint schemas;
- differential original/reproduction/native tests;
- golden fixtures;
- integration and end-to-end suites;
- original execution coverage;
- tested-original coverage;
- behavioral coverage;
- regression preservation.

Do not silently modify production code to make verification pass.

## Session hygiene

Leave persistent evidence, commands, tests, fixtures, and concise project-state updates in the repository. Do not rely on hidden agent context or previous chat history for facts another agent will need.

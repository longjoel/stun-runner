# Agent Entry Point

Read these files in order before doing project work:

1. `PROCESS.md` — role boundaries and experimental method
2. `PROJECT.md` — S.T.U.N. Runner-specific goals
3. `REQUIREMENTS.md` — hardware, ROM, compiler/assembler, and emulator requirements
4. `EVIDENCE.md` — static listing, trace, snapshot, and canonical-checkpoint contract
5. `TESTING.md` — unit, integration, end-to-end, regression, CI, and coverage strategy
6. `LCOV.md` — native and original-code coverage reporting
7. `MAME_INSTRUMENTATION.md` — stock MAME Lua/debugger instrumentation contract
8. `MAME_HARNESS.md` — Playwright-style automation harness and test-driver contract
9. `PREFLIGHT.md` — readiness gate, cost controls, stop conditions, and concurrency rules
10. `STATUS.md` — current milestone and immediate objective
11. `MILESTONES.md` — observable definition of progress
12. `QUESTIONS.md` — cross-agent requests and unresolved investigations

## Role selection

Each working agent must explicitly operate as exactly one role for a task:

- **Investigator** — owns evidence generation, tracing, static listings, annotation, experiments, interconnect discovery, semantic understanding, selectors, telemetry definitions, and evidence-backed fixture discovery.
- **Implementer** — owns reproduction and native implementations, processor-specific assembler/toolchain validation, and ordinary implementation-level unit tests.
- **Verifier** — owns the MAME harness, deterministic replay, checkpoint capture, semantic assertions, fixtures, failure artifacts, differential/integration/end-to-end tests, golden fixtures, coverage reporting, and mismatch classification.

Do not silently cross ownership boundaries. Use `QUESTIONS.md` for handoffs.

## Current priority

The repository is at **M0 — Reproducible machine, harness, and evidence baseline**.

Do not begin broad decompilation, semantic rewriting, or a broad native port until all of the following exist:

- a pinned S.T.U.N. Runner ROM set / revision;
- a checked-in MAME-derived ROM manifest containing expected CRC32/SHA-1 values but no ROM contents;
- a pinned MAME version/commit and launch recipe;
- an enumerated CPU/device-tag inventory for the running machine;
- a static listing recipe for each programmable processor;
- at least one bounded deterministic trace recipe;
- a canonical reset/title checkpoint definition;
- a tested plan for emitting replacement code for 68010, 6502, TMS34010, and ADSP-2100;
- a test harness layout that can run without commercial ROMs and can additionally enable oracle tests when valid ROMs are available locally;
- a stable Playwright-style MAME harness entry point capable of deterministic input, bounded waits, assertions, fixture/checkpoint creation, and failure artifacts;
- a first trace-to-LCOV report from original code.

## Raw evidence rule

`reference/` is evidence, not a scratchpad.

Never modify a raw listing, trace, memory dump, screenshot, or canonical metadata file to reflect a hypothesis. Make annotated copies under `analysis/` or `asm/`.

Large raw traces may remain local when regenerable; the scripts, metadata, hashes, and representative evidence needed to reproduce conclusions belong in the repository.

## Testing and harness rule

Treat the original game under the pinned MAME build as the behavioral oracle, and interact with that oracle through the stable harness described in `MAME_HARNESS.md` whenever possible.

Do not make ad hoc MAME interaction the normal workflow when the same action can be represented as a reusable harness capability, selector, fixture, experiment, or assertion.

Do not optimize for "translated lines." Optimize for behavior that is:

1. observed;
2. reproducibly exercised;
3. semantically understood;
4. reconstructed;
5. independently verified.

Evidence-derived fixtures and selectors must retain provenance. Expected values may only change when new evidence shows the prior expectation was wrong, not merely because an implementation differs.

All waits must be bounded. A timeout must produce useful diagnostic artifacts rather than hang indefinitely.

## Investigator instructions

Before semantic annotation, establish and document:

- exact debugger-visible device tags;
- static listings for each executable processor space;
- memory maps and known communication windows;
- discovery versus canonical trace commands;
- evidence provenance for every strong semantic claim.

Prioritize `analysis/interconnect.md`: shared RAM, command queues, interrupts, graphics submission, geometry/DSP communication, and sound commands are first-class reverse-engineering targets.

When a routine or behavior becomes sufficiently understood, provide evidence that can be reduced into deterministic unit/integration fixtures or semantic selectors. Do not alter fixtures or selectors merely to accommodate reconstruction behavior.

If a useful observation procedure will recur, prefer contributing a reusable harness/configuration capability over a one-off manual debugger ritual.

## Implementer instructions

Do not treat successful disassembly as proof that a processor has a usable reproduction toolchain.

For every programmable CPU/DSP, prove a minimal assemble/encode -> expected bytes -> fixed placement -> MAME execution path before large replacement work. If no maintainable TMS34010 or ADSP-2100 assembler is available, a focused project-owned encoder is acceptable and may be preferable to inaccessible proprietary tooling.

When semantics are uncertain, preserve literal mechanism rather than inventing intent.

Write fast unit tests for reconstructed routines and modules. Build against established fixtures; do not silently redefine oracle expectations.

Consume the same replay/checkpoint schema used by the MAME harness rather than inventing target-specific E2E inputs.

## Verifier instructions

Canonical evidence must identify ROM manifest, MAME build, start condition, harness/experiment identity, debugger/input scripts, trace flags, and captured state.

Prefer deterministic replay and machine-state comparison over visual similarity. Loop-condensed traces are useful for discovery but must not replace exact execution traces where instruction counts/timing matter.

Own the harness architecture described in `MAME_HARNESS.md`, including:

- stable game-agnostic automation API;
- semantic selector execution;
- bounded waits and timeouts;
- deterministic input replay;
- fixtures/save-state setup;
- normalized checkpoint schemas;
- automatic trace/screenshot/telemetry artifacts on failure;
- differential original/reproduction/native tests;
- golden fixtures;
- integration and end-to-end suites;
- original execution coverage;
- tested-original coverage;
- behavioral coverage;
- regression preservation.

Keep stock MAME/Lua details behind the harness adapter where practical so test specs survive MAME API changes.

Do not silently modify production code to make verification pass.

## Stop conditions

Stop and create a handoff/investigation request rather than guessing when:

- required evidence is missing or contradictory;
- ROM/MAME provenance mismatches;
- replay is not deterministic;
- a selector cannot be justified from evidence;
- a wait cannot be bounded reliably;
- an assembler/encoder cannot be independently validated;
- implementation conflicts with a trusted oracle fixture;
- changing an expected result would require convenience rather than new evidence.

An explicit `UNKNOWN` is a valid result.

## Session hygiene

Leave persistent evidence, commands, tests, selectors, fixtures, harness capabilities, and concise project-state updates in the repository. Do not rely on hidden agent context or previous chat history for facts another agent will need.

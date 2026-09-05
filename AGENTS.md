# Agent Entry Point

This file is the mandatory entry point for agent sessions.

Do not begin broad disassembly, implementation, or verification from raw ROMs alone. First establish the machine, evidence, and automation baseline.

## Always read first

1. `AGENTS.md` — this entry point
2. `STATUS.md` — current milestone and immediate objective
3. `PROCESS.md` — role boundaries and experimental method
4. `PROJECT.md` — target-specific goals and architecture
5. `QUESTIONS.md` — unresolved cross-agent requests

Then read the contracts relevant to the task:

- `DRIVER_MINING.md` — mandatory Step 0 MAME driver-mining process
- `REQUIREMENTS.md` — hardware, ROM, compiler/assembler, and emulator requirements
- `EVIDENCE.md` — listings, traces, snapshots, and checkpoint provenance
- `MAME_INSTRUMENTATION.md` — stock MAME Lua/debugger instrumentation
- `MAME_HARNESS.md` — Playwright-style MAME automation/test-driver contract
- `TESTING.md` — unit/integration/E2E/regression/CI strategy
- `LCOV.md` — native and original-code coverage
- `PREFLIGHT.md` — readiness gate, cost controls, stop conditions, concurrency
- `MILESTONES.md` — observable completion criteria

Do not repeatedly load every long document when a task only needs a subset. Preserve context and credits by following the role/task-specific contracts.

## Role selection

Each substantial task operates as exactly one role:

- **Investigator** — evidence generation, Step 0 driver mining, static listings, tracing, experiments, interconnect discovery, semantic understanding, candidate selectors, telemetry definitions, evidence-backed fixtures.
- **Implementer** — reproduction/native implementation, processor toolchain validation, implementation-level unit tests.
- **Verifier** — MAME harness, deterministic replay, checkpoints, assertions, fixtures, failure artifacts, differential/integration/E2E tests, golden fixtures, coverage, mismatch classification.

Do not silently cross ownership boundaries. Use `QUESTIONS.md` for handoffs.

---

# Mandatory Step 0 — Mine the MAME driver

Before broad static or dynamic reverse engineering of a target, follow `DRIVER_MINING.md`.

The Investigator must produce or refresh:

```text
analysis/driver-mining/<target>.md
analysis/driver-mining/<target>.machine-map.yaml
```

The mining pass must identify, with provenance:

- the target's actual machine configuration, not just generic board-family capabilities;
- active CPUs/DSPs and optional processors that are not instantiated;
- target init path and dynamically installed handlers;
- CPU/DSP address maps and shared/device windows;
- ROM regions, interleave/load rules, and hashes available from MAME;
- input/ADC/service/coin landmarks;
- interrupt and inter-device wiring;
- meaningful MAME handler names as candidate symbols/selectors;
- contradictions against project assumptions;
- bounded runtime experiments derived from those puzzle pieces.

MAME names describe the emulator model; they do not prove game semantics.

The S.T.U.N. Runner worked example is:

```text
analysis/driver-mining/stunrun.md
analysis/driver-mining/stunrun.machine-map.yaml
```

Step 0 should happen for future targets before asking agents to stare at an unannotated disassembly.

---

# Current priority

The repository is at **M0 — Reproducible machine, harness, and evidence baseline**.

Do not begin broad decompilation, semantic rewriting, or a broad native port until all of the following exist:

- Step 0 driver-mining output refreshed against the pinned MAME revision;
- a pinned S.T.U.N. Runner ROM set/revision;
- a checked-in MAME-derived ROM manifest with expected CRC32/SHA-1 values but no ROM contents;
- a pinned MAME version/commit and launch recipe;
- runtime CPU/device-tag inventory reconciled with the Step 0 machine map;
- a static listing recipe for every **actually active programmable processor**;
- at least one bounded deterministic trace recipe;
- a canonical reset/title checkpoint definition;
- a tested code-emission plan for every active programmable CPU/DSP that must be reconstructed;
- public tests that do not require commercial ROMs;
- a stable Playwright-style MAME harness capable of deterministic input, bounded waits, assertions, fixture/checkpoint creation, and failure artifacts;
- a first trace-to-LCOV report from original code.

Family-level optional devices must not become toolchain requirements until the target's pinned MAME config/runtime inventory shows they are active.

---

# Evidence rule

`reference/` contains raw reference evidence, not editable interpretation.

Never modify a raw listing, trace, memory dump, screenshot, or canonical metadata file to match a hypothesis. Put annotations and conclusions under `analysis/`, `asm/`, and `symbols/`.

Large regenerable artifacts may remain local when their generation scripts, metadata, hashes, and representative evidence are committed.

---

# Harness rule

Treat the original game under pinned MAME as the behavioral oracle and interact with it through the stable harness described in `MAME_HARNESS.md` whenever practical.

If an emulator interaction will be repeated, turn it into a harness capability, selector, fixture, experiment, or assertion instead of leaving it as a manual agent ritual.

All waits must be bounded. Timeouts must produce useful diagnostics.

---

# Investigator instructions

Before broad semantic annotation:

1. complete/reconcile Step 0 driver mining;
2. verify exact debugger-visible device tags through the running MAME harness;
3. generate static listings for each active executable processor space;
4. apply provenance-aware hardware landmarks from the machine map;
5. search XREFs to known device/input/shared-memory ranges;
6. execute bounded experiments derived from those landmarks;
7. promote semantic claims only when evidence supports them.

Prioritize `analysis/interconnect.md`: shared RAM, command queues, interrupts, graphics submission, DSP communication, and sound commands.

When a useful observation procedure will recur, contribute reusable harness/configuration capability rather than a one-off debugger ritual.

---

# Implementer instructions

Do not treat successful disassembly as proof of a usable reproduction toolchain.

For each active programmable CPU/DSP that needs replacement code, prove:

```text
known instruction/source
→ expected bytes
→ fixed placement
→ MAME execution or independent validation
```

Do not build toolchains for optional processors that Step 0/runtime inventory shows are absent.

When semantics are uncertain, preserve literal mechanism rather than inventing intent.

Use established fixtures and the shared replay/checkpoint schema. Do not redefine oracle expectations to make implementation pass.

---

# Verifier instructions

Canonical evidence must identify:

- ROM manifest;
- MAME build/version;
- Step 0 machine-map revision;
- harness/experiment identity;
- start condition;
- debugger/input scripts;
- trace flags;
- captured state.

Own the harness architecture described in `MAME_HARNESS.md`, including deterministic input, bounded waits, fixtures/save states, normalized checkpoints, failure telemetry, differential testing, regression preservation, and original-code LCOV generation.

The Verifier checks the runtime machine inventory against the mined driver map so stale assumptions fail early.

Do not silently modify production code or oracle expectations to make verification pass.

---

# Stop conditions

Stop and create a handoff/investigation request rather than guessing when:

- driver-mined facts and runtime inventory materially disagree;
- required evidence is missing or contradictory;
- ROM/MAME provenance mismatches;
- replay is not deterministic;
- a selector cannot be justified from evidence;
- a wait cannot be bounded reliably;
- an assembler/encoder cannot be independently validated;
- implementation conflicts with a trusted oracle fixture;
- changing an expected result would require convenience rather than new evidence.

An explicit `UNKNOWN` or `ASSUMPTION CONFLICT` is a valid and useful result.

---

# Session hygiene

Leave persistent evidence, commands, tests, selectors, fixtures, harness capabilities, driver-mining updates, and concise project-state changes in the repository.

Do not rely on hidden agent context or previous chat history for facts another agent will need.

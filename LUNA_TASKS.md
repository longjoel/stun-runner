# Luna-Sized Work Items

For the current phase, assume Luna is the default execution model.

The project must therefore decompose work so Luna is asked to perform bounded, externally verifiable engineering tasks rather than open-ended reverse engineering.

## Core rule

A Luna work item should have:

- one primary objective;
- a small, explicit input surface;
- one or two concrete output artifacts;
- acceptance checks that do not depend on the model's own interpretation;
- a clear stop condition;
- no requirement to infer broad subsystem semantics.

If a task cannot be described this way, split it before assigning it to Luna.

## Size limits

Prefer Luna tasks with all of the following properties:

- touches roughly 1-5 files;
- concerns one tool, one experiment, one address range, one schema, or one processor boundary;
- can be validated by tests, hashes, MAME output, a parser, a schema, or an existing project contract;
- avoids simultaneous discovery + architecture + implementation;
- can stop cleanly when required evidence is unavailable.

A task is too large for Luna when it asks for things like:

- "understand the graphics system";
- "decompile the boot process";
- "port the game";
- "figure out the DSP";
- "implement the MAME harness" as one issue;
- "map all inter-CPU communication" as one issue.

Split those into small evidence-producing and implementation-producing tasks instead.

## Work item template

Use this shape for Luna tickets:

```markdown
## Objective
One concrete outcome.

## Inputs
- exact files / commands / evidence to read
- exact schema or contract to follow

## Output
- exact file(s), command(s), or test(s) to create/update

## Acceptance
- machine-checkable or externally verifiable conditions

## Stop conditions
Stop without guessing if:
- required evidence is missing;
- MAME/device/runtime state contradicts the inputs;
- the task requires semantic inference not supported by evidence;
- a required tool/API is unavailable.

## Out of scope
Explicitly list adjacent work Luna must not expand into.
```

## Escalation rule

Do not ask Luna to push through ambiguity.

When Luna reaches a stop condition, the correct result is a concise handoff containing:

- what was attempted;
- what evidence was found;
- what remains unknown;
- the smallest next experiment or decision needed.

The project should prefer many successful small Luna tasks over one long speculative run.

---

# Initial M0 Luna backlog

These work items are intentionally ordered so later tasks consume artifacts produced by earlier ones.

## L0-001 — Local environment inventory

### Objective
Create one project command that reports the presence and versions of required local host tools without modifying the system.

### Inputs
- `PREFLIGHT.md`
- `REQUIREMENTS.md`

### Output
- `tools/env-check`
- tests for parsing/version reporting where practical

### Acceptance
The command reports at least:
- MAME executable/version if present;
- Python;
- CMake;
- Ninja;
- GCC/Clang;
- LCOV/genhtml;
- candidate m68k and 6502 toolchains when present.

Missing tools are reported clearly and do not crash the command.

### Stop conditions
Do not install packages or choose unresolved toolchains.

### Out of scope
No ROM access. No MAME launching. No build-system scaffolding beyond what the check itself needs.

---

## L0-002 — ROM manifest parser

### Objective
Parse pinned MAME `-listxml` output for `stunrun` into the documented checked-in manifest schema.

### Inputs
- generated local `stunrun.xml` fixture or a small legal test fixture;
- `REQUIREMENTS.md`.

### Output
- `tools/mame-manifest`
- schema/example for `roms/stunrun.manifest.json`
- parser tests using legal synthetic XML.

### Acceptance
For every ROM entry represented in the fixture, output captures filename, region/tag, size, CRC32, SHA-1, offset, and load/interleave metadata where exposed.

### Stop conditions
Do not guess fields that MAME does not emit.

### Out of scope
No ROM bytes. No downloading ROMs. No hash verification against local copyrighted files yet.

---

## L0-003 — MAME runtime inventory Lua script

### Objective
Create a generic Lua script that emits a machine-readable inventory of debugger-visible devices, CPUs, state/register entries, address spaces, screens, and input ports.

### Inputs
- `MAME_INSTRUMENTATION.md`
- `MAME_HARNESS.md`
- `analysis/driver-mining/stunrun.machine-map.yaml`

### Output
- `mame/lua/inventory.lua`
- documented JSON schema/example

### Acceptance
On a supported MAME launch, the script can emit JSON without target-specific hard-coded CPU tags.

### Stop conditions
If a desired MAME Lua property is unavailable in the pinned API, record it as unsupported instead of inventing an alternate meaning.

### Out of scope
No semantic selectors. No tracing. No input replay.

---

## L0-004 — Driver-map/runtime reconciliation tool

### Objective
Compare Step 0 machine-map expectations with runtime inventory and report matches, absent optional devices, and contradictions.

### Inputs
- `analysis/driver-mining/stunrun.machine-map.yaml`
- runtime inventory JSON from L0-003.

### Output
- `tools/check-machine-map`
- tests using synthetic inventories.

### Acceptance
The tool distinguishes:
- expected and present;
- explicitly optional and absent;
- unexpected runtime device;
- required device missing.

### Stop conditions
Do not reinterpret device semantics.

### Out of scope
No automatic modification of the Step 0 map.

---

## L0-005 — Deterministic replay schema

### Objective
Define and validate the target-independent experiment/replay schema.

### Inputs
- `PREFLIGHT.md`
- `MAME_HARNESS.md`

### Output
- `schemas/experiment.schema.json`
- legal examples including `boot_to_title` and `coin_start`
- schema validation tests

### Acceptance
Schema supports:
- experiment ID;
- start condition;
- frame-based events;
- digital controls;
- analog values;
- comments/labels;
- terminal checkpoint;
- versioning.

### Stop conditions
Do not choose MAME-specific implementation mechanisms in the schema.

### Out of scope
No actual MAME input injection yet.

---

## L0-006 — Basic MAME replay adapter

### Objective
Consume the experiment schema and inject only simple digital input events into MAME at specified frames.

### Inputs
- L0-003 runtime inventory capability;
- L0-005 experiment schema;
- `MAME_HARNESS.md`.

### Output
- reusable Lua replay module;
- one bounded smoke experiment.

### Acceptance
A synthetic or locally runnable experiment can schedule press/release events by frame and terminate after a bounded number of frames.

### Stop conditions
Stop if the pinned MAME Lua API cannot provide deterministic frame/input behavior; record the smallest missing capability.

### Out of scope
No semantic state detection. No analog controls yet. No save-state fixtures.

---

## L0-007 — Checkpoint metadata writer

### Objective
Write canonical experiment metadata in the format required by `EVIDENCE.md`.

### Inputs
- runtime inventory;
- experiment definition;
- ROM manifest identity;
- pinned MAME identity.

### Output
- metadata writer/module;
- schema/tests.

### Acceptance
Generated metadata identifies MAME version, ROM manifest digest, experiment ID, start condition, input script, machine-map revision, and captured artifact list.

### Stop conditions
Do not claim unavailable provenance fields.

### Out of scope
No memory/register capture yet.

---

## L0-008 — Register snapshot capture

### Objective
Capture selected registers/state entries for active programmable processors into normalized JSON.

### Inputs
- runtime inventory;
- machine map;
- checkpoint metadata module.

### Output
- reusable register snapshot module;
- tests for normalization logic.

### Acceptance
Missing optional devices are skipped with explicit metadata; active devices produce stable named fields.

### Stop conditions
Do not guess equivalent register names across CPU types.

### Out of scope
No semantic interpretation of register values.

---

## L0-009 — Memory range capture

### Objective
Capture configured address-space ranges into deterministic binary files plus metadata.

### Inputs
- machine map regions;
- runtime inventory.

### Output
- reusable memory capture module;
- configuration example for ADSP program/data windows.

### Acceptance
The module validates range, device, width/endian assumptions available from MAME and records capture provenance.

### Stop conditions
Do not infer which ranges matter semantically beyond configuration provided by the caller.

### Out of scope
No diffing or semantic decoding.

---

## L0-010 — Screenshot/checkpoint bundle assembly

### Objective
Assemble metadata, selected register snapshots, selected memory captures, and screenshot into one checkpoint directory.

### Inputs
- L0-007, L0-008, L0-009;
- MAME screenshot capability.

### Output
- checkpoint bundle writer;
- deterministic directory layout.

### Acceptance
One harness command/function produces the documented bundle shape and records which artifacts succeeded or were unavailable.

### Stop conditions
Do not add new capture types simply because MAME exposes them.

### Out of scope
No golden comparison yet.

---

## L0-011 — Bounded debugger trace controller

### Objective
Provide a harness helper that turns MAME instruction tracing on/off around a bounded experiment for one selected active CPU.

### Inputs
- runtime device tags;
- `MAME_INSTRUMENTATION.md`;
- `EVIDENCE.md`.

### Output
- trace helper;
- first 68010 boot trace recipe.

### Acceptance
Trace is explicitly associated with experiment metadata and uses exact device tags; canonical mode can request `noloop`.

### Stop conditions
Do not attempt multi-CPU orchestration in this task.

### Out of scope
No trace parsing or coverage conversion.

---

## L0-012 — 68010 trace parser

### Objective
Parse the exact trace format produced by L0-011 into normalized instruction-hit records.

### Inputs
- one small legal/synthetic trace fixture plus locally generated format samples.

### Output
- parser;
- unit tests;
- normalized JSON/CSV hit representation.

### Acceptance
Parser extracts at least PC/address and hit count without depending on semantic instruction interpretation.

### Stop conditions
If trace formatting varies by MAME version, preserve version-specific parsing explicitly.

### Out of scope
No LCOV generation yet.

---

## L0-013 — Stable pseudo-source listing generator

### Objective
Generate one-instruction-per-line pseudo-source from a stable 68010 listing for LCOV mapping.

### Inputs
- static MAME listing;
- `LCOV.md`.

### Output
- pseudo-source file;
- address-to-line map;
- tests for stable mapping.

### Acceptance
Same input listing produces identical pseudo-source and mapping.

### Stop conditions
Do not use mutable human annotations as the source mapping.

### Out of scope
No coverage calculation.

---

## L0-014 — Trace-to-LCOV converter

### Objective
Convert normalized 68010 address hits into LCOV `.info` against the stable pseudo-source map.

### Inputs
- L0-012 hit records;
- L0-013 mapping.

### Output
- `coverage/original-observed.info` generation path;
- tests.

### Acceptance
`lcov`/`genhtml` accepts the output and known executed addresses map to the expected pseudo-source lines.

### Stop conditions
Do not synthesize function coverage until trustworthy function boundaries exist.

### Out of scope
No GSP/ADSP/6502 support in this first task.

---

## L0-015 — Failure artifact retention

### Objective
Retain compact diagnostics automatically when a bounded experiment fails or times out.

### Inputs
- replay adapter;
- checkpoint modules;
- trace controller.

### Output
- retain-on-failure policy implementation.

### Acceptance
A deliberately failing smoke experiment produces metadata, last screenshot, selected state, and configured rolling/bounded trace artifacts.

### Stop conditions
Do not implement unbounded telemetry buffers.

### Out of scope
No semantic failure classification.

---

## L0-016 — ADSP program-window write experiment

### Objective
Run one bounded experiment that records main-CPU writes to the MAME-confirmed ADSP program window during boot.

### Inputs
- Step 0 region `0x800000-0x807fff`;
- harness trace/telemetry capabilities;
- pinned runtime inventory.

### Output
- experiment definition;
- raw evidence bundle;
- concise observation report containing writer PCs, address distribution, and timing/frame range.

### Acceptance
Every claim in the report links to captured evidence. If no writes occur, that negative result is recorded.

### Stop conditions
Do not infer the purpose of uploaded words or call routines "DSP loader" without further evidence.

### Out of scope
No ADSP semantic analysis or disassembly interpretation.

---

## L0-017 — Main CPU XREF extraction for mined landmarks

### Objective
Find static 68010 references to a fixed list of Step 0 address landmarks.

### Inputs
- main CPU static listing;
- machine-map YAML.

### Output
- machine-readable XREF table;
- human-readable short report.

### Acceptance
For each configured landmark, list referencing instruction addresses or report none found.

### Stop conditions
Do not assign gameplay semantics to surrounding routines.

### Out of scope
No function-boundary inference beyond what is mechanically available.

---

## L0-018 — 68010 code-emission smoke test

### Objective
Prove one tiny 68010 assemble/encode -> expected bytes -> fixed placement -> validation path.

### Inputs
- selected candidate assembler from environment inventory;
- known tiny instruction fixture.

### Output
- reproducible script/test;
- documented emitted bytes.

### Acceptance
Known source/instruction sequence produces expected bytes deterministically and can be inspected/executed in the agreed validation path.

### Stop conditions
If no usable toolchain is installed/available, report exactly what is missing; do not switch toolchains silently.

### Out of scope
No game code replacement.

---

## L0-019 — 6502 code-emission smoke test

Same structure as L0-018, scoped only to the active JSA sound CPU toolchain.

---

## L0-020 — TMS34010 code-emission research spike

### Objective
Determine whether one maintainable TMS34010 assembler/encoder path can round-trip one known instruction.

### Inputs
- documented candidate tools only;
- one known instruction/encoding pair.

### Output
- short evidence report;
- reproducible command if successful.

### Acceptance
Either:
- one candidate produces independently verified expected bytes; or
- all tried candidates fail with exact reasons and a handoff recommending the next bounded option.

### Stop conditions
Do not start implementing a custom assembler in this ticket.

### Out of scope
No broad toolchain buildout.

---

## L0-021 — ADSP-2100 code-emission research spike

Same structure as L0-020, scoped only to one known ADSP-2100 instruction/encoding pair.

---

# What Luna should not do yet

Until the above foundation exists, do not assign Luna tickets such as:

- infer the boot state machine;
- explain the graphics architecture;
- understand the ADSP algorithm;
- recover game object structures;
- reconstruct movement physics;
- identify all functions;
- port a subsystem to native C++.

Those should first be decomposed into evidence-collection tasks, XREF extraction, bounded differential experiments, and small implementation units with explicit fixtures.

## Practical scheduling rule

At any moment, prefer a queue like:

```text
1 evidence/tooling task in progress
1 independently testable follow-up ready
1 blocked/ambiguity item waiting for human or stronger-model review
```

Do not launch many Luna agents against the same unknown subsystem concurrently. Parallelize only when the tasks have independent inputs and outputs.

## Definition of Luna-ready

A ticket is Luna-ready when a reviewer can answer **yes** to all of these:

- Is the objective one sentence?
- Are the exact inputs named?
- Are the expected artifacts named?
- Can correctness be checked without trusting Luna's prose?
- Is there a stop condition before semantic guessing begins?
- Is adjacent work explicitly out of scope?

If not, split or rewrite the ticket first.

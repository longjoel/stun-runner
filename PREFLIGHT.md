# Preflight Checklist Before Large Agent Runs

This file defines the conditions that should be satisfied before spending substantial agent time on broad reverse engineering or reconstruction.

The goal is to prevent expensive work from being performed against an unstable machine definition, ambiguous evidence, conflicting agent state, an unrepeatable local setup, or ad hoc emulator interaction that should have been automated.

## 1. One-command developer bootstrap

The repository should expose a single obvious entry point for setup and validation, eventually something like:

```text
./bootstrap
./test
./coverage
./oracle-test
```

or equivalent `make` / `just` targets.

A fresh checkout should be able to determine:

- whether required host tools are installed;
- whether the pinned MAME build/version is available;
- whether optional ROM-dependent tests can run;
- whether required compilers/assemblers/encoders are available;
- whether public unit tests pass;
- whether the MAME harness can launch and enumerate a target;
- where generated evidence, failure artifacts, and coverage reports will be written.

Do not make agents independently rediscover setup commands.

## 2. Freeze language and build-system choices

Before broad native implementation begins, choose and document:

- native implementation language and language standard;
- build system;
- unit-test framework;
- formatting/lint rules;
- native coverage instrumentation path;
- expected Linux baseline.

Recommended initial direction unless evidence argues otherwise:

```text
Native language: C++20 or later
Build: CMake + Ninja
Tests: CTest-compatible runner
Coverage: GCC/Clang instrumentation -> LCOV
Platform layer: SDL where useful, but keep game semantics independent of SDL
```

This is a project choice, not a reverse-engineering fact. Record the final choice before Agent 2 builds substantial architecture around it.

## 3. MAME harness as a required project surface

The original MAME oracle must be driven through the stable browser-automation-style harness described in `MAME_HARNESS.md` for normal repeatable tests.

The harness must provide, at minimum:

- a stable launch/session API;
- automatic machine/device inventory;
- deterministic digital and analog input injection;
- bounded frame/cycle/emulated-time waits;
- semantic or machine-state selectors;
- assertions;
- fixture/checkpoint support;
- screenshots and diagnostic state capture;
- bounded trace control;
- automatic failure artifact bundles;
- a target-independent experiment/replay schema.

Manual debugger work remains valid for exploration, but repeated procedures must be promoted into reusable harness capabilities.

The project should explicitly aim for the developer experience of Playwright/Selenium applied to arcade software.

## 4. Deterministic input/replay contract

Define one canonical machine-readable input format before end-to-end work grows.

It must represent at least:

- start condition or fixture;
- frame/cycle-relative timing;
- digital controls;
- analog controls;
- coin/start/service inputs;
- optional comments/labels;
- expected terminal selector/checkpoint;
- timeout.

Example conceptual form:

```yaml
schema: arcade-experiment/v1
id: boot_to_gameplay
start: power_on
events:
  - frame: 300
    input: COIN1
    action: press
  - frame: 301
    input: COIN1
    action: release
  - frame: 360
    input: START1
    action: press
expect:
  selector: gameplay
  timeout_frames: 600
```

The same logical input script should be translatable to:

- original MAME oracle execution;
- reproduction execution;
- native execution.

Do not allow each target to define its own unrelated replay format.

## 5. Bounded waits and failure diagnostics

No harness wait may be unbounded.

A timeout should produce a deterministic failure and retain enough diagnostic context to make the failure actionable.

Expected failure artifacts should include a useful subset of:

```text
metadata.json
input.jsonl
screenshot.png
checkpoint-before.json
checkpoint-after.json
telemetry.jsonl
last-N-instruction.trace
coverage.info
```

Prefer rolling buffers and retain-on-failure policies over maximum-volume always-on telemetry.

## 6. Semantic selector contract

Selectors are analogous to Playwright locators.

They translate machine facts into stable test concepts such as:

```text
state("title")
state("gameplay")
object("player")
mailbox("adsp")
region("game_state")
```

Early selectors may be defined by concrete memory/register predicates.

Selectors must:

- be versioned or schema-governed;
- retain evidence/provenance;
- state exact vs normalized semantics;
- be Investigator-owned semantic facts;
- never be changed merely because reconstructed code disagrees.

## 7. Fixture contract

Fixtures should provide reusable known states similar to Playwright fixtures.

Examples:

```text
power_on
title_screen
coin_inserted
first_gameplay_frame
```

A fixture may be a MAME save state, deterministic replay result, or other validated representation.

Every fixture must include:

- generation recipe;
- ROM manifest identity;
- MAME version/commit;
- harness/experiment identity;
- schema version;
- validation mechanism.

Do not accept anonymous save-state binaries as project truth.

## 8. Agent concurrency and ownership

Three persistent roles do not imply three agents should edit the same files concurrently.

Rules:

- each substantial task gets a narrow branch or worktree;
- an agent must state its role and task before editing;
- shared coordination files should be updated deliberately, not opportunistically;
- raw evidence is immutable;
- Investigator owns semantic claims and selectors;
- Implementer owns implementation files;
- Verifier owns harness API semantics, fixtures, oracle/golden expectations, differential-test architecture, and failure artifacts;
- agents should hand off through committed artifacts and `QUESTIONS.md`, not unstated conversational context.

If two tasks require edits to the same semantic or test artifact, serialize them or explicitly reconcile them through review.

## 9. Stop conditions

Agents must know when to stop rather than spending tokens filling gaps with guesses.

Stop and create an investigation/handoff request when:

- required evidence is missing;
- two evidence sources materially disagree;
- the processor/device tag cannot be identified confidently;
- an expected ROM/hash/version mismatch occurs;
- deterministic replay cannot be reproduced;
- a selector cannot be justified from evidence;
- a wait cannot be bounded reliably;
- a fixture cannot be regenerated or validated;
- an assembler/encoder emits bytes that cannot be independently validated;
- implementation behavior conflicts with an established oracle fixture;
- a semantic conclusion would require assuming undocumented intent;
- progress requires changing an oracle expected value without new original-machine evidence.

An explicit `UNKNOWN` is better than an expensive hallucinated subsystem.

## 10. Cost-control rules

Large agent runs should operate on bounded objectives.

Prefer tasks such as:

```text
Implement generic inventory and device enumeration.
Implement the title-screen selector and prove it against two runs.
Identify and document reset -> main initialization path.
Map the main CPU <-> ADSP mailbox.
Annotate only routines executed by boot_to_title.
Implement only the verified fixed-point transform used by fixture X.
Explain why oracle test Y diverges.
```

Avoid prompts such as:

```text
Decompile the game.
Understand the graphics system.
Port everything to C++.
```

Before a long task, define:

- input evidence;
- expected artifacts;
- success condition;
- stop condition;
- maximum intended scope.

## 11. Artifact and storage policy

Generated artifacts fall into three classes.

### Commit normally

- harness code;
- scripts;
- manifests/hashes;
- experiment/replay definitions;
- selectors and their provenance;
- small deterministic fixtures;
- curated traces/excerpts;
- semantic metadata;
- coverage mapping files;
- test expectations with legal provenance;
- build and CI configuration.

### Regenerate locally unless specifically curated

- huge instruction traces;
- full memory dumps;
- large LCOV HTML reports;
- temporary MAME output;
- intermediate generated listings;
- bulky failure bundles from successful local debugging once summarized.

### Never commit

- commercial ROM contents;
- copyrighted binary assets extracted solely for redistribution;
- local paths, credentials, or private environment configuration.

Every large regenerable artifact should have enough metadata and scripts checked in to reproduce it.

## 12. Provenance and legal hygiene

For every derived fixture or golden artifact, record whether it is:

- original project-authored data;
- a numeric/state projection derived from local oracle execution;
- an emulator-generated diagnostic artifact;
- a copyrighted game asset that should remain local.

The public repository should prefer behavioral facts, hashes, state values, selectors, experiments, and tooling over redistributing original game content.

## 13. Independent verification

Reduce correlated agent mistakes.

The Verifier should verify behavior against the original oracle, not merely compare Agent 2 against Agent 1's interpretation.

Where practical:

- derive expected state independently from MAME evidence;
- keep test fixtures linked to raw evidence;
- re-run canonical experiments after major semantic changes;
- do not accept "both reconstructed targets agree" as proof when both came from the same incorrect hypothesis.

## 14. Schema stability

Before producing large quantities of data, define versioned schemas for:

- ROM manifest;
- experiment/input replay;
- semantic selectors;
- fixture metadata;
- checkpoint state;
- evidence metadata;
- trace-derived fixtures;
- symbol/semantic records;
- original execution coverage mapping;
- failure artifact metadata.

Each generated artifact should carry a schema version so tooling can reject stale incompatible data rather than misinterpret it.

## 15. Preflight gate

Large-scale reverse engineering may begin when all of the following are true:

- [ ] PR containing the process/requirements/testing/harness contracts is merged.
- [ ] Native language/build/test choices are frozen.
- [ ] One-command bootstrap/test/oracle entry points exist.
- [ ] Pinned MAME version/commit is recorded.
- [ ] Canonical ROM set is selected and MAME-derived manifest is checked in.
- [ ] Local ROM set validates against MAME.
- [ ] Harness launches the target through a stable entry point.
- [ ] Harness inventory records debugger-visible processor/device tags.
- [ ] Static-listing generation works for all programmable processors.
- [ ] Deterministic replay schema and first replay exist.
- [ ] At least one semantic or machine-state selector works reproducibly.
- [ ] All harness waits are bounded and timeouts fail cleanly.
- [ ] At least one assertion executes through the harness.
- [ ] At least one fixture or canonical start state can be generated and validated.
- [ ] A deliberately failing test automatically retains useful failure diagnostics.
- [ ] First canonical reset/title checkpoint bundle can be regenerated through the harness.
- [ ] A bounded MAME debugger trace can be started/stopped through the harness.
- [ ] Minimal code-emission path is validated for 68010, 6502, TMS34010, and ADSP-2100.
- [ ] Public tests run without ROMs.
- [ ] ROM-dependent oracle tests skip cleanly when ROMs are absent.
- [ ] LCOV native coverage pipeline works.
- [ ] Harness/MAME trace -> original-code LCOV prototype works for at least one processor.
- [ ] Agent branch/worktree and stop-condition rules are understood.

After this gate, spending substantial agent time becomes much more defensible because every result has a stable place to land and a repeatable way to be checked.

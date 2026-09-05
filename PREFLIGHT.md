# Preflight Checklist Before Large Agent Runs

This file defines the conditions that should be satisfied before spending substantial agent time on broad reverse engineering or reconstruction.

The goal is to prevent expensive work from being performed against an unstable machine definition, ambiguous evidence, conflicting agent state, or an unrepeatable local setup.

## 1. One-command developer bootstrap

The repository should expose a single obvious entry point for setup and validation, eventually something like:

```text
./bootstrap
./test
./coverage
```

or equivalent `make` / `just` targets.

A fresh checkout should be able to determine:

- whether required host tools are installed;
- whether the pinned MAME build/version is available;
- whether optional ROM-dependent tests can run;
- whether required compilers/assemblers/encoders are available;
- whether public unit tests pass;
- where generated evidence and coverage artifacts will be written.

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
Tests: lightweight C/C++ test framework or CTest-compatible runner
Coverage: GCC/Clang instrumentation -> LCOV
Platform layer: SDL where useful, but keep game semantics independent of SDL
```

This is a project choice, not a reverse-engineering fact. Record the final choice before Agent 2 builds substantial architecture around it.

## 3. Deterministic input/replay contract

Define one canonical machine-readable input format before end-to-end work grows.

It must represent at least:

- start condition;
- frame or cycle-relative timing;
- digital controls;
- analog controls;
- coin/start/service inputs;
- optional comments/labels;
- expected terminal checkpoint.

Example conceptual form:

```yaml
experiment: boot_to_gameplay
start: power_on
events:
  - frame: 300
    input: COIN1
    value: pressed
  - frame: 301
    input: COIN1
    value: released
  - frame: 360
    input: START1
    value: pressed
checkpoint: first_gameplay_frame
```

The same logical input script should be translatable to:

- original MAME oracle execution;
- reproduction execution;
- native execution.

Do not allow each target to define its own unrelated replay format.

## 4. Agent concurrency and ownership

Three persistent roles do not imply three agents should edit the same files concurrently.

Rules:

- each substantial task gets a narrow branch or worktree;
- an agent must state its role and task before editing;
- shared coordination files should be updated deliberately, not opportunistically;
- raw evidence is immutable;
- Investigator owns semantic claims;
- Implementer owns implementation files;
- Verifier owns oracle/golden expectations and differential-test architecture;
- agents should hand off through committed artifacts and `QUESTIONS.md`, not unstated conversational context.

If two tasks require edits to the same semantic or test artifact, serialize them or explicitly reconcile them through review.

## 5. Stop conditions

Agents must know when to stop rather than spending tokens filling gaps with guesses.

Stop and create an investigation/handoff request when:

- required evidence is missing;
- two evidence sources materially disagree;
- the processor/device tag cannot be identified confidently;
- an expected ROM/hash/version mismatch occurs;
- deterministic replay cannot be reproduced;
- an assembler/encoder emits bytes that cannot be independently validated;
- implementation behavior conflicts with an established oracle fixture;
- a semantic conclusion would require assuming undocumented intent;
- progress requires changing an oracle expected value without new original-machine evidence.

An explicit `UNKNOWN` is better than an expensive hallucinated subsystem.

## 6. Cost-control rules

Large agent runs should operate on bounded objectives.

Prefer tasks such as:

```text
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

## 7. Artifact and storage policy

Generated artifacts fall into three classes.

### Commit normally

- scripts;
- manifests/hashes;
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
- intermediate generated listings.

### Never commit

- commercial ROM contents;
- copyrighted binary assets extracted solely for redistribution;
- local paths, credentials, or private environment configuration.

Every large regenerable artifact should have enough metadata and scripts checked in to reproduce it.

## 8. Provenance and legal hygiene

For every derived fixture or golden artifact, record whether it is:

- original project-authored data;
- a numeric/state projection derived from local oracle execution;
- an emulator-generated diagnostic artifact;
- a copyrighted game asset that should remain local.

The public repository should prefer behavioral facts, hashes, state values, and tooling over redistributing original game content.

## 9. Independent verification

Reduce correlated agent mistakes.

The Verifier should verify behavior against the original oracle, not merely compare Agent 2 against Agent 1's interpretation.

Where practical:

- derive expected state independently from MAME evidence;
- keep test fixtures linked to raw evidence;
- re-run canonical experiments after major semantic changes;
- do not accept "both reconstructed targets agree" as proof when both came from the same incorrect hypothesis.

## 10. Schema stability

Before producing large quantities of data, define versioned schemas for:

- ROM manifest;
- experiment/input replay;
- checkpoint state;
- evidence metadata;
- trace-derived fixtures;
- symbol/semantic records;
- original execution coverage mapping.

Each generated artifact should carry a schema version so tooling can reject stale incompatible data rather than misinterpret it.

## 11. Preflight gate

Large-scale reverse engineering may begin when all of the following are true:

- [ ] PR containing the process/requirements/testing contracts is merged.
- [ ] Native language/build/test choices are frozen.
- [ ] One-command bootstrap/test entry points exist.
- [ ] Pinned MAME version/commit is recorded.
- [ ] Canonical ROM set is selected and MAME-derived manifest is checked in.
- [ ] Local ROM set validates against MAME.
- [ ] Debugger-visible processor/device tags are recorded.
- [ ] Static-listing generation works for all programmable processors.
- [ ] Deterministic replay schema and first replay exist.
- [ ] First canonical reset/title checkpoint bundle can be regenerated.
- [ ] Minimal code-emission path is validated for 68010, 6502, TMS34010, and ADSP-2100.
- [ ] Public tests run without ROMs.
- [ ] ROM-dependent oracle tests skip cleanly when ROMs are absent.
- [ ] LCOV native coverage pipeline works.
- [ ] MAME-trace -> original-code LCOV prototype works for at least one processor.
- [ ] Agent branch/worktree and stop-condition rules are understood.

After this gate, spending substantial agent time becomes much more defensible because every result has a stable place to land and a repeatable way to be checked.

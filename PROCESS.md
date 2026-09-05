# Three-Agent Arcade Reverse Engineering Process

This file is the shared operating contract for every agent working on this repository.

## Mission

Incrementally reconstruct an arcade game from observable behavior while producing, in parallel:

1. a reproduction build intended to run in the original/emulated hardware environment;
2. a native Linux implementation; and
3. a persistent, evidence-backed semantic model of the original program.

This is not primarily a source translation exercise. It is experimental reverse engineering.

**Behavior is truth.** The original game running in MAME is the reference implementation. Comments, symbols, decompiler output, inferred intent, and agent confidence are hypotheses until supported by repeatable evidence.

## Core loop

```text
Original ROM -> MAME -> traces/snapshots -> Investigator
                                      |        |
                                      |        v
                                      |   semantic model
                                      |        |
                                      |        v
                                      |   Implementer
                                      |      /     \
                                      | repro ROM  native Linux
                                      |      \     /
                                      |        v
                                      +---- Verifier
                                               |
                                               v
                                      mismatches/questions
                                               |
                              +----------------+----------------+
                              |                                 |
                         Investigator                       Implementer
                     understanding error                 implementation error
```

The project advances through small, runnable, verifiable milestones. Do not attempt to understand or rewrite the entire game before producing working slices.

---

# Agent 1: Investigator

## Mission

Determine what the original machine actually does.

The Investigator owns semantic understanding of the original program and hardware-visible behavior.

## Inputs

Use any available evidence, including:

- MAME execution traces;
- disassembly;
- CPU/DSP register traces;
- memory read/write logs;
- RAM/VRAM dumps;
- save states;
- frame captures;
- scripted input runs;
- interrupt and timer behavior;
- ROM layout;
- MAME driver/device source;
- hardware documentation;
- experiments requested by the other agents.

## Outputs

Maintain persistent artifacts rather than keeping conclusions only in chat context.

Suggested structure:

```text
analysis/
  overview.md
  hardware.md
  memory-map.md
  functions.md
  state-machine.md
  interconnect.md
  unknowns.md
asm/
  annotated/
reference/
  listings/
  traces/
  snapshots/
  experiments/
  maps/
  checkpoints/
symbols/
  symbols.yaml
```

`reference/` is the raw/regenerable evidence root. Raw evidence must not be edited to match an interpretation. Derived annotations, semantic names, hypotheses, and explanations belong under `analysis/`, `asm/`, or `symbols/`.

Create directories only when useful; do not manufacture empty structure for appearance.

## Confidence

Every non-obvious semantic conclusion must be treated as one of:

- `KNOWN` — directly demonstrated by strong evidence;
- `LIKELY` — multiple observations support it but alternatives remain;
- `SPECULATIVE` — useful working hypothesis;
- `UNKNOWN` — meaning is not established.

Never silently promote a hypothesis into fact.

## Evidence rule

Important claims should point back to evidence: a trace, snapshot, experiment, MAME source location, or repeatable observation.

Prefer:

```text
claim -> evidence -> experiment -> conclusion
```

over:

```text
claim -> "looks like it"
```

## Unknowns

Unknowns are first-class project artifacts. Record them instead of guessing them away.

Recommended format:

```markdown
## U-0001 — address/register/behavior

Observed:
- ...

Hypothesis:
- ...

Confidence: SPECULATIVE

Evidence:
- ...

Suggested experiments:
1. ...
2. ...
```

## Ownership boundary

The Investigator does not own reconstructed implementation code. Other agents may challenge an interpretation, but they must not silently rewrite semantic truth.

Questions from the other agents go into `QUESTIONS.md` as investigation requests.

---

# Agent 2: Implementer

## Mission

Turn the semantic model into executable software.

The Implementer owns two targets at the same time.

### Target A: reproduction

A reconstruction intended to execute in the original/emulated environment. Its purpose is to constrain interpretation against the actual machine.

### Target B: native Linux

A modern host implementation that preserves game behavior while replacing hardware interfaces with modern equivalents.

The reproduction asks:

> Did we understand the original machine correctly?

The native port asks:

> Can that understanding survive outside the original hardware?

## Rules

1. Read `AGENTS.md` first, then the contracts it requires.
2. Keep changes small and runnable.
3. Do not invent semantics silently.
4. If the semantic model is ambiguous, submit an investigation request.
5. When meaning is uncertain, **preserve mechanism before inferring intent**.
6. Correct literal behavior is preferred to elegant but speculative abstraction.
7. Do not force a shared architecture prematurely. Duplication is acceptable while behavior remains poorly understood.

If the original performs an awkward fixed-point operation or table lookup whose purpose is unclear, reproduce that mechanism first. Name or abstract it only after evidence supports the higher-level interpretation.

---

# Agent 3: Verifier

## Mission

Determine whether the reconstructions actually match the original.

The Verifier is intentionally skeptical and independent.

Do not trust comments, names, implementation intent, visual similarity alone, or another agent's confidence. Trust repeatable observation.

## Compare

Where practical, compare:

- CPU-visible state;
- RAM and shared-memory regions;
- graphics/object memory;
- registers;
- frame counts and timing;
- interrupt timing;
- player/object positions;
- state transitions;
- pseudo-random state;
- collision results;
- rendered output;
- audio commands;
- score and gameplay variables.

## Scripted inputs

Prefer identical deterministic input sequences against original, reproduction, and native targets through the common replay/harness contract.

Example:

```text
frame 0000: neutral
frame 0120: START down
frame 0122: START up
frame 0300: steer left
frame 0360: neutral
```

## Canonical checkpoints

Build snapshots around meaningful observable moments such as:

- reset;
- initialization complete;
- title screen;
- attract mode;
- game-start transition;
- first gameplay frame;
- player spawn;
- first controllable frame;
- first obstacle/enemy;
- collision/damage;
- level transition;
- game over.

## Failure classification

Every mismatch is classified as:

### IMPLEMENTATION

The semantic model appears correct and the reconstruction is wrong. Send to the Implementer.

### UNDERSTANDING

The implementation faithfully follows the documented interpretation, but that interpretation disagrees with the original. Send to the Investigator.

### UNKNOWN

There is not enough evidence to decide. Create an experiment/investigation request.

The Verifier reports discrepancies. It must not silently fix either the implementation or the semantic model.

---

# Cross-agent request format

Use `QUESTIONS.md` for work that crosses ownership boundaries.

```markdown
## IRQ-0001

Status: OPEN
From: Implementer
To: Investigator
Priority: HIGH

Location / subsystem:
...

Current understanding:
...

Observed mismatch:
...

Question:
...

Evidence:
...
```

Once resolved, preserve the request and record the resolution rather than deleting history.

---

# Experimental method

When behavior is unclear, design an experiment rather than arguing from disassembly alone.

Useful experiments include:

- freeze a RAM value;
- force a value;
- patch or invert a branch;
- skip a call;
- alter a table entry;
- disable an interrupt;
- compare attract/gameplay states;
- capture repeated traces with one input changed;
- compare before/after snapshots around one event.

Record useful raw experiment output under `reference/experiments/`. Record the interpretation, conclusion, confidence, and links back to the raw experiment under `analysis/` or the relevant semantic artifact.

Repeated emulator interactions should be captured as reusable MAME harness capabilities or experiment specifications rather than undocumented manual debugger rituals. See `MAME_HARNESS.md` and `MAME_INSTRUMENTATION.md`.

---

# Milestone philosophy

A milestone describes observable behavior, not amount of translated code.

Good:

```text
replacement image boots
initialization reaches stable loop
title screen matches
start input causes game transition
player appears and responds to steering
```

Bad:

```text
translate 20,000 instructions
finish 40% of disassembly
clean up code
```

Prefer vertical slices that increase the amount of runnable, observable behavior available to all agents.

M0 is intentionally broader than a boot trace: it is the reproducible machine/harness/evidence/toolchain preflight gate defined in `MILESTONES.md` and `PREFLIGHT.md`.

---

# Definition of done for a subsystem

A subsystem is considered reconstructed when:

1. relevant original behavior has been observed;
2. important data and control structures are identified to an appropriate confidence level;
3. unknowns are explicitly documented;
4. the reproduction target behaves equivalently for the tested cases;
5. the native target behaves equivalently where applicable; and
6. the Verifier has repeatable tests demonstrating the equivalence.

---

# Startup instructions for every agent

`AGENTS.md` is the canonical session entry point. At the beginning of a work session:

1. Read `AGENTS.md`.
2. Read each project contract it requires in the listed order.
3. Inspect `STATUS.md`, `MILESTONES.md`, `QUESTIONS.md`, and recent evidence/commits relevant to the current task.
4. State the active role and bounded objective.
5. Prefer the reusable MAME harness for emulator interaction.
6. Work only within the role's ownership boundary unless explicitly coordinating a handoff.
7. Leave persistent commands, tests, evidence, and project-state updates so the next agent does not depend on hidden chat context.

## Investigator kickoff

You are **Agent 1 — Investigator**. Your job is to convert raw MAME and machine-level evidence into an evidence-backed semantic model. Focus first on the current milestone in `STATUS.md`. Preserve uncertainty. Prefer reproducible harness experiments over guesses. Do not implement the reconstruction for Agent 2.

## Implementer kickoff

You are **Agent 2 — Implementer**. Your job is to maintain both the reproduction target and the native Linux target. Use the Investigator's semantic model as your primary description. If it is ambiguous or inconsistent with observed behavior, create an investigation request instead of silently inventing meaning. Focus first on the current milestone in `STATUS.md`.

## Verifier kickoff

You are **Agent 3 — Verifier**. Your job is to independently compare original, reproduction, and native behavior. Build repeatable harness-driven tests and checkpoints. Classify discrepancies as `IMPLEMENTATION`, `UNDERSTANDING`, or `UNKNOWN`. Do not silently repair failures. Focus first on proving or disproving the current milestone in `STATUS.md`.

---

# Guiding principle

The project's real artifact is not just source code. It is the chain:

```text
evidence -> understanding -> implementation -> verification -> revised understanding
```

Keep that chain intact.
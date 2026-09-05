# Testing and Verification Strategy

This project is treated as a traditional software project with an unusual advantage: the original arcade program running under MAME is the executable specification and behavioral oracle.

The goal is not merely to produce source that looks plausible. The goal is to continuously prove that reconstructed behavior matches the original.

## Harness-first rule

Oracle-facing tests should use the stable automation layer described in `MAME_HARNESS.md` whenever practical.

Treat MAME the way web developers treat a browser under Playwright or Selenium:

```text
browser -> MAME
page/DOM -> machine state
locator -> semantic selector
click/type -> input injection
fixture -> save state/checkpoint
trace viewer -> failure artifact bundle
```

Do not make one-off debugger rituals the default test interface. If an operation is repeated, promote it into the harness as a reusable selector, fixture, assertion, experiment, or diagnostic capability.

## Testing pyramid

```text
                    End-to-End
          original ↔ reproduction ↔ native

                 Integration
       CPU boundaries / shared memory /
       renderer / input / sound / timing

                      Unit
       routines / math / tables / state
```

All three layers are required.

---

## 1. Unit tests

Once the Investigator establishes a routine, data transform, table interpretation, or state transition with sufficient confidence, the Implementer should create ordinary fast unit tests around the reconstructed implementation.

Fixtures should be derived from observed original behavior whenever possible.

Example:

```yaml
case: velocity_update_001
source:
  trace: gameplay-left-0031
  cpu: maincpu
  pc: 0x00127a
confidence: KNOWN
input:
  velocity: 0x0120
  acceleration: 0xfff8
expected:
  velocity: 0x0118
```

Every evidence-derived test fixture should retain provenance so future agents can determine why the expected value exists.

Unit tests should cover at least:

- arithmetic and fixed-point behavior;
- table decoding;
- state transitions;
- object update functions;
- command encoding/decoding;
- input normalization;
- renderer-independent geometry/math routines;
- trace/listing parsers and project tooling;
- harness selectors, replay parsing, assertions, and artifact-generation logic.

---

## 2. Integration tests

Integration tests validate boundaries between reconstructed subsystems.

Important examples for the **active target configuration** include:

- 68010 → ADSP command/mailbox behavior;
- ADSP → shared-memory result behavior;
- 68010 → GSP command submission;
- GSP/shared-state synchronization;
- 68010 → JSA II sound command behavior;
- input registers → game-state transitions;
- timer/interrupt behavior;
- renderer command stream generation.

If Step 0/runtime inventory later shows an MSP or another optional programmable device is active for a different revision/target, add integration tests for that boundary then. Do not require absent family devices for S.T.U.N. Runner's current baseline.

Whenever possible, construct integration fixtures from a canonical MAME checkpoint.

Example form:

```text
Given:
  canonical machine state immediately before geometry submission

When:
  reconstructed subsystem executes the equivalent work

Then:
  command buffer matches
  shared RAM result matches
  status flags match
  expected interrupt/state transition occurs
```

The test should specify whether exact byte-for-byte equivalence is required or whether the comparison is normalized.

---

## 3. End-to-end tests

End-to-end tests are the strongest verification layer.

Run the same logical experiment against:

1. the original ROM under the pinned MAME build;
2. the reproduction build under the same emulated machine where applicable;
3. the native Linux build.

The experiment definition should be target-independent and should be consumed through each target's adapter rather than rewritten separately.

Use:

- the same ROM revision;
- the same deterministic initial state or equivalent fixture;
- the same deterministic input stream;
- the same checkpoint definitions;
- the same semantic selectors where meaningful;
- the same comparison schema.

At checkpoints compare useful observable state such as:

- game-state identifiers;
- player position, velocity, health, score, and timers;
- object/enemy state;
- relevant RAM regions;
- command buffers;
- framebuffer or normalized visual output;
- sound command streams;
- frame counts and timing where required.

Do not require every byte of the machine to match unless that is meaningful. Hardware noise, unused RAM, timestamps, and other nondeterministic values should be explicitly excluded rather than accidentally tolerated.

---

## 4. Semantic selectors and bounded waits

Semantic selectors are the emulator equivalent of Playwright locators.

Examples may eventually include:

```text
state("title")
state("gameplay")
object("player")
mailbox("adsp")
region("game_state")
```

Initially, a selector may simply map to one or more concrete memory/register conditions.

Selectors must retain evidence/provenance and belong to the Investigator-owned semantic model. They must not be changed merely to make reconstruction behavior pass.

All waits must be bounded by frames, cycles, or emulated time.

A timed-out wait is a test failure and should trigger automatic diagnostic capture. Never allow an oracle test to wait forever for a condition.

---

## 5. Canonical normalized state

Agent 3 should maintain a machine-readable normalized checkpoint representation.

Example:

```json
{
  "checkpoint": "player_spawn",
  "frame": 1832,
  "game_state": "gameplay",
  "player": {
    "x": 91,
    "y": 143,
    "speed": 712
  },
  "score": 12500,
  "objects": 17,
  "regions": {
    "game_state_ram": "sha256:..."
  }
}
```

This normalized form is especially important when the native port intentionally differs from original hardware representation while preserving gameplay behavior.

---

## 6. Fixtures

Use Playwright-style fixture semantics for expensive or common initial states.

Examples:

```text
power_on
title_screen
coin_inserted
first_gameplay_frame
```

Fixtures may be backed by MAME save states, deterministic replay, or another reproducible mechanism.

Every fixture must have:

- a generation recipe;
- pinned MAME identity;
- ROM manifest identity;
- schema/version metadata;
- a human-readable purpose;
- a way to regenerate or validate it.

A save-state binary without metadata is not a valid project fixture.

---

## 7. Golden tests

Curated original outputs may be used as golden fixtures.

Examples:

```text
tests/golden/title/state.json
tests/golden/title/frame.png
tests/golden/player-spawn/state.json
tests/golden/player-spawn/audio-commands.bin
```

Golden files must have documented provenance and must be regenerable from the pinned MAME setup.

Do not treat giant opaque traces as golden fixtures when a compact derived fixture is sufficient.

---

## 8. Fixture compiler

Tests should not normally parse multi-gigabyte raw traces directly.

Create tooling that converts raw evidence into small stable test fixtures:

```text
MAME trace / checkpoint bundle
          ↓
fixture extraction tool
          ↓
small deterministic JSON/binary fixture
          ↓
unit/integration tests
```

Raw evidence remains immutable. Generated fixtures are curated projections of that evidence.

A fixture must record:

- source evidence identifier;
- MAME version/commit;
- ROM manifest identifier;
- processor/device;
- address/range where relevant;
- extraction tool version or commit;
- normalization rules;
- confidence level.

---

## 9. Failure artifacts

The harness should behave like Playwright's trace-on-failure tooling.

A failed oracle or E2E test should automatically retain a compact bundle such as:

```text
metadata.json
input.jsonl
screenshot.png
checkpoint-before.json
checkpoint-after.json
telemetry.jsonl
maincpu-last-N.trace
coverage.info
```

Prefer rolling buffers and retain-on-failure policies instead of writing maximum-volume diagnostics for every successful test.

Useful policies include:

```text
trace: retain-on-failure
screenshots: only-on-failure
telemetry: rolling-buffer
coverage: always
```

Failure artifacts should be sufficient for an Investigator or Verifier to reproduce the divergence without manually replaying the entire session first.

---

## 10. Coverage

Traditional native source coverage is useful but insufficient. This project tracks multiple coverage dimensions.

LCOV is the common reporting layer for coverage. See `LCOV.md` for the exact integration model.

### 10.1 Native source coverage

Use normal compiler instrumentation and LCOV/genhtml for native code and project tools.

Report at least:

- line coverage;
- branch coverage where practical;
- function coverage.

### 10.2 Original execution coverage

From MAME traces, track which original executable addresses have actually been observed executing.

The project converts stable address-hit maps into LCOV tracefiles against generated pseudo-source listings, one instruction per stable line. This allows `genhtml` to produce heatmaps for original active processor code without conflating those metrics with native source coverage.

For the current S.T.U.N. Runner working model, report per active processor where possible:

```text
68010       executed addresses / known code addresses
TMS34010 GSP
ADSP-2100
6502
```

If runtime inventory proves another programmable processor is active, add its coverage dataset then. In particular, do not require a TMS34010 MSP coverage report while Step 0/runtime inventory says the MSP is absent.

Prefer maps/heatmaps that classify addresses or functions as:

```text
HOT
EXECUTED
COLD
NEVER OBSERVED
```

### 10.3 Tested original execution coverage

Distinguish code merely observed during exploratory play from code exercised by reproducible automated experiments.

Maintain separate LCOV datasets where practical:

```text
coverage/original-observed.info
coverage/original-tested.info
```

A key metric is:

> percentage of known original executable behavior covered by a reproducible test.

### 10.4 Semantic coverage

Track understanding, for example:

```text
functions identified
functions confidence=KNOWN
functions confidence=LIKELY
functions unknown
RAM locations named
subsystems understood
inter-processor interfaces documented
```

### 10.5 Behavioral coverage

Track user-visible and subsystem milestones separately:

```text
boot             PASS
title            PASS
coin input       PASS
game start       PASS
player movement  PASS
collision        PARTIAL
enemy AI         UNKNOWN
sound            UNKNOWN
```

No single percentage should be presented as "decompilation complete." Use the dimensions together.

---

## 11. Visual verification

Visual verification should be staged to avoid brittle tests too early.

### Level 1 — semantic/state equivalence

Compare positions, game state, command streams, object state, and other renderer-independent values.

### Level 2 — normalized visual equivalence

Compare cropped regions, object positions, perceptual or tolerance-based pixel differences, or other deliberately normalized rendering output.

### Level 3 — exact framebuffer equivalence

Require exact framebuffer hashes/pixels only when the reproduction target is expected to render identically.

Native ports may legitimately diverge in presentation while preserving gameplay semantics; document the expected level per test.

---

## 12. Continuous integration

The project should support two CI modes.

### Public CI

Must not require copyrighted commercial ROM contents.

Expected jobs include:

```text
format/lint
build project tools
build native target
build reproduction toolchains where possible
assembler encoder tests
unit tests
integration tests using checked-in legal fixtures
harness parser/selector/assertion tests
trace/parser tests
LCOV capture for native/tooling code
genhtml report generation
```

### ROM-dependent oracle verification

Runs only where a valid local/private ROM collection is available.

Expected jobs include:

```text
verify ROM hashes against generated MAME manifest
launch pinned MAME through harness
replay deterministic experiment
capture original checkpoint
retain diagnostics on failure
run reproduction
run native port
compare normalized states
run golden/differential tests
generate original observed/tested LCOV datasets
generate per-processor genhtml reports
```

If ROMs are unavailable, oracle tests should be reported as skipped, not failed.

Example:

```text
PASS  unit_math
PASS  trace_parser
PASS  native_build
SKIP  oracle_boot       ROM set unavailable
SKIP  oracle_gameplay   ROM set unavailable
```

---

## 13. Regression policy

Every verified bug should become a regression test when practical.

When Agent 3 finds a mismatch:

1. capture the smallest reproducible experiment;
2. classify it as IMPLEMENTATION, UNDERSTANDING, or UNKNOWN;
3. create/update an evidence-backed fixture or selector if appropriate;
4. fix the appropriate layer;
5. keep the test permanently if it represents meaningful behavior.

Do not delete a failing behavioral test merely because the implementation has changed. Update an expected value only when new evidence demonstrates that the prior expectation was wrong.

---

## 14. Agent responsibilities

### Investigator

- discovers original behavior;
- creates evidence-backed hypotheses;
- creates/evolves semantic selectors with provenance;
- proposes or generates fixtures from original observations;
- identifies previously unexecuted code and experiments needed to reach it;
- ensures traces contain enough address information for original execution coverage;
- promotes repeated observation procedures into reusable harness capabilities where practical;
- does not alter expected results merely to accommodate implementation behavior.

### Implementer

- builds reproduction and native code against established fixtures;
- adds normal unit tests for reconstructed modules;
- enables conventional LCOV-compatible coverage for native/tooling code;
- consumes the shared replay/checkpoint contracts;
- does not silently redefine oracle expectations;
- requests investigation when a verified fixture appears incompatible with the current semantic model.

### Verifier

- owns the stable harness and target adapters;
- owns bounded waits, assertions, fixtures, and failure artifact policy;
- owns differential, integration, golden, and end-to-end verification architecture;
- owns normalized checkpoint schemas;
- owns coverage reporting for original behavior;
- owns the trace-to-LCOV conversion pipeline and generated original-code `.info` datasets;
- independently classifies mismatches;
- does not silently modify production implementation to make tests pass.

---

## 15. Progress reporting

A useful project report should eventually contain metrics similar to:

```text
Native source coverage:                 78%
Original code observed:                 61%
Original code exercised by tests:       46%
Semantically understood original code:  38%
Behavioral milestones verified:         12 / 30
Reproduction equivalence:               10 / 30
Native equivalence:                      8 / 30
```

These values measure different things and must not be conflated.

The most important long-term metric is not lines translated. It is:

> How much of the original game's behavior is understood, reproducibly exercised, reconstructed, and independently verified?

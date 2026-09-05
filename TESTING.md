# Testing and Verification Strategy

This project is treated as a traditional software project with an unusual advantage: the original arcade program running under MAME is the executable specification and behavioral oracle.

The goal is not merely to produce source that looks plausible. The goal is to continuously prove that reconstructed behavior matches the original.

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
- trace/listing parsers and project tooling.

---

## 2. Integration tests

Integration tests validate boundaries between reconstructed subsystems.

Important examples include:

- 68010 → ADSP command/mailbox behavior;
- ADSP → shared-memory result behavior;
- 68010 → GSP/MSP command submission;
- GSP/MSP shared state and synchronization;
- 68010 → JSA II sound command behavior;
- input registers → game-state transitions;
- timer/interrupt behavior;
- renderer command stream generation.

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

Run the same experiment against:

1. the original ROM under the pinned MAME build;
2. the reproduction build under the same emulated machine where applicable;
3. the native Linux build.

Use:

- the same ROM revision;
- the same deterministic initial state;
- the same deterministic input stream;
- the same checkpoint definitions;
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

## 4. Canonical normalized state

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

## 5. Golden tests

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

## 6. Fixture compiler

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

## 7. Coverage

Traditional native source coverage is useful but insufficient. This project tracks multiple coverage dimensions.

LCOV is the common reporting layer for coverage. See `LCOV.md` for the exact integration model.

### 7.1 Native source coverage

Use normal compiler instrumentation and LCOV/genhtml for native code and project tools.

Report at least:

- line coverage;
- branch coverage where practical;
- function coverage.

### 7.2 Original execution coverage

From MAME traces, track which original executable addresses have actually been observed executing.

The project converts stable address-hit maps into LCOV tracefiles against generated pseudo-source listings, one instruction per stable line. This allows `genhtml` to produce heatmaps for original 68010, TMS34010, ADSP-2100, and 6502 code without conflating those metrics with native source coverage.

Report per processor where possible:

```text
68010       executed addresses / known code addresses
TMS34010 GSP
TMS34010 MSP
ADSP-2100
6502
```

Prefer maps/heatmaps that classify addresses or functions as:

```text
HOT
EXECUTED
COLD
NEVER OBSERVED
```

### 7.3 Tested original execution coverage

Distinguish code merely observed during exploratory play from code exercised by reproducible automated experiments.

Maintain separate LCOV datasets where practical:

```text
coverage/original-observed.info
coverage/original-tested.info
```

A key metric is:

> percentage of known original executable behavior covered by a reproducible test.

### 7.4 Semantic coverage

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

### 7.5 Behavioral coverage

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

## 8. Visual verification

Visual verification should be staged to avoid brittle tests too early.

### Level 1 — semantic/state equivalence

Compare positions, game state, command streams, object state, and other renderer-independent values.

### Level 2 — normalized visual equivalence

Compare cropped regions, object positions, perceptual or tolerance-based pixel differences, or other deliberately normalized rendering output.

### Level 3 — exact framebuffer equivalence

Require exact framebuffer hashes/pixels only when the reproduction target is expected to render identically.

Native ports may legitimately diverge in presentation while preserving gameplay semantics; document the expected level per test.

---

## 9. Continuous integration

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
trace/parser tests
LCOV capture for native/tooling code
genhtml report generation
```

### ROM-dependent oracle verification

Runs only where a valid local/private ROM collection is available.

Expected jobs include:

```text
verify ROM hashes against generated MAME manifest
run pinned MAME
replay deterministic experiment
capture original checkpoint
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

## 10. Regression policy

Every verified bug should become a regression test when practical.

When Agent 3 finds a mismatch:

1. capture the smallest reproducible experiment;
2. classify it as IMPLEMENTATION, UNDERSTANDING, or UNKNOWN;
3. create/update an evidence-backed fixture;
4. fix the appropriate layer;
5. keep the test permanently if it represents meaningful behavior.

Do not delete a failing behavioral test merely because the implementation has changed. Update an expected value only when new evidence demonstrates that the prior expectation was wrong.

---

## 11. Agent responsibilities

### Investigator

- discovers original behavior;
- creates evidence-backed hypotheses;
- proposes or generates fixtures from original observations;
- identifies previously unexecuted code and experiments needed to reach it;
- ensures traces contain enough address information for original execution coverage;
- does not alter expected results merely to accommodate implementation behavior.

### Implementer

- builds reproduction and native code against established fixtures;
- adds normal unit tests for reconstructed modules;
- enables conventional LCOV-compatible coverage for native/tooling code;
- does not silently redefine oracle expectations;
- requests investigation when a verified fixture appears incompatible with the current semantic model.

### Verifier

- owns differential, integration, golden, and end-to-end verification architecture;
- owns normalized checkpoint schemas;
- owns coverage reporting for original behavior;
- owns the trace-to-LCOV conversion pipeline and generated original-code `.info` datasets;
- independently classifies mismatches;
- does not silently modify production implementation to make tests pass.

---

## 12. Progress reporting

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

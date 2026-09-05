# LCOV Coverage Integration

LCOV is the project's common coverage reporting layer.

It is used in two distinct ways:

1. **Conventional source coverage** for native/reconstructed code and project tools.
2. **Translated execution coverage** for original arcade machine code observed under MAME.

These must remain distinguishable in reports. A native C/C++ line hit is not the same thing as an original 68010/TMS34010/ADSP/6502 instruction address observed executing.

---

## 1. Native source coverage

Native and host-side project code should use normal compiler instrumentation.

Preferred paths:

- GCC: `--coverage` / gcov + LCOV
- Clang/LLVM: LLVM coverage data converted through LCOV-compatible tooling where practical

Collect at least:

- line coverage
- function coverage
- branch coverage where useful

Current LCOV supports line/function capture by default, with branch coverage enabled explicitly. Reports are rendered with `genhtml`.

A conventional flow should look like:

```sh
lcov --capture --directory build --output-file coverage/native.info --branch-coverage
genhtml coverage/native.info --branch-coverage --output-directory coverage/html/native
```

Exact build-system commands may differ once the native target is selected.

---

## 2. Original machine-code execution coverage

Original execution coverage is derived from MAME traces, not compiler instrumentation.

The project should generate one stable pseudo-source file per executable processor, for example:

```text
coverage/source/original/maincpu-68010.lst
coverage/source/original/gsp-tms34010.lst
coverage/source/original/msp-tms34010.lst
coverage/source/original/adsp2100.lst
coverage/source/original/sound-6502.lst
```

Each executable instruction occupies exactly one stable source line.

Example:

```text
00012340  4E56 FFFC        LINK.W A6,#-4
00012344  302E 0008        MOVE.W (8,A6),D0
00012348  0640 0001        ADDI.W #1,D0
0001234C  4E5E             UNLK A6
0001234E  4E75             RTS
```

The line-to-address mapping must be deterministic for a pinned ROM manifest and MAME version.

A trace-to-LCOV converter then emits standard LCOV `DA` records whose line hit counts correspond to observed instruction execution counts.

Conceptually:

```text
TN:oracle_boot
SF:/.../coverage/source/original/maincpu-68010.lst
DA:1,0
DA:2,8
DA:3,8
DA:4,8
DA:5,8
LH:4
LF:5
end_of_record
```

This lets `genhtml` render original disassembly/listings as coverage-highlighted source files.

---

## 3. Stable mapping requirement

Never derive coverage line numbers from an annotated listing whose structure changes as comments are added.

LCOV source files for original code are generated artifacts with a stable rule:

> one known executable instruction = one source line

Annotations may be displayed elsewhere, but the address-to-line mapping must stay reproducible.

The converter should persist a mapping file such as:

```json
{
  "processor": "maincpu",
  "architecture": "m68010",
  "rom_manifest": "sha256:...",
  "listing_sha256": "...",
  "instructions": {
    "0x00012340": 1,
    "0x00012344": 2,
    "0x00012348": 3
  }
}
```

---

## 4. Trace conversion pipeline

Target pipeline:

```text
MAME trace
   ↓
trace parser
   ↓
(address, execution-count) map
   ↓
stable listing/address map
   ↓
LCOV .info tracefile
   ↓
genhtml
```

Recommended tool layout:

```text
tools/coverage/
    trace_to_hits.*
    listing_map.*
    hits_to_lcov.*
    merge_coverage.*
```

The implementation language is not important; deterministic output and tests are.

---

## 5. Test names

Use LCOV test names (`TN:`) to preserve experiment identity.

Examples:

```text
TN:oracle_reset
TN:oracle_boot_to_title
TN:oracle_coin_start
TN:oracle_player_spawn
TN:oracle_level_1_drive
```

This gives us per-experiment execution coverage in addition to merged project coverage.

---

## 6. Merging experiments

LCOV tracefiles can be merged to show cumulative execution coverage.

Keep individual files:

```text
coverage/original/oracle_boot.info
coverage/original/oracle_title.info
coverage/original/oracle_gameplay_01.info
```

Then generate cumulative coverage:

```sh
lcov \
  --add-tracefile coverage/original/oracle_boot.info \
  --add-tracefile coverage/original/oracle_title.info \
  --add-tracefile coverage/original/oracle_gameplay_01.info \
  --output-file coverage/original/all.info
```

Do not discard the individual tracefiles. They answer a different question: *which experiment exercises this code?*

---

## 7. Per-processor reports

Generate both per-processor and aggregate reports.

```text
coverage/html/original/maincpu/
coverage/html/original/gsp/
coverage/html/original/msp/
coverage/html/original/adsp/
coverage/html/original/sound/
coverage/html/original/all/
```

The aggregate top-level report should retain processor names in the source paths so they remain distinguishable.

---

## 8. Function coverage for original code

Once function boundaries become reliable, the converter may emit LCOV function records for original machine code.

Do not create function records merely because a disassembler guessed a boundary.

Suggested rule:

- `KNOWN` function boundary: eligible for LCOV function coverage
- `LIKELY`: optional, clearly labeled in generated metadata
- `SPECULATIVE` / `UNKNOWN`: line/instruction coverage only

This gives us a second useful original-code metric:

> known original functions exercised by deterministic tests

---

## 9. Branch coverage

Native branch coverage should use normal compiler-generated LCOV branch data.

Original-machine branch coverage is a later feature.

It is possible to derive branch edges from static analysis plus dynamic traces, but the project must not fabricate LCOV branch records until the branch model is reliable.

Start with instruction execution coverage first.

---

## 10. Coverage categories

The generated reports should remain explicit about what they measure.

Suggested outputs:

```text
coverage/native.info
coverage/original-observed.info
coverage/original-tested.info
```

Where:

- `native.info` — compiler-instrumented reconstructed/native source
- `original-observed.info` — union of exploratory and canonical original traces
- `original-tested.info` — only deterministic automated oracle experiments

The last distinction is particularly important.

Code observed during manual exploration is useful evidence, but code exercised by reproducible tests is stronger project coverage.

---

## 11. Coverage thresholds

Do not set arbitrary high global thresholds during early reverse engineering.

Use thresholds where they make semantic sense:

- project tooling and parsers: normal software-project thresholds are appropriate
- native modules that are considered complete: increasing source-coverage thresholds are appropriate
- original execution coverage: report progress rather than failing CI solely because untouched game behavior exists

Later milestones may define minimum coverage for specific subsystems.

LCOV supports fail-under criteria, but they should be introduced deliberately rather than used as a vanity metric.

---

## 12. CI behavior

Public CI can always generate native/tooling LCOV reports.

ROM-dependent coverage is generated only in an environment with validated original ROMs.

Expected behavior:

```text
public CI:
  native coverage       PASS
  tooling coverage      PASS
  original coverage     SKIP (ROM unavailable)

local/private oracle CI:
  native coverage       PASS
  original observed     PASS
  original tested       PASS
```

Generated HTML and `.info` files may be published as CI artifacts.

---

## 13. Agent responsibilities

### Investigator

- ensures raw traces contain enough information to derive accurate address hit counts
- maintains executable-range and instruction-boundary knowledge
- does not change listing/address mappings merely to improve coverage percentages

### Implementer

- enables conventional compiler coverage for native/reconstruction host code
- keeps unit-test coverage visible
- does not treat native source coverage as proof of behavioral equivalence

### Verifier

- owns the trace-to-LCOV conversion pipeline
- owns `original-observed.info` and `original-tested.info`
- validates deterministic test-name provenance
- publishes per-processor and aggregate reports
- ensures coverage numbers are not conflated across measurement types

---

## 14. Long-term dashboard

LCOV HTML is one reporting surface, not the whole project dashboard.

A future summary should combine LCOV-derived metrics with semantic and behavioral data:

```text
Native source line coverage:              81%
Original 68010 instructions observed:     66%
Original 68010 instructions tested:       51%
Original ADSP instructions observed:      43%
Known original functions tested:          37%
Behavioral milestones verified:         14/30
```

LCOV gives us a mature renderer, merging model, per-test tracefiles, and source-level heatmaps. The project-specific tooling supplies the reverse-engineering semantics around it.

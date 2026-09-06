# Native target (Agent 2)

`smoke.cpp` is the original M0 smoke baseline, kept as-is.

## Deterministic shell (first M4 scaffolding step)

`shell.c` is a fixed 600-frame title-path walk over the five verified
reproduction slices (ADSP init image, control/IRQ contract, upload framing,
ADSP-buffer/GSP-FIFO framing, JSA latch transport). It drives each slice at its
evidence-anchored frame — startup response at 1, upload states at
407/408/411, install-ready plus image/landmarks at 412, the `0x1E`
command at 447, the control/counts match at 600 — and prints a
machine-readable `checkpoint` line plus `RESULT PASS`/`RESULT FAIL`.

Deliberate limits (see `shell.c` header): the `0x00` command reads are
telemetry, the second IRQ4 response is order-only without frame
attribution, and there is no speculative game logic. Output is fully
deterministic (no addresses, no timing); the public test runs the shell
twice and requires byte-identical stdout.

## File-driven replay (M4 scaffolding, continued)

With one argv pointing at an `arcade-experiment/v1` JSON file
(`schemas/experiment.schema.json`), the walk length comes from
`expect.frame` and the file's input events dispatch at their frames as
telemetry (inputs have no consumer model yet — the shell owns no game
state by design). `native/experiment.c` is a dependency-free C99 parser
that enforces the schema strictly (exact tag, id pattern, required keys,
press/release/set enum, set-requires-value, no unknown keys on events or
expect) with bounded resources. Whole-run contracts (control tally,
title counts) assert only for 600-frame terminals and log as skipped
otherwise; `bounded_observation` terminals and unreadable files fail
loudly (exit 2), never as silent passes.

```sh
cmake --build build/native --target stunrun-native-shell
./build/native/stunrun-native-shell experiments/stunrun/coin_start.json
```

Public ROM-free checks: `tests/test_experiment_parser.py` (parser
self-check) and the extended `tests/test_native_shell.py` (compiled-in,
canonical `coin_start`, synthetic `set`/short-terminal, and refusal
paths).

## Checkpoint emitter (`stunrun-checkpoint/v1`)

`checkpoint.h` / `checkpoint.c` render the Verifier-owned checkpoint
schema in C (machine identity, frame/time, four processor register
sets, ADSP program-region summary, `adsp_program_loaded` selector — no
gameplay fields, per `analysis/checkpoint-schema.md`). The emitter never
truncates (NULL/undersized buffer yields 0) and is deterministic.
`native/testdata/checkpoint_m1.json` is the frozen golden document;
`tests/test_checkpoint_golden.py` requires the C self-check to pass,
the emitted bytes to equal the golden file, *and* the golden file to be
semantically equal to the oracle
`reference/checkpoints/m1-machine-map/state.json` via an independent
JSON parser — so a mistranscribed constant fails even though the C side
alone would pass.

Build and run:

```sh
cmake -S native -B build/native && cmake --build build/native
ctest --test-dir build/native
```

Public ROM-free checks: `tests/test_native_shell.py` (pass +
determinism) alongside the per-slice C tests.

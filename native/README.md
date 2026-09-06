# Native target (Agent 2)

`smoke.cpp` is the original M0 smoke baseline, kept as-is.

## Deterministic shell (first M4 scaffolding step)

`shell.c` is a fixed 600-frame title-path walk over the four verified
reproduction slices (ADSP init image, control/IRQ contract, upload
framing, JSA latch transport). It drives each slice at its
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

Build and run:

```sh
cmake -S native -B build/native && cmake --build build/native
ctest --test-dir build/native
```

Public ROM-free checks: `tests/test_native_shell.py` (pass +
determinism) alongside the per-slice C tests.

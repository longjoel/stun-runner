# M0 Audit — Current System Baseline

Date: 2026-09-05

## Verified complete

- `stunrun` rev 6 ROM set selected and validated with system MAME.
- MAME 0.289 binary/version/hash and launch recipe recorded.
- MAME-derived ROM manifest checked in without ROM contents.
- Runtime device inventory and active-processor reconciliation recorded.
- Generic inventory and bounded replay entry points work.
- Boot/frame experiment repeats with identical screenshot hash.
- Bounded static listings generated for all four active programmable processors.
- Bounded `noloop` debugger traces generated and metadata recorded.
- Trace-to-standard-LCOV conversion works for the 68010 trace.
- ROM-free native smoke coverage emits standard LCOV through GCC/gcov; CMake/CTest passes.
- Minimal fixed-placement NOP emission proofs decode through MAME for 68010, TMS34010, ADSP-2100, and 6502.
- ROM-free public tests and one-command bootstrap pass.
- ROM-dependent `oracle-test` passes when `STUNRUN_ROMPATH` is supplied and skips cleanly otherwise.
- Replay failure paths preserve `mame.log` and a machine-readable `failure.json` artifact.

## Explicitly unresolved

1. The installed package reports `mame0289-dirty` and does not expose its source commit. The executable hash is pinned, but the Step 0 source refresh cannot claim exact source-build identity yet.
2. The installed npm `lcov` command is available and native/original converters emit valid `.info`; `genhtml` is not installed, so HTML rendering is unavailable.
3. The replay selector is intentionally a bounded frame machine-state condition, not a semantic title/gameplay assertion. A semantic selector remains future work.
4. Empty GSP/ADSP traces in the six-frame power-on window are observations only; they do not establish that those CPUs never execute.

## Commands

```sh
./bootstrap
STUNRUN_ROMPATH=/path/to/roms ./oracle-test
tools/mame-listings /path/to/roms /tmp/stunrun-listings
tools/mame-trace /path/to/roms /tmp/stunrun-traces :mainpcb:maincpu 6
tools/emit-proof --rompath /path/to/roms --output /tmp/stunrun-emit-proof
```

M0 provenance note: the installed package's executable/version/hash and runtime behavior are pinned, but its upstream source commit is unavailable. The adjacent source checkout is not silently substituted for that identity. This is an explicit provenance limitation for review, not an unrecorded assumption.

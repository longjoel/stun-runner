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

The shell also emits a one-line `checkpoint-json=` document using the shared
`stunrun-checkpoint/v1` emitter. It is explicitly a
`native-shell-transport-model` checkpoint: CPU register fields remain zero
until a native CPU model exists, while the ADSP image-region summary and
`adsp_program_loaded` boundary are derived from the verified shell model.

Deliberate limits (see `shell.c` header): the `0x00` command reads are
telemetry, the second IRQ4 response is order-only without frame
attribution, and there is no speculative game logic. Output is fully
deterministic (no addresses, no timing); the public test runs the shell
twice and requires byte-identical stdout.

## File-driven replay (M4 scaffolding, continued)

With one argv pointing at an `arcade-experiment/v1` JSON file
(`schemas/experiment.schema.json`), the walk length comes from
`expect.frame` and the file's input events dispatch at their frames. The shell
retains a generic latest-value latch for each port/field pair and emits a
deterministic active-count/hash boundary; this is input plumbing only, not
game semantics. `native/experiment.c` is a dependency-free C99 parser
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

## Rendering boundary (M4 scaffolding)

`render.c` / `render.h` provide a dependency-free 512x240 RGB software
framebuffer with deterministic clear, pixel, hash, and PPM-write operations.
It also exposes a clipped opaque RGB blit primitive as the literal host-side
counterpart for the unresolved GSP pixel-blit boundary.
`stunrun_render_fill_xy` provides the corresponding clipped inclusive-bounds
primitive for the traced GSP `FILL XY` path; its coordinates remain raw.
`stunrun_render_line` provides the endpoint-preserving clipped counterpart for
the traced `LINE 0` path; neither primitive assigns game meaning to inputs.
The `stunrun_render_gsp_visible` primitive applies the evidence-backed
four-lines-per-VRAM-row layout and 16-bit word byte lanes to a supplied GSP
VRAM/palette state; it does not provide that state or claim a complete game
renderer.
`stunrun_gsp_palette_decode` owns the corresponding two-plane palette
conversion observed in the snapshot exporter (low-plane high/low bytes become
R/G, high-plane low byte becomes B).
`stunrun_gsp_video_set_palette_planes` installs that conversion directly into
the native video state for future raw-snapshot producers.
`stunrun_gsp_video_render` is the state-level entry point that submits loaded
VRAM and palette data to the visible-layout renderer.
`stunrun_gsp_video_set_vram_words` provides the corresponding owned, validated
VRAM-state boundary; failed input leaves the existing state unchanged.
`stunrun_gsp_video_set_vram_bytes` accepts the little-endian byte form emitted
by the captured-state exporter and performs the conversion in C.
File loading follows the same transactional rule: malformed or incomplete
VRAM/palette files do not discard a previously loaded video state.
With no video-state inputs the native shell emits a blank-frame hash tagged
`mode=blank-scaffold`; this proves the host rendering/logging boundary without
presenting synthetic pixels as reconstructed game output. For paired MAME
evidence, export snapshots with `tools/export-gsp-native-state` and set
`STUNRUN_GSP_VRAM_BIN`, `STUNRUN_GSP_PALETTE_BIN`, and optionally
`STUNRUN_RENDER_PPM=/path/frame.ppm`. The shell then reports
`mode=gsp-visible-state` and renders through the native primitive. This is a
fixture bridge, not yet a native game-state producer. Raw little-endian MAME
palette snapshots can be supplied directly with
`STUNRUN_GSP_PALETTE_LOW_BIN` and `STUNRUN_GSP_PALETTE_HIGH_BIN`; the shell
then applies the evidence-backed two-plane decode in C instead of requiring
the RGB export step. `tools/export-gsp-native-state` can emit those files with
`--palette-low-out` and `--palette-high-out` alongside its existing RGB output.

The shell also accepts a bounded text-cursor fixture using
`STUNRUN_GSP_TEXT_TABLE_BIN` (exactly 512 little-endian 16-bit words) and
`STUNRUN_GSP_TEXT_WORDS_BIN` (packed descriptor words), plus
`STUNRUN_GSP_TEXT_A0`, `STUNRUN_GSP_TEXT_A1`, and
`STUNRUN_GSP_TEXT_Y_BIAS`. It decodes the measured low/high byte lanes and
renders the glyphs in the supplied table at the observed A1 cells. This is a
reproducible native producer boundary for captured HUD data; it deliberately
does not infer a live game-state source for those files.

For the upstream record boundary, provide the same table through
`STUNRUN_GSP_TEXT_TABLE_BIN`, replace the packed-word input with
`STUNRUN_GSP_TEXT_RECORD_BIN` (a sequence of 8-word little-endian records),
and set `STUNRUN_GSP_TEXT_RECORD_BASES` as a comma-separated list in the same
order plus `STUNRUN_GSP_TEXT_Y_BIAS`. The shell derives each record's
descriptor address and A1 cursor from its raw header and reports
`mode=gsp-text-record-fixture`. If the base-list variable is omitted,
`STUNRUN_GSP_TEXT_RECORD_BASE` plus an observed `0x80` record stride is used
as a compatibility fallback.

`tools/export-gsp-text-fixture` converts the captured
`stunrun-memory-snapshot/v1` glyph-table JSON to the table binary and accepts
descriptor words in source order, for example:

```sh
tools/export-gsp-text-fixture snapshot-1793.json \\
  --table-out glyph-table.bin --words-out credits-words.bin \\
  --word 0x7243 --word 0x6465 --word 0x7469 --word 0x3A73 \\
  --word 0x3020 --word 0x0020
```

The optional `STUNRUN_GEOM_TABLE_BIN` input accepts a 768-byte big-endian
table fixture and exercises the evidence-backed 384-word road-buffer upload
and twin-copy boundary. It intentionally does not select a course or assign
semantics to the table words; those remain Investigator-owned questions.
Use `tools/export-geometry-native-state` on a settled main-CPU snapshot
covering `0xFF9584–0xFF9B83` to produce this fixture; the exporter rejects a
nonmatching `+0x300` twin before writing it.
The shared C slice exposes only the observed finite course-to-ROM-base map;
unknown course values are rejected because no general stride is established.
The same fixture then passes through the observed 384-read/192-write
base-buffer drain to the host-side FIFO model at `0xC0000C`; payload semantics
remain open.

Convert an MAME PNG to PPM and compare it with the native output using
`tools/compare-ppm`; the tool reports dimensions, changed pixels/channels, and
absolute error rather than treating a mismatched image as a vague failure.

## Projected road-strip boundary

`road_strip.h` / `road_strip.c` define the first backend-neutral projected
strip command: four screen-space vertices plus a uniform color. They also
provide an ordered multi-strip frame fixture so the demo exercises a complete
small road surface rather than one isolated polygon. The software backend
rasterizes these deterministically for tests and PPM comparison.
`opengl_backend.c` submits the same vertices as OpenGL
`GL_TRIANGLE_STRIP`s to a caller-owned current context; it does not create a
window or assign semantics to the unresolved road-buffer fields.

The built-in geometry is explicitly an intermediate rendering fixture, not a
decoded original track segment. Enable the shell fixture with:

```sh
STUNRUN_ROAD_STRIP_FIXTURE=1 \
STUNRUN_RENDER_PPM=/tmp/stunrun-road-strip.ppm \
  ./build/native/stunrun-native-shell
```

The shell then reports `mode=road-strip-fixture`. The verified original-data
boundary remains separate: a supplied geometry table still flows through the
native game loop's 384-word dual-buffer upload and 192-word fake GSP FIFO
submission before any future projection decoder is attached.

When GLFW and OpenGL development libraries are available, CMake also builds
`stunrun-opengl-demo`. It opens a 512×240 window, advances the same fixed-step
native loop, and submits the strip through the OpenGL backend. Set
`STUNRUN_OPENGL_FRAMES=N` for a bounded demo run; omit it for an interactive
window.

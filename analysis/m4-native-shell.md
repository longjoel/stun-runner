# M4 native shell evidence

M4 is complete for the native shell contract. The Linux target has a
deterministic fixed-step clock, replay input boundary, logging/checkpoint
output, and a dependency-free software framebuffer. No game semantics or
synthetic game imagery are included.

## Verified contract

| Boundary | Evidence |
|---|---|
| Fixed timing | `native/shell.c` advances integer frames and reports `frame * 1,000,000 / 60`; the 600-frame test expects `10,000,000` |
| Input | `native/shell.c` latches the latest press/release/set value for each port/field pair and emits a deterministic FNV-1a state hash |
| Replay | `native/experiment.c` parses `arcade-experiment/v1`; `native-shell` dispatches events at their exact frames, including the canonical `coin_start` fixture |
| Rendering | `native/render.c` provides a 512×240 RGB framebuffer matching the pinned MAME screenshot geometry, clear, pixel, hash, and PPM-write operations; the shell emits `mode=blank-scaffold` |
| Checkpoint/logging | `stunrun-checkpoint/v1` JSON and deterministic textual checkpoint output are emitted by the shell |
| Semantic restraint | The framebuffer is explicitly blank until M5 supplies evidence-backed visible output; CPU fields in the native checkpoint remain zero |

## Verification

From the repository root:

```sh
python3 -m unittest discover -s tests
cmake -S native -B /tmp/stunrun-native-build
cmake --build /tmp/stunrun-native-build
ctest --test-dir /tmp/stunrun-native-build --output-on-failure
```

The verification run passed 77/77 ROM-free Python tests and 13/13 native CTest
targets. The MAME harness and original-code evidence remain the behavioral
oracle; this milestone only establishes the host-side shell.

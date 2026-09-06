# ADSP replacement image

The first M2 slice is a deliberately minimal ADSP-2100 program image. It uses
the known `NOP` encoding from the M0 emission proof, preserves the four-word
ADSP reset-vector area at `0x0000`, and records the MAME-confirmed reset entry
point at program address `0x0004`.

Build the image without ROM contents:

```sh
tools/build-replacement-image \
  --processor adsp2100 --fixture nop \
  --output /tmp/stunrun-m2-adsp-image
```

Validate placement and bounded execution in the pinned MAME runtime:

```sh
tools/mame-replacement-image \
  /tmp/stunrun-m2-adsp-image/image.json \
  --rompath /path/to/private/stunrun-roms \
  --output /tmp/stunrun-m2-adsp-run
```

The loader waits until after the original title-path upload, writes the fixed
image into the MAME-mapped ADSP program RAM, resets the ADSP PC to the image
entry, and records the read-back word and post-settle PC. This is an image
placement/runtime-integration proof, not yet a behavioral replacement for the
original ADSP program.

For a local, ROM-derived full-image experiment, capture the active ADSP RAM at
the established populated boundary (frame 136 is retained as an all-zero
negative control; the first captured populated image is frame 600):

```sh
tools/mame-adsp-program-dump /path/to/private/stunrun-roms \
  /tmp/stunrun-m3-adsp-original --frame 600
tools/mame-replacement-image \
  /tmp/stunrun-m3-adsp-original/image.json \
  --rompath /path/to/private/stunrun-roms \
  --output /tmp/stunrun-m3-adsp-run
```

The captured binary is local reference evidence and must not be committed.

## C port of the init slice (Agent 2, M3)

`adsp_init_image.h` / `adsp_init_image.c` re-emit the same four fixtures
(`nop`, `reset-loop`, `init-prefix`, `init-state`) in C99 with no
dependencies, plus the four observed DM landmarks (`DM($0955)=0x1242`,
`DM($0956)=0x124E`, `DM($0959)=0x7FFF`, `DM($095A)=0xFFFF` from
`analysis/ram-map.md`). Byte output is identical to
`tools/build-replacement-image` for all four fixtures (verified by direct
comparison, not by re-reading one side's output). The native target
compiles these same sources (`native/CMakeLists.txt`,
`adsp-init-slice-c`), so reproduction and native share one canonical
implementation. Public ROM-free checks: `tests/test_adsp_c_image.py` and
the compiled self-check `adsp_init_image_test.c`.

## C port of the control/IRQ contract and install boundary (Agent 2, M3)

`adsp_control_seq.h` / `adsp_control_seq.c` encode the 68010-visible ADSP
control-window contract from
`reference/experiments/stunrun/adsp-control-window.metadata.json`: the 11
operations totaling the 73 observed all-zero writes per 600-frame
title-path run (LED, `/BR`, `/HALT`, reset assert/release, bank, default
controls), the `0x818060` IRQ-clear pair from PCs `0x021426`/`0x02C234`,
and the zero-read IRQ-state window. A multiset tally consumes observed
`(address, data)` pairs and reports whether a run reproduced the
contract; intra-run write order was never recorded, so no sequence order
is claimed. The same module encodes the upload/install frame boundary
from `m3-adsp-upload-boundary.metadata.json` (empty through 407,
populating 408–410, complete at 411, install-ready at 412). Comparing a
replacement run against the canonical title checkpoint (M3 item 3) stays
Verifier-owned; the tally is the reusable checker that comparison
consumes. Native wiring: `adsp-control-slice-c` in `native/CMakeLists.txt`.
Public ROM-free checks: `tests/test_adsp_c_control.py` and the compiled
self-check `adsp_control_seq_test.c`.

## C port of the 68010 upload-stream framing (Agent 2, M3)

`adsp_upload_stream.h` / `adsp_upload_stream.c` encode the 68010-side
program-upload contract from
`reference/experiments/stunrun/adsp-program-upload.metadata.json`: the
ROM-resident source pointer (`0x0001702E` via `0x02C204`/`0x02D2E0`),
the transfer loop (`0x02D304–0x02D364`) with its single writer PC
`0x02D35C`, the 17-record table totaling 2728 words (= 5456 halfword
taps), the explicit destination gap (word indices 2203–4136), and the
`0xFF` terminator. The destination rule `address = 0x800000 + 4*index`
reproduces the observed `0x800000–0x8048D6` coverage exactly. Record
payload semantics remain unresolved and are not modeled. Native wiring:
`adsp-upload-slice-c` in `native/CMakeLists.txt`. Public ROM-free
checks: `tests/test_adsp_c_upload.py` and the compiled self-check
`adsp_upload_stream_test.c`.

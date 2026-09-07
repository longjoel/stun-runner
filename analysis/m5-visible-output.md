# M5 first visible output — oracle boundary

M5 is not complete yet. The original rendered checkpoint is now pinned as a
repeatable local oracle, and the native shell can emit an image artifact with
matching geometry, but the native pixels are still the deliberate blank M4
scaffold.

## Original oracle

Two fresh installed-MAME runs used the pinned title checkpoint recipe:

```sh
SDL_VIDEODRIVER=dummy mame -rompath /tmp/stunrun-roms-system \
  -noreadconfig -nowriteconfig -cfg_directory <fresh-cfg> \
  -nvram_directory <fresh-nvram> -nonvram_save stunrun -video soft \
  -sound none -skip_gameinfo -seconds_to_run 10 \
  -snapname m5-oracle -snapshot_directory <output> \
  -autoboot_script mame/lua/checkpoint.lua
```

Both produced a byte-identical `512×240` 8-bit RGB PNG:

```text
sha256: 9aa7b12be07ef28b5796c64e67e722b595664cd09b8c5daead3d7a3707b9e85e
```

The frame contains the S.T.U.N. Runner title/attract presentation, vehicle,
roadway, and HUD. This is a rendered observation, not a claim that the native
target has reconstructed those subsystems.

## Native boundary

Set `STUNRUN_RENDER_PPM=/tmp/native-frame.ppm` when running the native shell.
It writes a `512×240` P6 RGB frame and reports its deterministic pixel hash.
The current native frame is tagged `mode=blank-scaffold`; comparing it against
the oracle is expected to fail until evidence-backed title rendering is
implemented. The next investigation is to identify the smallest original
renderer/palette/command slice needed to reproduce one stable visible region.

The dependency-free `tools/compare-ppm` gate reports that comparison rather
than reducing it to a single hash. Against the two-run oracle conversion and
the current native blank frame, the observed result was:

```text
dimensions: 512x240 on both sides
changed pixels: 121610 / 122880
changed channels: 334729
maximum channel error: 255
mean channel error: 79.6585205078125
```

This is the expected negative control and is retained so the first real native
visual slice can be measured against the same oracle.

## First renderer boundary

A paired GSP trace over frames `850–930` narrows the first input-dependent
visual divergence to a pixel-blit loop. The stationary and driven traces share
the preceding `PIXBLT B,XY` and address-generation instructions; the first
aligned difference is the driven path taking `ADDXY A8,A1` at `0xFFF465A0`
where the stationary path executes another `PIXBLT B,XY` at `0xFFF46590`.
The driven trace also contains 558 occurrences of the known `F4000000` writer
set versus 405 in the stationary trace. This is the current smallest literal
renderer boundary, not yet a decoded tile, palette, or full-screen renderer.

Provenance and complete trace hashes are in
`reference/experiments/stunrun/m5-gsp-render-boundary.metadata.json`.

## GSP snapshot address units

The GSP program space reports an address shift of `3`, so a byte-oriented
snapshot must not assume that consecutive sampled addresses are consecutive
GSP values. `tools/mame-memory-snapshot` now accepts `--address-stride` and
records it in the result metadata. For the installed MAME configuration, a
16-bit GSP VRAM word capture uses stride `16`; the corresponding 8-bit view
uses stride `8`.

The GSP display registers are mapped at `0xc0000000` and are likewise
captured at stride `16`. The register indices relevant to the multisync
scanline callback are `DPYCTL=8`, `DPYSTART=9`, `DPYTAP=27`, `DPYADR=30`,
`HEBLNK=1`, and `HSBLNK=2`. The bounded GSP-state recipe now reports these
values and effective `fine_scroll` alongside the PC/status and memory checksums. The
fine-scroll control is read from GSP control register 1 and masked to the
multisync 3-bit range, matching MAME's `hdgsp_control_hi_w` behavior. This establishes the
runtime inputs needed to apply MAME's literal scanline formula to a VRAM
snapshot; it does not yet claim a decoded native renderer.

## Literal frame decoder

`tools/decode-gsp-frame` applies the multisync callback to a captured 16-bit
GSP VRAM snapshot and the currently selected 256-entry palette reads. It is
deliberately parameterized for `DPYTAP` and fine scroll because the current
low-rate state sample is a frame-boundary observation, not a per-scanline
register trace. A synthetic fixture covers the word-byte selection and row/
column formula.

The first real decode reproduces the title scene's broad structure, but is not
pixel-equivalent yet. With the frame-600 captures and explicit
`--dpy-tap 0x4c0 --fine-scroll 7`, the current comparison is:

```text
dimensions: 512x240 on both sides
changed pixels: 60343 / 122880
mean channel error: 28.99599880642361
```

This is a useful positive boundary, not an M5 completion claim. The remaining
work is to capture the display parameters at the actual scanline/update point
or reconcile MAME's screenshot crop with the frame-boundary register sample.

The GSP-state tool now accepts `--screen PATH` and calls MAME's
`screen:snapshot()` at the requested frame. A frame-600 paired run produced a
`512×240` PNG with the canonical oracle hash
`9aa7b12be07ef28b5796c64e67e722b595664cd09b8c5daead3d7a3707b9e85`, proving
that the memory/register capture and rendered oracle can be collected from one
deterministic run.

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

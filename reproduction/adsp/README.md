# ADSP replacement image

The first M2 slice is a deliberately minimal ADSP-2100 program image. It uses
the known `NOP` encoding from the M0 emission proof, places one 32-bit word at
ADSP program address `0x0000`, and records a fixed entry point at the same
address.

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

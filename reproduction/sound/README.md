# JSA-II sound latch transport (Agent 2 reproduction slice)

`jsa_latch.h` / `jsa_latch.c` encode the 68010 <-> 6502 sound
command/response path from `reference/experiments/stunrun/sound-boundary-tap.metadata.json`
and `sound-handler-search.metadata.json`:

- the MAME-confirmed two-latch transport (68010 `0x600000` write asserts
  the 6502 NMI; 6502 `$2A02` write asserts main IRQ4; each side reads and
  clears its latch);
- the `$280A`/`$2802` command, `$280C` rdio, `$280E` IRQ-ack register
  classification;
- the frame-447 `0x1E` command-byte pairing and the frame-1 `0xFF`
  startup response;
- the identical 11-category title-path event counts from both 600-frame
  runs, with a field-by-field matcher for Verifier consumption.

Byte payload semantics beyond the frame-447 pairing are unresolved and
not modeled. The native target compiles these same sources
(`native/CMakeLists.txt`, `jsa-latch-slice-c`), so reproduction and
native share one canonical implementation. Public ROM-free checks:
`tests/test_sound_c_latch.py` and the compiled self-check
`jsa_latch_test.c`.

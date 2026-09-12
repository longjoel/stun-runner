# 62-byte course record layout (gated frame 1038, mouth scene)

Regen: tools/mame-memory-snapshot /var/tmp/stunrun-roms OUT --device
:mainpcb:maincpu --base 0xFF9C00 --count 2560 --width 16 --address-stride 2
--frames 1045 --targets 1038 --input drive --screen OUT/shot-1038.png
(snapshot sha256 b68cf4a5462f8b560913547855e7a93df587f14d6365db5668e41bc5094e36e4;
screen shows tunnel-mouth entry, same run).

Record = 31 words at stride 0x3E (62 bytes). Offsets in bytes.

| offset | content | example (slot 0xFF9F2A, entry 47) |
| --- | --- | --- |
| +$00/+$02 | prev link (pool head or record) | 00FF DA96 |
| +$04/+$06 | next link | 00FF A386 |
| +$08 | unknown (animated) | 0000 |
| +$0A..+$12 | animated fields (pos/color) | 563E FFFF FEF7 0002 8C2B |
| +$14..+$24 | 18-byte orientation matrix (chain-propagated) | 3DDA 016E 105B FFFE 3FC2 FA72 EF94 055E 3D9F |
| +$26..+$28 | table entry pointer (32-bit) | 0004 9348 -> 0x49348 = entry 47 |
| +$2A/+$2C | unknown | 0080 97F0 |
| +$2E..+$34 | zero | 0000 x4 |
| +$36/+$38 | status flags | 0000 0200 (heap) / FEE0 0100 (static) |
| +$3A..+$3C | unknown | 0000 0000 |

Gated instances at frame 1038 (mouth):
- 0xFF9F2A -> entry 47 (shape 2580 ribs), chain ...DA96 <-> 9F2A <-> A386...
- 0xFFA386 -> entry 47, prev 9F2A next 9EEC
- 0xFFA3C4 -> entry 47, prev A4FA next DA96
- 0xFFA402 -> entry 151 (run 146-151 near flats), prev A196 next DA86
- 0xFF9C80 -> entry 0 (desert start, STATIC: links zero, flags FEE0/0100)

Caveats:
- Entry-204 binding seen at slot+$26 0xFFA428 in write-trace runs (trans, gated)
  reads as entry 151 at slot base 0xFFA402 in this snapshot run: same slot,
  different entry. Either intra-frame write/snapshot ordering or run divergence
  (boot-path flake is proven: rec1038b slow-boot). The layout, chain, and
  entry-pointer mechanism are unaffected; per-frame entry identity at a slot
  must be treated as same-run-only evidence.
- Width/stride: memory-snapshot needs --width 16 --address-stride 2 for words;
  stride 1 with width 16 returns byte-shifted u16 reads (looks plausible, is not).
- Boot nondeterminism: identical snapshot configs have produced fast-boot
  (mouth@1038) and slow-boot ("DOWNLOADING GSP"@1038) runs. Gate every scene claim
  with a same-run screen.

## Native rib slice (Implementer, 2026-09-11)

`native/tunnel_rib.{h,c}` renders the entry-47 rib faces as four flat-shaded
quads (two facets per side, kink at y=123) through the shared software
backend, in sampled band color (151,43,34). Corners measured from the gated
shot-1038 (see masks embedded in `native/tunnel_rib_test.c`); the test demands
>=70% precision and recall against those masks plus clear center-gap and
wall-field probes. `ctest`: native-tunnel-rib-c passes; full native suite
34/34, Python suite 129/129. Rendered output verified by eye
(/tmp/ribwork/rib.png scratch): two band pairs at the mouth position.

## Native vista slice (Implementer, 2026-09-11)

`native/vista_quad.{h,c}` renders the entry-151 far field as two quads through
the shared backend: a straight neck converging to VP ~(253, 15) plus a road
flare curving left below y=125 (the demo road bends), flat (182,151,120).
`native/vista_quad_test.c` embeds the measured tan mask (x 214-269, y 85-145)
and demands >=85% precision/recall; composite vista+ribs eye-verified
(/tmp/ribwork/combo.png scratch). Full suites: 35 native, 129 Python, green.

## Native torus-lines slice (Implementer, 2026-09-11)

`native/torus_lines.{h,c}` renders three wireframe floor cross-lines from the
gated frame-1920 torus scene (score 1080) as Bresenham lines in sampled color
(177,46,36): left/right spans at y=120 with a center opening (x 201-312) and
one center span at y=172. Record provenance: ten chained 62B records bound to
entry 41 (shape 0x2580) with heap flags 0x1800/0x1A00 (wireframe mode, vs
0x0/0x200 solid at the mouth); exact record-to-line mapping still open, and
the diffuse radial fragments are not modeled. The test pins endpoints,
midpoints, clear zones, and total pixel count (425). Eye-verified
(/tmp/ribwork/torus.png scratch).

## Native torus side rails (Implementer, 2026-09-11)

`native/torus_lines.{h,c}` gains `stunrun_torus_rails_fixture` (+
`stunrun_render_torus_rails`): two steep side rails (left x11/y78 to
x180/y106, right mirrored about x256, slope +/-6) plus short connectors
from the rail ends to the floor-line corners, all in wireframe
(177,46,36), measured per-row from shot-1920. The test probes rail
endpoints/midpoints, clear center, and a 375-392 total pixel count;
eye-verified (/tmp/ribwork/torus_combo.png scratch: rails funnel into the
floor corners, center span below).

## Native torus below-floor fragments (Implementer, 2026-09-11)

`stunrun_torus_frags_fixture` (+ `stunrun_render_torus_frags`) adds the
four short fragments just under the floor line — outer left
(40,122)-(138,154), outer right (482,122)-(382,154), inner left
(203,121)-(223,127), inner right (310,121)-(290,127): the rails
continuing past the floor line with a perspective kink (slope ~6 above,
~3 below), endpoints traced per-row through y154. The test probes all
eight endpoints plus midpoints, clear center, and a 232-250 total pixel
count; eye-verified (torus_combo.png: outer frags continue below the
floor line, inner frags peek just under it). Full suites: 38 native
(Debug, asserts live), 129 Python, green.

## Torus wireframe record↔screen correspondence (Investigator, 2026-09-11)

Regen: tools/mame-memory-snapshot ROMS OUT --device :mainpcb:maincpu
--base 0xFF9C00 --count 2560 --width 16 --address-stride 2 --frames 1927
--targets 1920 --input drive --screen OUT/shot-1920.png (same-run gate:
torus wireframe, score 1080). Scanning the snapshot for 62B-aligned
slots with heap wireframe flags (0x1800/0x1A00 at +$38) yields 12
mutually-0x3E-aligned slots — against 11 modeled wireframe lines
(3 floor + 4 rails + 4 frags). Record-to-line mapping stays open, but
the pair sets are now published side by side:

| slot | entry ptr | +$08 tag | +$0A | +$0C | +$0E | +$10 tag | +$12 | flags |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 0xFF9F68 | 0x492D0 | 0005 | C250 | 0001 | 0F0B | 0005 | B1C8 | 0000.1800 |
| 0xFFA158 | 0x492D0 | 0005 | D65D | 0001 | 335D | 0005 | CAE7 | 0000.1800 |
| 0xFFA196 | 0x492D0 | 0005 | E064 | 0001 | 4586 | 0005 | D776 | 0000.1A00 |
| 0xFFA1D4 | 0x492D0 | 0005 | FE79 | 0001 | 7C04 | 0005 | FD1F | 0000.1800 |
| 0xFFA250 | 0x492D0 | 0005 | F472 | 0001 | 69DA | 0005 | F091 | 0000.1A00 |
| 0xFFA28E | 0x492D0 | 0006 | 1C8E | 0001 | B286 | 0006 | 22C2 | 0000.1A00 |
| 0xFFA30A | 0x492F8 | 0006 | 2C06 | 0001 | D5C3 | 0006 | 3FF0 | 0000.1A00 |
| 0xFFA386 | 0x492D0 | 0006 | 1287 | 0001 | A05B | 0006 | 1637 | 0000.1800 |
| 0xFFA3C4 | 0x492F8 | 0006 | 2575 | 0001 | C475 | 0006 | 3059 | 0000.1800 |
| 0xFFA5F2 | 0x492D0 | 0005 | EA6B | 0001 | 57B0 | 0005 | E403 | 0000.1800 |
| 0xFFA6AC | 0x492D0 | 0005 | CC56 | 0001 | 2133 | 0005 | BE57 | 0000.1A00 |
| 0xFFA7E2 | 0x492D0 | 0006 | 0880 | 0001 | 8E30 | 0006 | 09AB | 0000.1A00 |

(+$08 mirrors the +$10 tag 0005/0006 in every record; +$0C is const
0001. Matrix +$14..+$24:
chain ≈ 2CAx DA8x 1A7x 06Bx 29Dx 2FEx D2Ax E15x 211x-2128; the two
0x492F8 records carry a clearly different orientation each —
A30A: 31E9 DA85 0E1F 12CA 29D7 2CA2 DCA1 E156 2BA3;
A3C4: 2FAB DA85 1477 0CDA 29D8 2EB0 D747 E155 26B2.)

World-static result (1900↔1920 differential, 2026-09-11): a same-config
re-run captured frame 1900 (gated: torus section, score 1080; first
attempt hit the slow-boot flake and was discarded). Ten of the twelve
slots are present with IDENTICAL links/entry/anim/matrix/flags; the
only per-slot change in 20 frames is +$2C decrementing by 0x30 or 0x60
(9760→9730 … 99D0→9970), plus two list-link maintenance writes and the
absence of the two 0x492F8 slots (section still streaming in). The
screen moves substantially between the frames while record geometry
words do not move at all: world records are camera-independent, screen
position = f(record, camera). +$2C is the per-record camera-relative
parameter (depth/tick countdown candidate). Next: identify the camera
words (entry-146 craft record / 0xFFDC32 per-frame state) and the GSP
projection to close record→screen mapping.

## Record motion classification, 1900↔1920 (Investigator, 2026-09-11)

Full 45-slot diff between the gated frames (same alignment family
0x1a; scripts /tmp/scan_all_records.py, /tmp/diff_all.py; regenerate
1900 via the recipe above with --frames 1907 --targets 1900):

- STATIC WORLD: 92D0 x10 (+$2C only), 9A78/A3C4/A428 x4 (zero changes).
  World-fixed geometry; screen motion is camera-side.
- COPIES with shared deltas (template placed Nx, run uniformity):
  A022/A728/A7A4 (entries 9C30/9C1C/9C08, identical anim):
  D(+$0A,+$0E,+$12) = (+0xDA4,+0x1965,+0x111F), matrices fully
  re-propagated; A11A/A538/A630 (908C) + A576 (90A0) + A918 (9078):
  D = (+0x459/+0x45A,+0x7E1,+0x572), matrices drift +-1. Different
  entries scroll at different vectors = moving objects, not camera
  (camera would shift all equally).
- ODD VECTORS: 9EEC (9190) moves negative (-0x294,-0x42D,-0x15E),
  +$2C increases +0x60; A0DC/A4BC (9C58) small negative per-slot
  deltas (not copies).
- REBINDING (streaming): 9FE4 92D0-solid -> 9B04, A5B4 9EB0 -> 9B18,
  A30A 99C4 -> 92F8-wireframe, A3C4 92D0 -> 92F8-wireframe with flags
  0000 -> 1800 (activation). Section streams in over the 20 frames.
- Flag churn 4000 -> 0000 on the 9Cxx groups plus A11A/A538/A576/A630/
  A918 (activation); 9190/9A78/A3C4/A428/92D0/92F8 keep theirs.

Not entry-146 (no slot here matches a per-frame-rebuilt craft
profile); the camera words remain unidentified. Velocity semantics
(position vs phase) for +$0A/+$0E/+$12 stay open — the vectors above
are the evidence the projection work must explain.

Camera-hunt null result (same day): under the 0x28 entry-stride
hypothesis entry 146 would sit at 0x4A2C0, but no slot in either frame
carries that pointer — entry 146 is unbound here, outside the window,
or the stride hypothesis is wrong. Next camera attempts in order:
(1) snapshot 0xFFDC00 window at both frames for the per-frame state
word (needs two MAME runs); (2) derive the course-table entry stride
from ROM (anchors: entry 47 = 0x49348); (3) GSP-side projection math
at the 0xFFF45xxx reader pipeline.

ASSUMPTION CONFLICT with the older "ten chained entry-41" claim: count
is 12, and entry pointers (0x492D0 x10, 0x492F8 x2) sit 0x28 apart in
the same 0x4xxxx ROM region as mouth entry 47 (0x49348). Under a 0x28
entry stride these are entries 44/45 — same run 41-82, different
instances (near/mid/far layers), not entry 41. Do not cite entry 41
for the torus until the 0x2ECBE bindings trace is reconciled.
Alignment caveat: slots were first scanned 2 bytes high (flags read at
+$36 instead of +$38, yielding a nonsense entry 0x92D00080); the chain
low-16 links (xxx8 offsets) forced the −2 correction, after which
links, flags, and entry pointers all match the mouth layout.

## Torus teal arc mass — analyzed, explicitly open (2026-09-11)

Per-row/per-bin measurement of shot-1920 teal pixels (r==0, g<40, b>0;
shades (0,13,21), (0,20,33), (0,27,45), (0,34,57)) shows a broad
stippled band, not vector arcs: hundreds of short dashes spread
x0-510, y27-115, ~80px tall everywhere, with the band centroid drifting
diagonally from (x16,y48) to (x496,y94) (slope ~0.09/px). No crisp
arc spine exists to fit chords through; the texture reads as dithered
fill of the torus tube surface. Faithful reproduction needs a
stipple/fill primitive the native backend does not have — no slice
attempted. Representative evidence: /tmp/rec1920b/shot-1920.png.

## Native craft-core slice (Implementer, 2026-09-11)

`native/craft.{h,c}` renders the level-flight craft core as eight quads through
the shared backend: canopy top + two lobes (off-white 159,153,154), nose +
fuselage core + lower red mass (craft red 191,0,0), wing stubs (modal shade
207,0,0, tips x225/x271 at y169 tapering to the fuselage at y159), all
measured from the gated shot-1038 red-run extents. No 62B record is proven
to drive the craft (entry-146 rebinds often but shows no craft-like
residency constancy; L/R fork-input runs at frame 1100 are bit-identical,
so steering had not taken effect), hence screen-measured geometry only.
Wing gradient/grey-speck detail, stripe edge remnants (2px), the red right
flank (x271-275, y167-168), and outlines are open. The test embeds
canopy/fuselage masks with >=70%/75% agreement plus part probes, wing
probes, a wing-population count, and stripe black-core/edge probes;
eye-verified (/tmp/ribwork/craft.png, scene.png scratch).

Depth-order correction (same day): `stunrun_render_road_frame` paints
strips back-to-front from the array tail, so the array head is nearest.
The stripe initially went to the tail (painted first, buried under the
red mass) and the rib/vista/craft order comments were inverted; the
stripe moved to the array head, the rib near pair reordered first, and
the comments now state the convention. All overlaps except the stripe
were edge-only, so no other output changed. Full suites: 38 native
(Debug, asserts live), 129 Python, green.

## Native mouth-entry scene combo (Implementer, 2026-09-11)

`native/scene.{h,c}` composites the three frame-1038 slices far-to-near
through the shared backend: vista quad, tunnel rib, craft core
(`stunrun_render_mouth_entry`). The torus wireframe belongs to a different
section (frame 1920) and is excluded. The test asserts one probe pixel per
layer plus clear zones; eye-verified (/tmp/ribwork/scene.png scratch:
tan neck rising, red rib bands flanking, canopy + red fuselage centered
below). Full suites: 38 native (Debug, asserts live), 129 Python, green.

Build-type correction (same day): the tree had been configured with
`-DCMAKE_BUILD_TYPE=Release`, whose `-DNDEBUG` silently compiled out every
`assert()` in the C self-checks, so earlier "green" runs were vacuous.
Reconfigured to Debug, re-ran the full suite genuinely green, and pinned a
no-Release-family note in `native/README.md`.

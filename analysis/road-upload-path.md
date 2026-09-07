# Road upload path: twin buffers → GSP FIFO

Agent 2 render-path dive, 2026-09-06. Evidence-backed; semantics open.

## Established chain (all OBSERVED-IN-TRACE/REPLAY)

1. ROM table (e.g. `0x44630`, 384 words) marched by maincpu PCs
   `0x29760`/`0x2976E` into work RAM **base `0xFF9584`** (768 B verbatim).
2. Dual copy at **twin `0xFF9884`** (`+0x300`) by disjoint writer PCs.
3. Slow base-only animation (`0x298C0`, `0x298FA`, ramping, both lanes).
4. **FIFO transfer PC `0x02248E` drains the BASE buffer sequentially**
   (`0xFF9584`, `0xFF9586`, …) into the GSP FIFO at `0xC0000C`
   (1,920 twin-base reads in replay frames 1280–1287), alongside
   `0x22000`-region and `0xFF9000`-region reads (272). Local artifacts:
   `/tmp/race-twinread` (truncated at 200k events — tap drowns in
   fetches; narrow PC ranges for follow-ups).
5. GSP renders; VRAM→pixels path already bridged (`decode-gsp-frame`,
   native renderer).

The transfer-loop read was subsequently isolated with a narrow 68010 RAM-read
trace. At `0x02248E`, all three complete bursts read exactly the base range
`0xFF9584–0xFF9882` (384 unique 16-bit words) at frames 1290, 1293, and
1297. No read from the twin range occurred at that PC. The twin is active in a
different path: `0x0298BE` reads `0xFF9884–0xFF9B82`, while `0x0298C0` reads
the corresponding base range, including split bursts when a transfer crosses
a frame boundary. This identifies the twin as an animation/transform source,
not an alternate road-FIFO input.

## What this rules out

- Maincpu does NOT stage road data through the ADSP serial buffer:
  zero maincpu writes to `0x810000–0x813FFF` over replay frames
  1280–1420 (`/tmp/race-fifow`). The 0x810000 blocks come from the
  ADSP side (separate pipeline: ADSP-computed display lists).
- The award path (immediates, update 8) shares no tables with this.

## Open (handoff)

- Twin-copy reads by the transfer loop: **CLOSED NEGATIVE** for the observed
  transfer loop and roadway interval. `0x02248E` drains base only; the twin is
  consumed by the separate `0x0298BE`/`0x0298C0` animation/transform pair.
- Animator semantics (scroll ramp vs next-segment build).
- `0xFF9000`-region records' role in the transfer (272 reads).
- Field semantics inside the 768 B (which words are curvature etc.).
- Segment→table-index map (Installer/Investigator static work).

## Repro

All three probes replay `/tmp/race/r1.inp` (deterministic):
`tools/mame-ram-write-trace … --base 0x810000 --end 0x813FFF
--start-frame 1280 --end-frame 1420 --input none
--playback /tmp/race/r1.inp` (expect: empty);
`tools/mame-rom-read-trace … --pc-range 0x20000-0x50000
--rom-end 0xFFFFFF --start-frame 1280 --end-frame 1340
--input none --playback /tmp/race/r1.inp` then filter
`0xFF9584–0xFF9B83` (expect: `0x2248E` sequential drain).

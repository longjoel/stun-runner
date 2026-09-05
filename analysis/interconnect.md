# Interconnect Map — M1 Working Record

This file is the Investigator-owned M1 record. It intentionally begins with
unknowns; MAME handler names and address ranges identify hardware interfaces,
not game-level semantics.

## MAME-confirmed starting points

| Interface | Evidence | Current claim |
|---|---|---|
| 68010 program space | `analysis/driver-mining/stunrun.machine-map.yaml` | Active main CPU at `:mainpcb:maincpu`. |
| ADSP program window | same machine map; `hd68k_adsp_program_r/w` | 68010-visible range `0x800000–0x807fff`. |
| ADSP data window | same machine map; `hd68k_adsp_data_r/w` | 68010-visible range `0x808000–0x80bfff`. |
| ADSP serial/output buffer | `init_adsp()`; `hd68k_adsp_buffer_r/w` | 68010-visible range `0x810000–0x813fff`. |
| ADSP control/IRQ interfaces | `init_adsp()`; control, clear, and state handlers | 68010-visible ranges `0x818000–0x81801f`, `0x818060–0x81807f`, and `0x838000–0x83ffff`. |
| ADSP object ROM | `ROM_REGION16_BE("mainpcb:user1")` | 0x60000-byte MAME region labeled ADSP object ROM. |
| JSA interrupt callback | Step 0 driver-mining record | Sound board is wired into the main-board interrupt path; runtime behavior remains to be measured. |
| MSP | runtime reconciliation | No active MSP device is present for this target. |

## Observed-in-trace

- Two independent 600-frame `adsp_program_upload` runs successfully registered
  a 68010 program-space write watchpoint over `0x800000–0x807fff`, but the
  installed headless MAME did not deliver watchpoint action callbacks. See
  `reference/experiments/stunrun/adsp-program-upload.metadata.json`.
- The watchpoint result is setup-only; it does **not** establish an access
  count, ADSP activity, or the program's load/source path.
- The six-frame M0 trace metadata still records empty GSP/ADSP instruction
  traces as a bounded observation, not proof of inactivity.
- OBSERVED-IN-TRACE: independent 60-frame traces for both GSP and ADSP are
  empty, while independent 600-frame traces are nonempty. The first observed
  GSP trace instruction is at `0xFFF59920`; the first observed non-NOP ADSP
  instruction is `0x0004: CALL $0780`. This bounds first activity to frames
  61–600 without identifying the exact frame. Follow-up bounded probes narrow
  first GSP instruction activity to frame 132 and first ADSP instruction
  activity to frame 122; the first non-NOP ADSP instruction is present by
  frame 136. Independent repeats of both 600-frame traces are byte-identical.
- See `reference/experiments/stunrun/processor-trace-600.metadata.json` for
  trace hashes and sizes.
- OBSERVED-IN-TRACE: the 68010 executes `0x02C5EA` 14 times in each bounded
  600-frame no-input and `coin_start` trace. Each execution passes
  `0x810000` to `0x02F0AE`; that routine consumes a length-prefixed block from
  the MAME-mapped ADSP serial/output buffer and `0x02247A` copies the block to
  the GSP FIFO at `0xC0000C`, followed by a `-1` terminator check. This is an
  observed ADSP-buffer→GSP data-flow edge, not proof of the ADSP producer's
  command semantics. See
  `reference/experiments/stunrun/adsp-interface-map.metadata.json`.
- OBSERVED-IN-TRACE: the ADSP executes the MAME special-data offsets for
  `SIMBUF` (`DM($2000)`), `SIMCLK` (`$2001`), `SOMLATCH` (`$2002`),
  `SOMCLK` (`$2003`), `GINT` (`$2006`), and object-ROM bank select `MP`
  (`$2007`) in both 600-frame traces. The aggregate counts are respectively
  `73416`, `33158`, `84700`, `30`, `30`, and `10335`; `$2004` and `XOUT`
  (`$2005`) have zero observed instructions. This establishes that the ADSP
  special-I/O path is active, including the GINT and object-ROM controls, but
  does not establish the values written or the meaning of each serial block.
  See `reference/experiments/stunrun/adsp-special-io-trace.metadata.json`.
- OBSERVED-IN-TRACE: ADSP PC `0x00A0` selects MP bank 0, reads `SIMBUF`,
  shifts the result, and writes the transformed value through the serial
  clock path. PC `0x04BC` selects MP value 1 and repeatedly reads `SIMBUF`
  while processing serial items before restoring MP from ADSP data memory.
  Separately, PCs `0x004A–0x004F` write `GINT` and poll an internal marker.
  These are literal producer-side protocol candidates; their values and
  external consumer semantics remain unresolved.
- RESOLVED FOR TITLE PATH: each bounded trace contains 30 ADSP `GINT`
  writes and 30 main-CPU `IRQ 2` entries. Every `IRQ 2` entry enters the
  handler at `0x0213FE`, clears the ADSP interrupt window at `0x818060` from
  `0x021402`, executes the handler body, and returns at `0x021418`. MAME's
  `update_interrupts()` routes `m_adsp_irq_state` to main-CPU line 2, while
  `GINT` sets that state and the clear handler resets it. This resolves the
  observed ADSP `GINT → 68010 IRQ2 → acknowledgement` edge; it does not
  resolve the serial-block payload semantics. See
  `reference/experiments/stunrun/adsp-special-io-trace.metadata.json`.
- OBSERVED-IN-TRACE: direct 68010 taps over `0x810000–0x813fff` in independent
  1200-frame no-input and coin/start runs observe identical 625,989 reads,
  spanning `0x810000–0x813f50` from PCs `0x02F0C2`, `0x02F0E2`, `0x02248E`,
  and `0x02249A`. Each run has 106 count/terminator pairs: the count at
  `0x810000` is followed by a `0xFFFF` read at `0x810000 + 2*count`. This
  directly corroborates the instruction-trace ADSP-buffer→GSP FIFO edge: the
  stateful tap closes each block with exactly `count` transfer-helper writes to
  `0xC0000C`. No buffer writes were observed in the bounded 600-frame probe.
  See
  `reference/experiments/stunrun/adsp-buffer-window.metadata.json`.
- OBSERVED-IN-TRACE: direct 68010 data-window taps in independent 600-frame
  no-input and coin/start runs observe exactly one write, `0x02C20C` →
  `0x80BFFE` with data `0xFFFF`, and zero 68010 reads in
  `0x808000–0x80BFFF`. This is the reset/initialization landmark visible in
  the bounded title path; it does not characterize internal ADSP data-space
  accesses. See `reference/experiments/stunrun/adsp-data-window.metadata.json`.
- OBSERVED-IN-TRACE (clean state snapshot): a direct read of the ADSP program
  space is empty at frame 136 and contains 2,718 nonzero words at frame 600.
  This proves title-path population of the executable ADSP RAM by the canonical
  title boundary even though the headless watchpoint path does not identify the
  writer PC or payload. See
  `reference/experiments/stunrun/adsp-program-snapshot.metadata.json`.

## Open contracts

### 68010 ↔ ADSP

- OBSERVED-IN-TRACE: the clean snapshot run shows the ADSP program space empty
  through frame 122 and populated by frame 136; the program is therefore not
  simply static zero-filled RAM for the whole title path.
- RESOLVED FOR TITLE PATH: direct 68010 program-space taps observe 5456 writes
  in each independent 600-frame run, all from PC `0x02D35C`, covering
  `0x800000–0x8048d6`. The static instruction at `0x02D35C` is
  `move.l D0,(A2)+`; the preceding loop at `0x02D304–0x02D364` decodes source
  bytes, computes the destination from `0x800000`, and supplies the observed
  upload. See `reference/experiments/stunrun/adsp-program-upload.metadata.json`.
- OBSERVED-IN-TRACE: the no-input and coin/start callers at `0x02C204` read the
  same 32-bit source pointer `0x0001702E` from `$17000` and pass it to
  `0x02D2E0`. The uploader therefore consumes a repeatable RAM-resident
  encoded stream; its ownership and producer remain unknown.
- OBSERVED-IN-TRACE: normalizing the source-byte tap by its 68010 lane masks
  yields 17 records with control `0`, counts summing to 2,728 24-bit ADSP
  program words, one explicit destination gap covering word indices 2203–4136,
  and a `0xFF` terminator. This matches the 5,456 observed halfword writes
  from `0x02D35C` and the populated ADSP image. The record table is preserved
  in `reference/experiments/stunrun/adsp-program-upload.metadata.json`.
- MAME-CONFIRMED: the target has no separate ADSP program-code ROM region.
  The ADSP program map is RAM at `0x0000–0x1fff`, with `0x2000–0x3fff`
  marked `nopr`/ROM? by the driver. The separate 0x60000-byte `user1` region
  is explicitly labeled ADSP object ROM and is accessed through the special
  `SIMBUF/MP` path, not identified as executable program storage.
- NARROWED UNKNOWN: the title path populates ADSP program RAM through the
  observed `0x02D35C` loop, but the source table/buffer semantics and exact
  instruction-level normalization of the paired `MOVE.L` halfword taps remain
  open.
- NARROWED UNKNOWN: beyond the single `0x02C20C` initialization write, no
  68010 data-window traffic is observed in the bounded title path. Internal
  ADSP data-space traffic and post-gameplay main-CPU accesses remain open.
- UNKNOWN: which data-window offsets are commands, status, or shared data, and
  whether access occurs only after a verified gameplay transition.
- MAME-CONFIRMED: the data special map exposes SIMBUF/SOM/XOUT/GINT and
  object-ROM bank-selection controls; the main CPU also has buffer/control/IRQ
  windows listed above. Runtime ownership and hot offsets remain unresolved.
- OBSERVED-IN-TRACE: the ADSP executes an internal startup sequence beginning
  with `0x0004: CALL $0780` and `0x0005: CALL $0834`; the captured operations
  include internal `PM(0x1236)` and `DM(0x0955–0x095A)` accesses. This is not
  evidence of a 68010-visible transfer.
- OBSERVED-IN-TRACE: the 600-frame `coin_start` schedule changes the ADSP
  instruction trace relative to no input; the first divergence enters an ADSP
  branch reading `DM($001B)`. This proves input-dependent ADSP execution, not
  the direction or mechanism of the 68010↔ADSP exchange.

### 68010 input/status path

- OBSERVED-IN-TRACE: normal coin/start reaches the input checks at `0x02C1A0`
  and `0x02C1EE`.
- OBSERVED-IN-TRACE: an early Coin 2 variant enters a bounded trace dominated
  by `0x0013EC` → `0x0013FC`/`0x001404`, polling `0x60C001`.
- OBSERVED-IN-REPLAY: the normal and early Coin 1/start sequences both reach
  the same road/demo image as no input at frame 600, with `Credits: 0`.
- STATIC-CANDIDATE: routine `0x043590` is the only listing routine that samples
  coin bits 7/6 at `0x60C001`; it has six direct callers, but none executes in
  the tested boot/title/service/coin-start traces.
- UNKNOWN: whether the normal Coin 1 path leaves the displayed credit count at
  zero because the run remains in attract/demo, because a required transition
  is missing, or because the installed MAME input model differs from the ROM's
  expected environment. The current evidence does not justify treating the
  input checks as a credit-state contract.
- UNKNOWN: interrupt/flag direction and acknowledgement sequence.

### 68010 ↔ GSP/PSP

- OBSERVED-IN-TRACE: GSP execution begins by frame 132 in the bounded probe;
  early instructions access candidate control locations in the `0xC000...`
  and `0xF480...` ranges. Register semantics and the 68010 submission path are
  unresolved.
- OBSERVED-IN-TRACE: the same `coin_start` schedule changes the GSP trace;
  the first differing region is adjacent to repeated `0xF4800020` and
  `0xF4800030` accesses. This is an input-dependent control-path landmark,
  not a proven command protocol.
- STATIC-CANDIDATE+OBSERVED-IN-TRACE: the 68010 reset/init routines at
  `0x000356–0x0004F4` write through `0xC00002`/`0xC0000C` and configure
  `0xF480...`, palette, and control locations. Their counts are identical in
  the 600-frame no-input and coin-start traces, so they are initialization
  landmarks rather than input-dependent command evidence.
- UNKNOWN: command submission region and synchronization mechanism.

The static/runtime GSP search is recorded in
`reference/experiments/stunrun/gsp-handler-search.metadata.json`.

### 68010 ↔ JSA sound board

- OBSERVED-IN-TRACE: the active `:mainpcb:jsa:cpu` executes a deterministic
  600-frame boot/title stream beginning at `0x4000`; early accesses include
  `0x2A04` and `0x280C`. Independent traces are byte-identical. Source mapping
  identifies these as JSA WRIO and RDIO mirror accesses, respectively.
- OBSERVED-IN-TRACE: `coin_start` changes the 6502 trace and includes an
  explicit `IRQ 0` interruption while executing at `0x414C`; the source of
  that periodic sound IRQ remains unresolved, but it is distinct from the
  frame-447 command/NMI pairing below.
- STATIC-CANDIDATE+OBSERVED-IN-TRACE: the 68010 executes `0x023EF6`, which
  writes a byte to `0x600000`, and executes `0x023F66`/`0x023FAE`, which read
  the JSA window in an interrupt/ring-buffer-shaped path. Their counts are
  stable between no-input and coin-start traces (`8`, `2`, and `2`), so this
  narrows the ROM-side access path without assigning byte-level semantics.
- STATIC-CANDIDATE+OBSERVED-IN-TRACE: the observed writes are reached through
  `0x030170 -> 0x023EDA`; the caller loads bytes from a state-indexed buffer at
  `0xFFDB64` before invoking the JSA helper. This is the strongest current
  ROM-side command-buffer candidate, but its byte layout and ownership remain
  unresolved.
- RESOLVED FOR TITLE PATH (JSA→68010 half): both independent main-CPU traces
  contain exactly two `IRQ 4` entries. Each enters `0x023F24`, reads the JSA
  response at `0x600000` through `0x023F66`, and returns. MAME wires the JSA
  `sound_int_write_line` callback to `m_sound_int_state`, which
  `update_interrupts()` routes to main-CPU line 4. This establishes the
  response-interrupt edge, but not the 6502 command payload or the cause of
  the two observed events. See
  `reference/experiments/stunrun/sound-handler-search.metadata.json`.
- MAME-CONFIRMED TRANSPORT: the 68010 `0x600000` write path enters
  `main_command_w`, which schedules a delayed command latch and asserts the
  JSA 6502 NMI; the JSA-II sound CPU reads that command at `$2802` through
  `sound_command_r`. In the reverse direction, a 6502 write to `$2A02`
  through `sound_response_w` schedules a delayed response latch, invokes the
  main interrupt callback, and produces the observed main IRQ4; the 68010
  reads and clears that response through `main_response_r` at `0x600000`.
  This establishes the emulator transport contract; the title-path byte
  pairing is recorded below, while broader payload semantics and exact
  acknowledgement behavior remain unresolved. See
  `reference/experiments/stunrun/sound-handler-search.metadata.json`.
- STATIC-CANDIDATE+MAME-CONFIRMED-MIRROR: the 6502 sound listing polls `$280C`
  at `0x4154`, then consumes a queued byte from `$0235,Y` and writes it to
  `$2A02` at `0x4161`. JSA-II maps `$2802` with mirror mask `0x1F9`; the ROM's
  `$280A` access is the `sound_command_r` mirror, while `$280C` is `rdio_r` and
  `$280E` is `sound_irq_ack_r`.
- OBSERVED-IN-TRACE (clean bounded memory-tap probe): independent 600-frame
  no-input and coin/start runs expose identical classified activity when the
  taps cover the JSA mirror ranges: one main command, four main responses, four
  sound-command reads, and three sound-response writes, with separate status,
  IRQ-ack, OKI, WRIO, and MIX traffic. At frame 447 the main command reports
  bus data `0x1E1E` with mask `0xFF00` at ROM PC `0x023EF6`; the subsequent
  6502 read at PC `0x5839` from `$280A` returns `0x1E`. This is the strongest
  current title-path byte pairing. The result is recorded in
  `reference/experiments/stunrun/sound-boundary-tap.metadata.json`.
- UNKNOWN: whether the frame-447 command is the smallest stable sound fixture,
  producer buffer ownership, and exact acknowledgement behavior.
- UNKNOWN: which deterministic input/event is the smallest useful sound trigger.

The three-processor differential hashes and first-divergence landmarks are
recorded in
`reference/experiments/stunrun/processor-input-differential.metadata.json`.
The static/runtime candidate search is recorded in
`reference/experiments/stunrun/sound-handler-search.metadata.json`.
The service-input sound differential is recorded in
`reference/experiments/stunrun/sound-service-differential.metadata.json`.

## Evidence queue

The bounded experiment definitions and results belong under `reference/`; the
interpretation of those results belongs here. Start with the ADSP upload slice
specified in `analysis/m1-plan.md`, then update this file with evidence links,
confidence labels, and narrowed unknowns.
